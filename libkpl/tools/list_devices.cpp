#include <kpl/ledger_device.h>
#include <iostream>

int main(int argc, char** argv)
{
  auto Devs = kpl::LedgerDevice::listDevices();
  for (auto const& Dev: Devs) {
    std::cout << Dev->name() << std::endl;
  }
  return 0;
}
