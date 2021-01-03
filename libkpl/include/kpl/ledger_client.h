#ifndef KPL_LEDGER_CLIENT_H
#define KPL_LEDGER_CLIENT_H

#include <memory>
#include <vector>

#include <kpl/errors.h>
#include <kpl/exports.h>

namespace kpl {

class LedgerDevice;
class LedgerClient;
class LedgerAnswerBase;

struct KPL_API APDUStream {
  friend class LedgerClient;

  APDUStream &append(uint8_t const *Data, const size_t DataLen) {
    Buf_.insert(Buf_.end(), Data, Data + DataLen);
    return *this;
  }

  template <size_t N> APDUStream &append(std::array<uint8_t, N> const &Data) {
    return append(&Data[0], Data.size());
  }

  Result exchange(LedgerAnswerBase &Out, unsigned TimeoutMS = 0);

  ~APDUStream();

protected:
  APDUStream(LedgerClient &Client, uint8_t CLA, uint8_t Ins, uint8_t P1,
             uint8_t P2);

private:
  void wipe();

  LedgerClient &Client_;
  std::vector<uint8_t> Buf_;
};

class KPL_API LedgerClient {
public:
  LedgerClient(LedgerDevice &Dev, uint8_t CLA = 0xE0) : Dev_(&Dev), CLA_(CLA) {}
  LedgerClient(LedgerClient &&) = default;
  LedgerClient &operator=(LedgerClient &&) = default;
  ~LedgerClient();

  APDUStream apduStream(uint8_t Ins, uint8_t P1 = 0, uint8_t P2 = 0);
  Result rawExchange(LedgerAnswerBase &Out, uint8_t const *Data = nullptr,
                     const size_t DataLen = 0, unsigned TimeoutMS = 0);

  LedgerDevice &dev() { return *Dev_; }

private:
  LedgerDevice *Dev_;
  uint8_t CLA_;
};

} // namespace kpl

#endif
