#ifndef APP_GLOBALS
#define APP_GLOBALS

#include "os.h"
#include "ux.h"
#include "os_io_seproxyhal.h"

#include <kpl/kpl_csts.h>

extern ux_state_t ux;
// display stepped screens
extern unsigned int ux_step;
extern unsigned int ux_step_count;

typedef struct {
  uint8_t data[KPL_KEY_SIZE];
} stored_key_t;

typedef struct internalStorage_t {
  stored_key_t keys[KPL_SLOT_COUNT];
  // Bitfield
  uint8_t key_valids;
  uint8_t initialized;
} internalStorage_t;

extern const internalStorage_t N_storage_real;
#define N_storage (*(volatile internalStorage_t*) PIC(&N_storage_real))

static inline bool is_key_valid(uint8_t slot) {
  return (N_storage.key_valids >> slot) & 1;
}

static inline void set_key_as_valid(uint8_t slot) {
  uint8_t key_valids = N_storage.key_valids;
  key_valids |= (1U<<slot);
  nvm_write(&N_storage.key_valids, &key_valids, sizeof(key_valids));
}

#endif
