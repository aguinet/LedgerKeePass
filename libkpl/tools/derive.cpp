#include <kpl/kpl.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "utils.h"

int main(int argc, char** argv)
{
  if (argc <= 1) {
    fprintf(stderr, "Usage: %s name\n", argv[0]);
    return 1;
  }
  const char* Name = argv[1];

  auto KPLDev = getFirstDeviceKPL();
  if (!KPLDev) {
    return 1;
  }
  auto& KPL = KPLDev.kpl();

  uint8_t Key[32];
  auto Res = KPL.getKeyFromName(Name, Key, sizeof(Key));
  if (Res != kpl::Result::SUCCESS) {
    fprintf(stderr, "Unable to get key (%d): %s.\n", Res, kpl::errorStr(Res));
    return 1;
  }
  for (uint8_t V: Key) {
    printf("%02X", V);
  }
  printf("\n");
  return 0;
}
