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

  std::vector<uint8_t> Slots;
  auto Res = KPL.getValidKeySlots(Slots);
  if (Res != kpl::Result::SUCCESS) {
    fprintf(stderr, "Unable to get slots: %d!\n", Res);
    return 1;
  }
  if (Slots.size() == 0) {
    puts("No valid slots!");
    return 0;
  }
  for (uint8_t S : Slots) {
    printf("%d ", S);
  }
  printf("\n");
  return 0;
}
