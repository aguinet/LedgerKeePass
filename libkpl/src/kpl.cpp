#include <memory>
#include <cassert>

#include <sodium.h>

#include <kpl/app_errors.h>
#include <kpl/errors.h>
#include <kpl/kpl.h>
#include <kpl/ledger_answer.h>
#include <kpl/ledger_device.h>

#include "instrs.h"

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

static Result parseAppSW(int SW)
{
  switch (SW) {
    case KPL_SW_SUCCESS:
      return Result::SUCCESS;
    case KPL_SW_UNKNOWN_INSTR:
    case KPL_SW_INVALID_CLS:
    case KPL_SW_INVALID_PARAMETER:
      return Result::PROTOCOL_BAD_PARAM;
    case KPL_SW_UNAUTHORIZED_ACCESS:
      return Result::APP_UNAUTHORIZED_ACCESS;
    case KPL_SW_EMPTY_SLOT:
      return Result::APP_EMPTY_SLOT;
    default:
      return Result::APP_UNKNOWN_ERROR;
  }
}

KPL::KPL(LedgerDevice& Dev):
  Client_(Dev)
{ }

KPL::~KPL() = default;

ErrorOr<KPL> KPL::fromDevice(LedgerDevice& Device, Version& AppVer, unsigned TimeoutMS)
{
  auto Res = Device.connect();
  if (Res != Result::SUCCESS) {
    return {ErrorTag{}, Res};
  }
  KPL Ret(Device);

  Res = Ret.fillAppVersion(TimeoutMS);
  if (Res != Result::SUCCESS) {
    return {ErrorTag{}, Res};
  }
  AppVer = Ret.AppVer_;

  const auto LibVer = Version::lib();
  if (!LibVer.isProtocolCompatible(Ret.AppVer_)) {
    return {ErrorTag{}, Result::PROTOCOL_BAD_VERSION};
  }
  return {std::move(Ret)};
}

Result KPL::setKey(uint8_t Slot, uint8_t const* Key, const size_t KeyLen, unsigned TimeoutMS)
{
  if (KeyLen != keySize()) {
    return Result::LIB_BAD_LENGTH;
  }
  auto APDU = Client_.apduStream(Instrs::STORE_KEY, Slot)
                     .append(Key, KeyLen);
  LedgerAnswer<0> Ans;
  auto Res = APDU.exchange(Ans, TimeoutMS);
  if (Res != Result::SUCCESS) {
    return Res;
  }
  return parseAppSW(Ans.SW());
}

static Result sendKeyApduAndDecrypt(LedgerClient& Client, uint8_t* Out, APDUStream& APDU, X25519Scalar const& OwnPrivKey, X25519Pt const& OwnPubKey, unsigned TimeoutMS)
{
  int SW;
  constexpr size_t AnsLen = sizeof(X25519Pt) + KPL_KEY_SIZE;
  LedgerAnswer<AnsLen> Ans;
  auto Res = APDU.exchange(Ans, TimeoutMS);
  if (Res != Result::SUCCESS) {
    return Res;
  }
  Res = parseAppSW(Ans.SW());
  if (Res != Result::SUCCESS) {
    return Res;
  }
  if (Ans.dataSize() != AnsLen) {
    return Result::PROTOCOL_BAD_LENGTH;
  }

  // Decrypt
  X25519Pt AppPubKey;
  memcpy(&AppPubKey[0], Ans.data_begin(), sizeof(AppPubKey));
  memcpy(Out, Ans.data_begin() + sizeof(AppPubKey), KPL_KEY_SIZE);
  if (!X25519DecryptKey(Out, AppPubKey, OwnPubKey, OwnPrivKey)) {
    return Result::LIB_X25519_FAIL;
  }

  return Result::SUCCESS;
}

Result KPL::getKey(uint8_t Slot, uint8_t* Out, size_t OutLen, unsigned TimeoutMS)
{
  if (OutLen != keySize()) {
    return Result::LIB_BAD_LENGTH;
  }

  X25519Scalar OwnPrivKey;
  X25519Pt OwnPubKey;
  X25519GenPrivKey(OwnPrivKey, OwnPubKey);

  auto Stream = Client_.apduStream(Instrs::GET_KEY, Slot);
  Stream.append(OwnPubKey);
  const auto Res = sendKeyApduAndDecrypt(Client_, Out, Stream, OwnPrivKey, OwnPubKey, TimeoutMS);
  sodium_memzero(&OwnPrivKey[0], sizeof(OwnPrivKey));
  sodium_memzero(&OwnPubKey[0], sizeof(OwnPubKey));
  return Res;
}

Result KPL::getKeyFromName(const char* Name, uint8_t* Out, const size_t OutLen, unsigned TimeoutMS)
{
  const size_t NameLen = strlen(Name);
  if (NameLen > maxNameSize() || OutLen != keySize()) {
    return Result::LIB_BAD_LENGTH;
  }

  X25519Scalar OwnPrivKey;
  X25519Pt OwnPubKey;
  X25519GenPrivKey(OwnPrivKey, OwnPubKey);

  auto Stream = Client_.apduStream(Instrs::GET_KEY_FROM_NAME);
  Stream.append(OwnPubKey);
  Stream.append(reinterpret_cast<uint8_t const*>(Name), NameLen);
  const auto Res = sendKeyApduAndDecrypt(Client_, Out, Stream, OwnPrivKey, OwnPubKey, TimeoutMS);
  sodium_memzero(&OwnPrivKey[0], sizeof(OwnPrivKey));
  sodium_memzero(&OwnPubKey[0], sizeof(OwnPubKey));
  return Res;
}

Result KPL::fillAppVersion(unsigned TimeoutMS)
{
  LedgerAnswer<3> Buf;
  auto Res = Client_.apduStream(Instrs::GET_APP_CONFIGURATION).exchange(Buf, TimeoutMS);
  if (Res != Result::SUCCESS) {
    return Res;
  }
  Res = parseAppSW(Buf.SW());
  if (Res != Result::SUCCESS) {
    return Res;
  }
  if (Buf.dataSize() != 3) {
    return Result::PROTOCOL_BAD_LENGTH;
  }
  AppVer_ = Version{Buf[0], Buf[1], Buf[2]};
  return Result::SUCCESS;
}

Result KPL::getValidKeySlots(std::vector<uint8_t>& Out, unsigned TimeoutMS)
{
  LedgerAnswer<1> Buf;
  auto Res = Client_.apduStream(Instrs::GET_VALID_KEY_SLOTS).exchange(Buf, TimeoutMS);
  if (Res != Result::SUCCESS) {
    return Res;
  }
  Res = parseAppSW(Buf.SW());
  if (Res != Result::SUCCESS) {
    return Res;
  }
  if (Buf.dataSize() != 1) {
    return Result::PROTOCOL_BAD_LENGTH;
  }
  Out.reserve(slotCount());
  uint8_t Slots = Buf[0];
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
