#include <kpl/kpl.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

int main(int argc, char** argv)
{
  if (argc <= 1) {
    fprintf(stderr, "Usage: %s slot\n", argv[0]);
    return 1;
  }
  const uint8_t Slot = atoi(argv[1]);

  kpl::Version AppVer;
  auto EKPL = kpl::KPL::getWithFirstDongle(AppVer);
  if (!EKPL) {
    fprintf(stderr, "Unable to connect to device: %d!\n", EKPL.errorValue());
    return 1;
  }
  auto& KPL = EKPL.get();

  uint8_t Key[kpl::KPL::keySize()];
  auto Res = KPL.getKey(Slot, Key, sizeof(Key));
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
