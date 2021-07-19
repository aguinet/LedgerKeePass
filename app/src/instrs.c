#include "instrs.h"
#include "approve.h"
#include "globals.h"
#include "menu.h"
#include "os.h"
#include "x25519.h"
#include "nv_state.h"

#ifdef TARGET_NANOS
#include "lcx_sha256.h"
#endif

#include <assert.h>
#include <kpl/app_errors.h>
#include <kpl/kpl_csts.h>

static uint16_t handleGetAppConfiguration(uint8_t slot, uint8_t p2,
                                          uint8_t const *data, uint8_t data_len,
                                          volatile unsigned int *flags,
                                          volatile unsigned int *tx) {
  G_io_apdu_buffer[0] = LEDGER_MAJOR_VERSION;
  G_io_apdu_buffer[1] = LEDGER_MINOR_VERSION;
  G_io_apdu_buffer[2] = LEDGER_PATCH_VERSION;
  *tx = 3;
  return KPL_SW_SUCCESS;
}

static struct {
  uint8_t key[KPL_KEY_SIZE];
  uint8_t slot;
} handleStoreKeyConfirmEraseArgs_;

static void storeKeySlot(uint8_t slot, uint8_t const *data) {
  stored_key_t key;
  os_memcpy(key.data, data, KPL_KEY_SIZE);

  nvm_write((void *)&N_storage.keys[slot], &key, sizeof(key));
  set_key_as_valid(slot);
  explicit_bzero(&key, sizeof(stored_key_t));
}

static uint16_t handleStoreKeyConfirmErase(uint8_t *tx) {
  storeKeySlot(handleStoreKeyConfirmEraseArgs_.slot,
               handleStoreKeyConfirmEraseArgs_.key);

  explicit_bzero(&handleStoreKeyConfirmEraseArgs_,
                 sizeof(handleStoreKeyConfirmEraseArgs_));

  *tx = 0;
  return KPL_SW_SUCCESS;
}

static uint16_t handleStoreKey(uint8_t slot, uint8_t p2, uint8_t const *data,
                               uint8_t data_len, volatile unsigned int *flags,
                               volatile unsigned int *tx) {
  if (slot >= KPL_SLOT_COUNT) {
    return INVALID_PARAMETER;
  }

  if (data_len != KPL_KEY_SIZE) {
    return INVALID_PARAMETER;
  }

  if (is_key_valid(slot)) {
    // We need approval of the user to erase the key.
    os_memcpy(handleStoreKeyConfirmEraseArgs_.key, data, data_len);
    handleStoreKeyConfirmEraseArgs_.slot = slot;

    os_memcpy(ApproveLine1, "Slot #X already set.", 21);
    CCASSERT(1, KPL_SLOT_COUNT < 10);
    ApproveLine1[6] = '0' + slot;
    os_memcpy(ApproveLine2, "Erase?", 7);
    ui_approval(handleStoreKeyConfirmErase);
    *flags |= IO_ASYNCH_REPLY;
    return KPL_SW_SUCCESS;
  }

  storeKeySlot(slot, data);
  *tx = 0;
  return KPL_SW_SUCCESS;
}

static struct {
  uint8_t caller_pub[X25519_PTSIZE];
  uint8_t Slot;
} GetKeyAfterApproveArgs_;

static uint16_t handleGetKeyAfterApprove(uint8_t *tx) {
  const uint8_t Slot = GetKeyAfterApproveArgs_.Slot;
  assert(Slot < KPL_SLOT_COUNT);

  stored_key_t volatile const *key = &N_storage.keys[Slot];
  assert(is_key_valid(Slot));

  uint8_t *kpkey = &G_io_apdu_buffer[X25519_PTSIZE];
  for (size_t i = 0; i < KPL_KEY_SIZE; ++i) {
    kpkey[i] = key->data[i];
  }

  uint8_t *own_pubkey = &G_io_apdu_buffer[0];
  if (!X25519EncryptKPKey(kpkey, own_pubkey,
                          GetKeyAfterApproveArgs_.caller_pub)) {
    ui_idle();
    return INVALID_PARAMETER;
  }

  *tx = X25519_PTSIZE + KPL_KEY_SIZE;
  return KPL_SW_SUCCESS;
}

static uint16_t handleGetKey(uint8_t slot, uint8_t p2, uint8_t const *data,
                             uint8_t data_len, volatile unsigned int *flags,
                             volatile unsigned int *tx) {
  if (slot >= KPL_SLOT_COUNT || data_len != X25519_PTSIZE) {
    return INVALID_PARAMETER;
  }
  if (!is_key_valid(slot)) {
    return KPL_SW_EMPTY_SLOT;
  }
  GetKeyAfterApproveArgs_.Slot = slot;
  os_memcpy(GetKeyAfterApproveArgs_.caller_pub, data, X25519_PTSIZE);

  // Warning: if this string changes, the tests need to be adpated!
  os_memcpy(ApproveLine1, "Database open slot", 19);
  os_memcpy(ApproveLine2, "Slot #X", 8);
  CCASSERT(1, KPL_SLOT_COUNT < 10);
  ApproveLine2[6] = '0' + slot;
  ui_approval(handleGetKeyAfterApprove);
  *flags |= IO_ASYNCH_REPLY;
  return KPL_SW_SUCCESS;
}

#define SLIP21_PATH_SYSCALL "\x00" SLIP21_PATH
#define SLIP21_PATH_SYSCALL_LEN (1 + SLIP21_PATH_LEN)
#define SEED_PREFIX "dbname\x00"
#define SEED_PREFIX_LEN 7

static struct {
  uint8_t name[SEED_PREFIX_LEN + KPL_MAX_NAME_SIZE];
  uint8_t caller_pub[32];
  uint8_t name_len;
} GetKeyFromNameArgs_;

static uint16_t handleGetKeyFromNameAfterApprove(uint8_t *tx) {
  uint8_t *kpkey = &G_io_apdu_buffer[X25519_PTSIZE];
  // Inspired by
  // https://github.com/LedgerHQ/app-passwords/blob/b64b12b32e4c6bca21c208b97a42e3bd025bc926/src/password_typing.c#L46
  uint8_t hashName[CX_SHA256_SIZE];
  cx_hash_sha256(GetKeyFromNameArgs_.name, GetKeyFromNameArgs_.name_len,
                 hashName, sizeof(hashName));

  CCASSERT(1, CX_SHA256_SIZE % 2 == 0);
  uint32_t path[1 + CX_SHA256_SIZE / 2];
  path[0] = KPL_BIP39_PATH;
  for (size_t i = 0; i < CX_SHA256_SIZE / 2; ++i) {
    path[i + 1] = 0x80000000 | (uint32_t)(hashName[2 * i]) |
                  ((uint32_t)(hashName[2 * i + 1]) << 8);
  }
  uint8_t tmp[64];
  os_perso_derive_node_bip32(CX_CURVE_SECP256K1, path,
                             sizeof(path) / sizeof(uint32_t), tmp, &tmp[32]);

  // Hash the resulting private key and chain code. This will be the resulting
  // key.
  cx_hash_sha256(tmp, sizeof(tmp), kpkey, KPL_KEY_SIZE);
  explicit_bzero(tmp, sizeof(tmp));

  // Encrypt the resulting key
  uint8_t *own_pubkey = &G_io_apdu_buffer[0];
  if (!X25519EncryptKPKey(kpkey, own_pubkey, GetKeyFromNameArgs_.caller_pub)) {
    ui_idle();
    return INVALID_PARAMETER;
  }

  *tx = X25519_PTSIZE + KPL_KEY_SIZE;
  return KPL_SW_SUCCESS;
}

// All characters must be printable ASCII
static bool is_name_valid(const uint8_t *buf, size_t len) {
  for (size_t i = 0; i < len; ++i) {
    const uint8_t v = buf[i];
    if (v <= 0x1F || v >= 0x7F) {
      return false;
    }
  }
  return true;
}
static uint16_t handleGetKeyFromName(uint8_t p1, uint8_t p2,
                                     uint8_t const *data, uint8_t data_len,
                                     volatile unsigned int *flags,
                                     volatile unsigned int *tx) {
  if (data_len <= X25519_PTSIZE ||
      (data_len > (X25519_PTSIZE + KPL_MAX_NAME_SIZE))) {
    return INVALID_PARAMETER;
  }

  os_memcpy(GetKeyFromNameArgs_.caller_pub, data, X25519_PTSIZE);
  data += X25519_PTSIZE;
  data_len -= X25519_PTSIZE;

  if (!is_name_valid(data, data_len)) {
    return KPL_SW_INVALID_NAME;
  }

  uint8_t *name = GetKeyFromNameArgs_.name;
  os_memcpy(name, SEED_PREFIX, SEED_PREFIX_LEN);
  os_memcpy(&name[SEED_PREFIX_LEN], data, data_len);
  GetKeyFromNameArgs_.name_len = SEED_PREFIX_LEN + data_len;

  // Warning: if this string changes, the tests need to be adpated!
  os_memcpy(ApproveLine1, "Database open name", 19);
  CCASSERT(1, sizeof(ApproveLine2) >= KPL_MAX_NAME_SIZE + 3);
  ApproveLine2[0] = '\'';
  os_memcpy(&ApproveLine2[1], data, data_len);
  ApproveLine2[data_len + 1] = '\'';
  ApproveLine2[data_len + 2] = 0;
  ui_approval(handleGetKeyFromNameAfterApprove);
  *flags |= IO_ASYNCH_REPLY;
  return KPL_SW_SUCCESS;
}

static uint16_t handleGetValidSlots(uint8_t p1, uint8_t p2, uint8_t const *data,
                                    uint8_t data_len,
                                    volatile unsigned int *flags,
                                    volatile unsigned int *tx) {
  G_io_apdu_buffer[0] = N_storage.key_valids;
  *tx = 1;
  return KPL_SW_SUCCESS;
}

static uint16_t handleEraseAllSlotsApprove(uint8_t *tx) {
  *tx = 0;
  nv_app_state_init(true /* force */);
  return KPL_SW_SUCCESS;
}

static uint16_t handleEraseAllSlots(uint8_t p1, uint8_t p2, uint8_t const *data,
                             uint8_t data_len, volatile unsigned int *flags,
                             volatile unsigned int *tx) {
  // Warning: if this string changes, the tests need to be adpated!
  os_memcpy(ApproveLine1, "Erase all slots?", 17);
  os_memcpy(ApproveLine2, "They will be erased forever!", 29);
  ui_approval(handleEraseAllSlotsApprove);
  *flags |= IO_ASYNCH_REPLY;
  return KPL_SW_SUCCESS;
}

static const handleInstrFunTy Funcs[] = {
    handleGetAppConfiguration, handleStoreKey, handleGetKey,
    handleGetKeyFromName, handleGetValidSlots, handleEraseAllSlots};

handleInstrFunTy getHandleInstr(uint8_t Ins) {
  if (Ins >= INS_LAST) {
    return NULL;
  }
  return (handleInstrFunTy)PIC(Funcs[Ins]);
}
