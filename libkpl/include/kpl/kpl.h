#ifndef LIBKPL_KPL_H
#define LIBKPL_KPL_H

#include <cstdint>
#include <memory>
#include <vector>

#include <kpl/errors.h>
#include <kpl/kpl_csts.h>
#include <kpl/version.h>

namespace kpl {

struct KPL
{
  ~KPL();
  KPL(KPL const&) = delete;
  KPL(KPL&&); 
  KPL& operator=(KPL&&);

  static constexpr inline unsigned slotCount() { return KPL_SLOT_COUNT; }
  static constexpr inline size_t keySize() { return KPL_KEY_SIZE; }
  static constexpr inline size_t maxNameSize() { return KPL_MAX_NAME_SIZE; }

  // If error is BAD_PROTOCOL_VERSION, app version is filled in AppVer.
  static ErrorOr<KPL> getWithFirstDongle(Version& AppVer);

  Result setKey(uint8_t Slot, uint8_t const* Key, const size_t KeyLen);
  Result getKey(uint8_t Slot, uint8_t* Out, size_t OutLen);
  Result getValidKeySlots(std::vector<uint8_t>& Out);
  Result getKeyFromName(const char* Name, uint8_t* Out, const size_t OutLen);

  Version const& appVer() const { return AppVer_; }

private:
  KPL(void* Handle);
  void swap(KPL&& O);
  void close();

  Result fillAppVersion();

  void* Handle_; 
  Version AppVer_;
};

} // kpl

#endif
