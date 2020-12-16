#include "x25519.h"
#include "os.h"
#include "instrs.h"
#include "globals.h"
#include "approve.h"
#include "menu.h"
#include <kpl/kpl_csts.h>
#include <assert.h>

static void handleGetAppConfiguration(uint8_t slot, uint8_t p2, uint8_t const* data, uint8_t data_len, volatile unsigned int *flags, volatile unsigned int *tx)
{
  G_io_apdu_buffer[0] = LEDGER_MAJOR_VERSION;
  G_io_apdu_buffer[1] = LEDGER_MINOR_VERSION;
  G_io_apdu_buffer[2] = LEDGER_PATCH_VERSION;
  *tx = 3;
  THROW(0x9000);
}

static struct {
  uint8_t key[KPL_KEY_SIZE];
  uint8_t slot;
} handleStoreKeyConfirmEraseArgs_;

static void storeKeySlot(uint8_t slot, uint8_t const* data)
{
  stored_key_t key;
  memset(&key, 0, sizeof(key));
  memcpy(key.data, data, KPL_KEY_SIZE);

  nvm_write(&N_storage.keys[slot], &key, sizeof(key));
  set_key_as_valid(slot);
  explicit_bzero(&key, sizeof(stored_key_t));
}

static void handleStoreKeyConfirmErase()
{
  storeKeySlot(handleStoreKeyConfirmEraseArgs_.slot,
    handleStoreKeyConfirmEraseArgs_.key);

  explicit_bzero(&handleStoreKeyConfirmEraseArgs_,
    sizeof(handleStoreKeyConfirmEraseArgs_));

  G_io_apdu_buffer[0] = 0x90;
  G_io_apdu_buffer[1] = 0x00;
  io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, 2);
}

static void handleStoreKey(uint8_t slot, uint8_t p2, uint8_t const* data, uint8_t data_len, volatile unsigned int *flags, volatile unsigned int *tx)
{
  if (slot >= KPL_SLOT_COUNT) {
    THROW(INVALID_PARAMETER);
  }

  if (data_len != KPL_KEY_SIZE) {
    THROW(INVALID_PARAMETER);
  }

  if (is_key_valid(slot)) {
    // We need approval of the user to erase the key.
    memcpy(handleStoreKeyConfirmEraseArgs_.key, data, data_len);
    handleStoreKeyConfirmEraseArgs_.slot = slot;

    snprintf(ApproveLine1, sizeof(ApproveLine1), "Slot #%d already set.", slot);
    strncpy(ApproveLine2, "Erase?", sizeof(ApproveLine2));
    ui_approval(handleStoreKeyConfirmErase);
    *flags |= IO_ASYNCH_REPLY;
    return;
  }

  storeKeySlot(slot, data);
  *tx = 0;
  THROW(0x9000);
}

static struct {
  uint8_t caller_pub[X25519_PTSIZE];
  uint8_t Slot;
} GetKeyAfterApproveArgs_;

static void handleGetKeyAfterApprove()
{
  const uint8_t Slot = GetKeyAfterApproveArgs_.Slot;
  assert(Slot < KPL_SLOT_COUNT);

  stored_key_t volatile const* key = &N_storage.keys[Slot];
  assert(is_key_valid(Slot));

  uint8_t* kpkey = &G_io_apdu_buffer[X25519_PTSIZE];
  for (size_t i = 0; i < KPL_KEY_SIZE; ++i) {
    kpkey[i] = key->data[i];
  }

  uint8_t* own_pubkey = &G_io_apdu_buffer[0];
  if (!X25519EncryptKPKey(kpkey, own_pubkey,
        GetKeyAfterApproveArgs_.caller_pub)) {
    ui_idle();
    THROW(INVALID_PARAMETER);
  }

  uint8_t tx = X25519_PTSIZE+KPL_KEY_SIZE;
  G_io_apdu_buffer[tx++] = 0x90;
  G_io_apdu_buffer[tx++] = 0x00;
  io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, tx);
}

static void handleGetKey(uint8_t slot, uint8_t p2, uint8_t const* data, uint8_t data_len, volatile unsigned int *flags, volatile unsigned int *tx)
{
  if (slot >= KPL_SLOT_COUNT || data_len != X25519_PTSIZE) {
    THROW(INVALID_PARAMETER);
  }
  stored_key_t volatile const* key = &N_storage.keys[slot];
  if (!is_key_valid(slot)) {
    THROW(INVALID_PARAMETER);
  }
  GetKeyAfterApproveArgs_.Slot = slot;
  memcpy(GetKeyAfterApproveArgs_.caller_pub, data, X25519_PTSIZE);

  strncpy(ApproveLine1, "Keepass open slot", sizeof(ApproveLine1));
  snprintf(ApproveLine2, sizeof(ApproveLine2), "Slot #%d", slot);
  ui_approval(handleGetKeyAfterApprove);
  *flags |= IO_ASYNCH_REPLY;
}

#define PATH_PREFIX "\x00keepass_seed_"
#define PATH_PREFIX_LEN 14
static struct {
  uint8_t path[256];
  uint8_t caller_pub[32];
  uint8_t path_len;
} GetKeyFromNameArgs_;

static void handleGetKeyFromNameAfterApprove()
{
  uint8_t* kpkey = &G_io_apdu_buffer[X25519_PTSIZE];
  os_perso_derive_node_with_seed_key(HDW_SLIP21, 0,
      GetKeyFromNameArgs_.path, GetKeyFromNameArgs_.path_len,
      kpkey, NULL,
      "keepass_seed", 12);

  // Encrypt the resulting key
  uint8_t* own_pubkey = &G_io_apdu_buffer[0];
  if (!X25519EncryptKPKey(kpkey, own_pubkey, GetKeyFromNameArgs_.caller_pub)) {
    ui_idle();
    THROW(INVALID_PARAMETER);
  }

  uint8_t tx = X25519_PTSIZE+KPL_KEY_SIZE;
  G_io_apdu_buffer[tx++] = 0x90;
  G_io_apdu_buffer[tx++] = 0x00;
  io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, tx);
}

static void handleGetKeyFromName(uint8_t p1, uint8_t p2, uint8_t const* data, uint8_t data_len, volatile unsigned int *flags, volatile unsigned int *tx)
{
  if (data_len <= X25519_PTSIZE || (data_len > (X25519_PTSIZE+KPL_MAX_NAME_SIZE))) {
    THROW(INVALID_PARAMETER);
  }

  memcpy(GetKeyFromNameArgs_.caller_pub, data, X25519_PTSIZE);
  data += X25519_PTSIZE;
  data_len -= X25519_PTSIZE;

  uint8_t* path = GetKeyFromNameArgs_.path;
  memcpy(path, PATH_PREFIX, PATH_PREFIX_LEN);
  memcpy(&path[PATH_PREFIX_LEN], data, data_len);
  GetKeyFromNameArgs_.path_len = data_len + PATH_PREFIX_LEN;

  // Warning: if this string changes, the tests need to be adpated!
  strncpy(ApproveLine1, "Keepass open name", sizeof(ApproveLine1));
  memcpy(ApproveLine2, data, data_len);
  ApproveLine2[data_len] = 0;
  ui_approval(handleGetKeyFromNameAfterApprove);
  *flags |= IO_ASYNCH_REPLY;
}

static void handleGetValidSlots(uint8_t p1, uint8_t p2, uint8_t const* data, uint8_t data_len, volatile unsigned int *flags, volatile unsigned int *tx)
{
  G_io_apdu_buffer[0] = N_storage.key_valids;
  *tx = 1;
  THROW(0x9000);
}

static handleInstrFunTy Funcs[] = {
  handleGetAppConfiguration,
  handleStoreKey,
  handleGetKey,
  handleGetKeyFromName,
  handleGetValidSlots};

handleInstrFunTy getHandleInstr(uint8_t Ins)
{
  if (Ins >= INS_LAST) {
    return NULL;
  }
  return PIC(Funcs[Ins]);
}
