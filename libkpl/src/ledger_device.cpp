#include <algorithm>
#include <kpl/ledger_device.h>

#include "ledger_device_tcp.h"
#include "ledger_device_usb.h"

namespace kpl {

LedgerDevice::~LedgerDevice() = default;

VecDevices LedgerDevice::listDevices() {
  VecDevices Ret;
  {
    VecDevices TCP = LedgerDeviceTCP::listDevices();
    Ret.reserve(Ret.size() + TCP.size());
    std::transform(TCP.begin(), TCP.end(), std::back_inserter(Ret),
                   [](auto &Dev) { return std::move(Dev); });
  }
  {
    VecDevices USB = LedgerDeviceUSB::listDevices();
    Ret.reserve(Ret.size() + USB.size());
    std::transform(USB.begin(), USB.end(), std::back_inserter(Ret),
                   [](auto &Dev) { return std::move(Dev); });
  }
  return Ret;
}

std::unique_ptr<LedgerDevice> LedgerDevice::getFirstDevice() {
  VecDevices TCP = LedgerDeviceTCP::listDevices();
  if (TCP.size() > 0) {
    return std::move(TCP[0]);
  }
  VecDevices USB = LedgerDeviceUSB::listDevices();
  if (USB.size() > 0) {
    return std::move(USB[0]);
  }
  return {};
}

} // namespace kpl
