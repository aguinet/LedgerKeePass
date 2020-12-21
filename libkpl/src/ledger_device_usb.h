#ifndef KPL_LEDGER_DEVICE_USB_H
#define KPL_LEDGER_DEVICE_USB_H

#include <kpl/ledger_device.h>

#include <hidapi/hidapi.h>

#include <memory>
#include <vector>

namespace kpl {

class LedgerDeviceUSB: public LedgerDevice
{
public:
  static VecDevices listDevices();

  ~LedgerDeviceUSB() override;

  std::string name() const override; 
  bool connect() override;

protected:
  virtual bool exchange(LedgerAnswerBase& Out,
    uint8_t const* Data, const size_t DataLen,
    unsigned TimeoutMS = 0);

private:
  LedgerDeviceUSB(const char* Path,
    std::string&& Manufacturer, std::string&& Product,
    std::string&& Serial);

  void close();
  bool send(uint8_t const* Data, size_t DataLen);
  bool read(uint8_t* Out, size_t OutLen, unsigned TimeoutMS = 0);

  std::string Path_;
  std::string Manufacturer_;
  std::string Product_;
  std::string Serial_;

  hid_device* HIDDev_;
};

} // kpl

#endif
