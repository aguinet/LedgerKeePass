#include "utils.h"
#include <kpl/errors.h>
#include <kpl/kpl.h>
#include <kpl/ledger_device.h>

#include <cstdio>

KPLDev::KPLDev(KPLDev&&) = default;
KPLDev::~KPLDev() = default;

KPLDev getFirstDeviceKPL() {
  KPLDev Ret;
  Ret.Dev_ = kpl::LedgerDevice::getFirstDevice();
  if (!Ret.Dev_) {
    fprintf(stderr, "Unable to find a Ledger device!\n");
    return {};
  }
  fprintf(stderr, "Using device '%s'\n", Ret.Dev_->name().c_str());
  kpl::Version AppVer;
  auto EKPL = kpl::KPL::fromDevice(*Ret.Dev_, AppVer);
  if (!EKPL) {
    const auto errVal = EKPL.errorValue();
    fprintf(stderr, "Error while initializing connection (%d): %s.\n", errVal,
            kpl::errorStr(errVal));
    return {};
  }
  Ret.KPL_.reset(new kpl::KPL{std::move(EKPL.get())});
  return Ret;
}
