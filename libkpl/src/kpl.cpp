#include <memory>
#include <cassert>

#include <sodium.h>

#include <kpl/kpl.h>
#include "instrs.h"
#include "secbuf.h"


extern "C" {
#include "dongleComm.h"
}

static dongleHandle hdl(void* H) {
  return reinterpret_cast<dongleHandle>(H);
}

using X25519Scalar = std::array<uint8_t, crypto_box_SECRETKEYBYTES>;
using X25519Pt = std::array<uint8_t, crypto_box_PUBLICKEYBYTES>;

static void X25519GenPrivKey(X25519Scalar& OwnPrivKey, X25519Pt& OwnPubKey)
{
  randombytes_buf(&OwnPrivKey[0], crypto_box_SECRETKEYBYTES);
  crypto_scalarmult_base(&OwnPubKey[0], &OwnPrivKey[0]);
}

static bool X25519DecryptKey(uint8_t* Key,
  X25519Pt const& AppPubKey, X25519Pt const& OwnPubKey,
  X25519Scalar const& OwnPrivKey)
{
  X25519Pt SharedSecret;
  if (crypto_scalarmult(&SharedSecret[0], &OwnPrivKey[0], &AppPubKey[0]) != 0) {
    return false;
  }

  crypto_generichash_state h;
  crypto_generichash_init(&h, NULL, 0U, KPL_KEY_SIZE);
  crypto_generichash_update(&h, &SharedSecret[0], sizeof(SharedSecret));
  crypto_generichash_update(&h, &OwnPubKey[0], sizeof(OwnPubKey));
  crypto_generichash_update(&h, &AppPubKey[0], sizeof(AppPubKey));

  uint8_t Keystream[KPL_KEY_SIZE];
  crypto_generichash_final(&h, &Keystream[0], KPL_KEY_SIZE);

  for (size_t i = 0; i < KPL_KEY_SIZE; ++i) {
    Key[i] ^= Keystream[i];
  }

  sodium_memzero(&SharedSecret[0], sizeof(SharedSecret));
  sodium_memzero(&h, sizeof(h));
  sodium_memzero(&Keystream[0], sizeof(Keystream));
  return true;
}

namespace kpl {

static SecBuf genAPDU(uint8_t Ins,
  uint8_t P1, uint8_t P2,
  uint8_t const* Data, size_t DataLen,
  uint8_t ExpectedRetLen, X25519Pt const* PubKey = nullptr)
{
  uint8_t const* DataEnd = Data+DataLen;
  if (PubKey) {
    DataLen += sizeof(X25519Pt);
  }
  SecBuf Ret(DataLen + 6);
  if (!Ret) {
    return Ret;
  }
  auto It = Ret.begin();
  *(It++) = 0xE0;
  *(It++) = Ins;
  *(It++) = P1;
  *(It++) = P2;
  *(It++) = DataLen;
  if (PubKey) {
    It = std::copy(PubKey->begin(), PubKey->end(), It);
  }
  if (Data) {
    It = std::copy(Data, DataEnd, It);
  }
  *It = ExpectedRetLen;
  return Ret;
}

KPL::KPL(void* H):
  Handle_(H)
{ }
KPL::KPL(KPL&& O):
  Handle_(nullptr)
{
  swap(std::move(O));
}
KPL& KPL::operator=(KPL&& O) {
  swap(std::move(O));
  return *this;
}

KPL::~KPL()
{
  close();
}

void KPL::close()
{
  if (!Handle_)
    return;
  closeDongle(hdl(Handle_));
  exitDongle();
  Handle_ = nullptr;
}

void KPL::swap(KPL&& O)
{
  std::swap(Handle_, O.Handle_);
  std::swap(AppVer_, O.AppVer_);
}

ErrorOr<KPL> KPL::getWithFirstDongle(Version& AppVer)
{
  initDongle();
  auto H = getFirstDongle();
  if (H == NULL) {
    exitDongle();
    return {ErrorTag{}, Result::DEVICE_NOT_FOUND};
  }
  KPL Ret(H);

  Result Res = Ret.fillAppVersion();
  if (Res != Result::SUCCESS) {
    Ret.close();
    return {ErrorTag{}, Res};
  }
  AppVer = Ret.AppVer_;

  const auto LibVer = Version::lib();
  if (!LibVer.isProtocolCompatible(Ret.AppVer_)) {
    Ret.close();
    return {ErrorTag{}, Result::BAD_PROTOCOL_VERSION};
  }
  return {std::move(Ret)};
}

Result KPL::setKey(uint8_t Slot, uint8_t const* Key, const size_t KeyLen)
{
  if (KeyLen != keySize()) {
    return Result::BAD_LENGTH;
  }
  const auto APDU = genAPDU(Instrs::STORE_KEY, Slot, 0,
    Key, KeyLen, 0);
  if (!APDU) {
    return Result::OUT_OF_MEMORY;
  }
  uint8_t Buf;
  int SW;
  sendApduDongle(hdl(Handle_), APDU.begin(), APDU.size(), &Buf, 1, &SW);
  return (SW == 0x9000) ? Result::SUCCESS : Result::APP_EXCEPTION;
}

static Result sendKeyApduAndDecrypt(dongleHandle Hdl, uint8_t* Out, SecBuf const& APDU,
    X25519Scalar const& OwnPrivKey, X25519Pt const& OwnPubKey)
{
  int SW;
  uint8_t RecvData[sizeof(X25519Pt) + KPL_KEY_SIZE];
  int RecvLen = sendApduDongle(Hdl, APDU.begin(), APDU.size(),
    RecvData, sizeof(RecvData), &SW);
  if (SW != 0x9000) {
    return Result::APP_EXCEPTION;
  }
  if (RecvLen != sizeof(RecvData)) {
    return Result::BAD_LENGTH;
  }

  // Decrypt
  X25519Pt AppPubKey;
  memcpy(&AppPubKey[0], RecvData, sizeof(AppPubKey));
  memcpy(Out, &RecvData[sizeof(AppPubKey)], KPL_KEY_SIZE);
  if (!X25519DecryptKey(Out, AppPubKey, OwnPubKey, OwnPrivKey)) {
    return Result::X25519_FAIL;
  }

  return Result::SUCCESS;
}

Result KPL::getKey(uint8_t Slot, uint8_t* Out, size_t OutLen)
{
  if (OutLen != keySize()) {
    return Result::BAD_LENGTH;
  }

  X25519Scalar OwnPrivKey;
  X25519Pt OwnPubKey;
  X25519GenPrivKey(OwnPrivKey, OwnPubKey);

  const auto APDU = genAPDU(Instrs::GET_KEY, Slot, 0,
    nullptr, 0,
    sizeof(X25519Pt) + KPL_KEY_SIZE, &OwnPubKey);
  if (!APDU) {
    return Result::OUT_OF_MEMORY;
  }
  const auto Res = sendKeyApduAndDecrypt(hdl(Handle_), Out, APDU, OwnPrivKey, OwnPubKey);
  sodium_memzero(&OwnPrivKey[0], sizeof(OwnPrivKey));
  sodium_memzero(&OwnPubKey[0], sizeof(OwnPubKey));
  return Res;
}

Result KPL::getKeyFromName(const char* Name, uint8_t* Out, const size_t OutLen)
{
  const size_t NameLen = strlen(Name);
  if (NameLen >= maxNameSize() || OutLen != keySize()) {
    return Result::BAD_LENGTH;
  }

  X25519Scalar OwnPrivKey;
  X25519Pt OwnPubKey;
  X25519GenPrivKey(OwnPrivKey, OwnPubKey);

  const auto APDU = genAPDU(Instrs::GET_KEY_FROM_SEED, 0, 0,
    (uint8_t const*)Name, NameLen,
    sizeof(X25519Pt) + KPL_KEY_SIZE, &OwnPubKey);
  if (!APDU) {
    return Result::OUT_OF_MEMORY;
  }
  const auto Res = sendKeyApduAndDecrypt(hdl(Handle_), Out, APDU, OwnPrivKey, OwnPubKey);
  sodium_memzero(&OwnPrivKey[0], sizeof(OwnPrivKey));
  sodium_memzero(&OwnPubKey[0], sizeof(OwnPubKey));
  return Res;
}

Result KPL::fillAppVersion()
{
  const auto APDU = genAPDU(Instrs::GET_APP_CONFIGURATION,
    0, 0, NULL, 0, 3);
  if (!APDU) {
    return Result::OUT_OF_MEMORY;
  }
  int SW;
  uint8_t Info[3];
  const int RecvLen = sendApduDongle(hdl(Handle_),
    APDU.begin(), APDU.size(),
    Info, sizeof(Info), &SW);
  if (SW != 0x9000) {
    printf("%x\n", SW);
    return Result::APP_EXCEPTION;
  }
  if (RecvLen != 3) { 
    return Result::BAD_LENGTH;
  }
  AppVer_ = Version{Info[0], Info[1], Info[2]};
  return Result::SUCCESS;
}

Result KPL::getValidKeySlots(std::vector<uint8_t>& Out)
{
  const auto APDU = genAPDU(Instrs::GET_VALID_KEY_SLOTS,
    0, 0, NULL, 0, 1);
  if (!APDU) {
    return Result::OUT_OF_MEMORY;
  }
  int SW;
  uint8_t Slots;
  const int RecvLen = sendApduDongle(hdl(Handle_),
    APDU.begin(), APDU.size(),
    &Slots, 1, &SW);
  if (SW != 0x9000) {
    Out.clear();
    return Result::APP_EXCEPTION;
  }
  if (RecvLen != 1) {
    return Result::BAD_LENGTH;
  }
  Out.reserve(slotCount());
  uint8_t S = 0;
  while (Slots > 0) {
    if (Slots & 1) {
      Out.push_back(S);
    }
    Slots >>= 1;
    ++S;
  }
  return Result::SUCCESS;
}

} // kpl
