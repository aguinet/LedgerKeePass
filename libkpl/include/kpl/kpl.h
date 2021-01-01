#ifndef LIBKPL_KPL_H
#define LIBKPL_KPL_H

#include <cstdint>
#include <memory>
#include <vector>

#include <kpl/errors.h>
#include <kpl/kpl_csts.h>
#include <kpl/ledger_client.h>
#include <kpl/version.h>

namespace kpl {

class LedgerDevice;
struct KPLWithDevice;

struct KPL {
  ~KPL();
  KPL(KPL const &) = delete;
  KPL(KPL &&) = default;
  KPL &operator=(KPL &&) = default;

  static constexpr inline unsigned slotCount() { return KPL_SLOT_COUNT; }
  static constexpr inline size_t keySize() { return KPL_KEY_SIZE; }
  static constexpr inline size_t maxNameSize() { return KPL_MAX_NAME_SIZE; }

  // If error is BAD_PROTOCOL_VERSION, app version is filled in AppVer.
  static ErrorOr<KPL> fromDevice(LedgerDevice &Device, Version &AppVer,
                                 unsigned TimeoutMS = 0);

  Result setKey(uint8_t Slot, uint8_t const *Key, const size_t KeyLen,
                unsigned TimeoutMS = 0);
  Result getKey(uint8_t Slot, uint8_t *Out, size_t OutLen,
                unsigned TimeoutMS = 0);
  Result getValidKeySlots(std::vector<uint8_t> &Out, unsigned TimeoutMS = 0);
  Result getKeyFromName(const char *Name, uint8_t *Out, const size_t OutLen,
                        unsigned TimeoutMS = 0);

  Version const &appVer() const { return AppVer_; }

private:
  KPL(LedgerDevice &D);

  void close();

  Result fillAppVersion(unsigned TimeoutMS);

  LedgerClient Client_;
  Version AppVer_;
};

} // namespace kpl

#endif
