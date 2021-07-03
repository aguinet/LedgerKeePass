#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <kpl/kpl.h>
#include <kpl/ledger_device.h>
#include <vector>

#include "utils.h"

int main(int argc, char **argv) {
  auto KPLDev = getFirstDeviceKPL();
  if (!KPLDev) {
    return 1;
  }
  auto &KPL = KPLDev.kpl();

  auto Res = KPL.eraseAllSlots();
  if (Res != kpl::Result::SUCCESS) {
    fprintf(stderr, "Unable to erase all slots: %d!\n", Res);
    return 1;
  }
  puts("All slots have been erased!");
  return 0;
}
