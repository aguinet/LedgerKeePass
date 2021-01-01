#include <kpl/kpl_csts.h>
#include <kpl/version.h>

namespace kpl {

Version Version::lib() {
  return {KPL_VERSION_MAJOR, KPL_VERSION_MINOR, KPL_VERSION_PATCH};
}

} // namespace kpl
