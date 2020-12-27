#include <kpl/ledger_device.h>
#include <kpl/kpl.h>
#include <cstdio>
#include <vector>
#include <cstring>
#include <ctype.h>

#include "utils.h"

static std::vector<uint8_t> fromHex(const char* Hex)
{
  const size_t Len = strlen(Hex);
  std::vector<uint8_t> Ret;
  Ret.resize(Len/2);
  char Buf[3];
  Buf[2] = 0;
  for (size_t i = 0; i < Len/2; ++i) {
    Buf[0] = tolower(Hex[2*i]);
    Buf[1] = tolower(Hex[2*i+1]);
    sscanf(Buf, "%02x", &Ret[i]);
  }
  return Ret;
}

int main(int argc, char** argv)
{
  if (argc <= 2) {
    fprintf(stderr, "Usage: %s slot key\n", argv[0]);
    return 1;
  }
  const uint8_t Slot = atoi(argv[1]);
  const auto Key = fromHex(argv[2]);

  auto KPLDev = getFirstDeviceKPL();
  if (!KPLDev) {
    return 1;
  }
  auto& KPL = KPLDev.kpl();

  auto Res = KPL.setKey(Slot, &Key[0], Key.size());
  if (Res != kpl::Result::SUCCESS) {
    fprintf(stderr, "Unable to set key (%d): %s.\n", Res, kpl::errorStr(Res)); 
    return 1;
  }
  printf("Key set on slot %d.\n", Slot);
  return 0;
}
