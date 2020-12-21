#ifndef KPL_LEDGER_DEVICE_H
#define KPL_LEDGER_DEVICE_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <kpl/ledger_answer.h>

namespace kpl {

class LedgerClient;
class LedgerAnswerBase;

class LedgerDevice {
  friend class LedgerClient;

public:
  using VecDevices = std::vector<std::unique_ptr<LedgerDevice>>;

  virtual ~LedgerDevice(); 

  virtual std::string name() const = 0;
  virtual bool connect() = 0;

  static VecDevices listDevices();
  static std::unique_ptr<LedgerDevice> getFirstDevice();

protected:
  virtual bool exchange(LedgerAnswerBase& Out,
    uint8_t const* Data, const size_t DataLen,
    unsigned TimeoutMS = 0) = 0;
};

using VecDevices = LedgerDevice::VecDevices;

} // kpl

#endif
