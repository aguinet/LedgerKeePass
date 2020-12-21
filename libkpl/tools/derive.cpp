#include <kpl/kpl.h>
#include <kpl/ledger_device.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

int main(int argc, char** argv)
{
  if (argc <= 1) {
    fprintf(stderr, "Usage: %s name\n", argv[0]);
    return 1;
  }
  const char* Name = argv[1];

  auto Dev = kpl::LedgerDevice::getFirstDevice();
  if (!Dev) {
    fprintf(stderr, "Unable to find a Ledger device!\n");
    return 1;
  }
  fprintf(stderr, "Using device '%s'\n", Dev->name().c_str());
  kpl::Version AppVer;
  auto EKPL = kpl::KPL::fromDevice(std::move(Dev), AppVer);
  if (!EKPL) {
    fprintf(stderr, "Error while initializing connection: %d!\n", EKPL.errorValue());
    return 1;
  }
  auto& KPL = EKPL.get();

  uint8_t Key[32];
  auto Res = KPL.getKeyFromName(Name, Key, sizeof(Key));
  if (Res != kpl::Result::SUCCESS) {
    fprintf(stderr, "Unable to get key: %d!\n", Res);
    return 1;
  }
  for (uint8_t V: Key) {
    printf("%02X", V);
  }
  printf("\n");
  return 0;
}
