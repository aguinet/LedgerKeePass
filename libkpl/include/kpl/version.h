#ifndef KPL_VERSION_H
#define KPL_VERSION_H

#include <cstdint>
#include <kpl/exports.h>

namespace kpl {

struct Version {
  Version() = default;
  Version(Version const &) = default;
  Version(uint8_t Major, uint8_t Minor, uint8_t Patch)
      : Major_(Major), Minor_(Minor), Patch_(Patch) {}

  bool isProtocolCompatible(Version const &O) const {
    return protocol() == O.protocol();
  }

  uint8_t protocol() const { return Major_; }

  KPL_API static Version lib();

private:
  uint8_t Major_;
  uint8_t Minor_;
  uint8_t Patch_;
};

} // namespace kpl

#endif
