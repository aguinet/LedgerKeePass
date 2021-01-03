#ifndef KPL_TOOLS_UTILS_H
#define KPL_TOOLS_UTILS_H

#include <memory>

namespace kpl {
class LedgerDevice;
class KPL;
} // namespace kpl

struct KPLDev {
  KPLDev() = default;
  KPLDev(KPLDev &&); 
  KPLDev &operator=(KPLDev &&) = default;
  ~KPLDev();

  kpl::KPL &kpl() { return *KPL_; }
  kpl::LedgerDevice &dev() { return *Dev_; }

  operator bool() const { return KPL_.get() != nullptr; }

  std::unique_ptr<kpl::LedgerDevice> Dev_;
  std::unique_ptr<kpl::KPL> KPL_;
};

KPLDev getFirstDeviceKPL();

#endif
