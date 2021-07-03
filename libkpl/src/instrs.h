#ifndef KPL_INSTRS_H
#define KPL_INSTRS_H

namespace kpl {

enum Instrs : uint8_t {
  GET_APP_CONFIGURATION = 0,
  STORE_KEY = 1,
  GET_KEY = 2,
  GET_KEY_FROM_NAME = 3,
  GET_VALID_KEY_SLOTS = 4,
  ERASE_ALL_SLOTS = 5
};

} // namespace kpl

#endif
