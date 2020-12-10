#include <kpl/version.h>
#include <kpl/kpl_csts.h>

namespace kpl {

Version Version::lib() {
  return {KPL_VERSION_MAJOR, KPL_VERSION_MINOR, KPL_VERSION_PATCH};
}

} // kpl
