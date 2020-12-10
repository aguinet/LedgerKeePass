#include <kpl/kpl.h>
#include <iostream>
#include <vector>
#include <cstring>
#include <ctype.h>

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
    std::cerr << "Usage: " << argv[0] << " slot key" << std::endl;
    return 1;
  }
  const uint8_t Slot = atoi(argv[1]);
  const auto Key = fromHex(argv[2]);

  kpl::Version AppVer;
  auto EKPL = kpl::KPL::getWithFirstDongle(AppVer);
  if (!EKPL) {
    std::cerr << "Unable to connect to device: " << EKPL.errorValue() << std::endl;
    return 1;
  }
  auto& KPL = EKPL.get();

  auto Res = KPL.setKey(Slot, &Key[0], Key.size());
  if (Res != kpl::Result::SUCCESS) {
    std::cerr << "Unable to set key: " << Res << std::endl;
    return 1;
  }
  std::cout << "Key set on slot " << (int)Slot << "." << std::endl;
  return 0;
}
