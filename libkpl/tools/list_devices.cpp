#include <iostream>
#include <kpl/ledger_device.h>

int main(int argc, char **argv) {
  auto Devs = kpl::LedgerDevice::listDevices();
  for (auto const &Dev : Devs) {
    std::cout << Dev->name() << std::endl;
  }
  return 0;
}
