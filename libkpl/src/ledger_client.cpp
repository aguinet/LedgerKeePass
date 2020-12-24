#include <kpl/ledger_client.h>
#include <kpl/ledger_device.h>
#include <kpl/ledger_answer.h>

#include <cassert>

#include <sodium.h>

namespace kpl {

static constexpr uint8_t APDU_CLA_IDX = 0;
static constexpr uint8_t APDU_INS_IDX = 1;
static constexpr uint8_t APDU_P1_IDX  = 2;
static constexpr uint8_t APDU_P2_IDX  = 3;
static constexpr uint8_t APDU_LEN_IDX = 4;
static constexpr size_t APDU_HEADER_SIZE = APDU_LEN_IDX+1;

APDUStream::APDUStream(LedgerClient& Client, uint8_t CLA, uint8_t Ins, uint8_t P1, uint8_t P2):
  Client_(Client)
{
  Buf_.resize(APDU_HEADER_SIZE);
  Buf_[APDU_CLA_IDX] = CLA;
  Buf_[APDU_INS_IDX] = Ins;
  Buf_[APDU_P1_IDX] = P1;
  Buf_[APDU_P2_IDX] = P2;
  Buf_[APDU_LEN_IDX] = 0; // will be fixed afterwards
}

bool APDUStream::exchange(LedgerAnswerBase& Out, unsigned TimeoutMS)
{
  assert(Buf_.size() >= APDU_HEADER_SIZE && "object already exchanged");

  Buf_[APDU_LEN_IDX] = Buf_.size()-APDU_HEADER_SIZE;
  const bool Ret = Client_.rawExchange(Out, &Buf_[0], Buf_.size(), TimeoutMS);
  wipe();
  if (!Ret) {
    return false;
  }
  return Out.bufSize() >= sizeof(SWTy);
}

void APDUStream::wipe()
{
  if (Buf_.size() > 0) {
    sodium_memzero(&Buf_.front(), Buf_.size());
    Buf_.clear();
  }
}

APDUStream::~APDUStream() {
  wipe();
}

LedgerClient::~LedgerClient() = default;

APDUStream LedgerClient::apduStream(uint8_t Ins, uint8_t P1, uint8_t P2)
{
  return APDUStream{*this, CLA_, Ins, P1, P2};
}

bool LedgerClient::rawExchange(LedgerAnswerBase& Out, uint8_t const* Data, const size_t DataLen, unsigned TimeoutMS)
{
  return dev().exchange(Out, Data, DataLen, TimeoutMS);
}

} // kpl
