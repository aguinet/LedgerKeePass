#include <kpl/c_api/kpl.h>
#include <kpl/kpl.h>
#include <kpl/errors.h>

#include "wrap.h"

KPL_C_API_OBJ_WRAPPERS(kpl::KPL, KPLObj)
KPL_C_API_OBJ_WRAPPERS(kpl::LedgerDevice, KPLLedgerDevice)

KPLResult KPLFromDevice(KPLObj** Ret, KPLLedgerDevice* Dev, unsigned TimeoutMS)
{
  if (!Ret || !Dev) {
    return KPL_C_BAD_PARAMS;
  }
  kpl::Version AppVer;
  auto ERet = kpl::KPL::fromDevice(*kpl::unwrap(Dev), AppVer, TimeoutMS);
  if (!ERet) {
    return wrapRes(ERet.errorValue());
  }
  auto* CObj = new kpl::KPL{std::move(ERet.get())};
  if (!CObj) {
    return KPL_LIB_OUT_OF_MEMORY;
  }
  *Ret = kpl::wrap(CObj);
  return KPL_SUCCESS;
}

void KPLFree(KPLObj* Obj)
{
  if (Obj) {
    delete kpl::unwrap(Obj);
  }
}


KPLResult KPLSetKey(KPLObj* Obj, uint8_t Slot, uint8_t const *Key, const size_t KeyLen, unsigned TimeoutMS)
{
  if (!Obj || !Key) {
    return KPL_C_BAD_PARAMS;
  }
  return wrapRes(kpl::unwrap(Obj)->setKey(Slot, Key, KeyLen, TimeoutMS));
}

KPLResult KPLGetKey(KPLObj* Obj, uint8_t Slot, uint8_t *Out, size_t OutLen, unsigned TimeoutMS)
{
  if (!Obj || !Out) {
    return KPL_C_BAD_PARAMS;
  }
  return wrapRes(kpl::unwrap(Obj)->getKey(Slot, Out, OutLen, TimeoutMS));
}

KPLResult KPLGetKeyFromName(KPLObj* Obj, const char *Name, uint8_t *Out, const size_t OutLen, unsigned TimeoutMS)
{
  if (!Obj || !Name || !Out) {
    return KPL_C_BAD_PARAMS;
  }
  return wrapRes(kpl::unwrap(Obj)->getKeyFromName(Name, Out, OutLen, TimeoutMS));
}

KPLResult KPLGetValidKeySlots(KPLObj* Obj, uint8_t* Out, size_t* Count, unsigned TimeoutMS)
{
  if (!Obj || !Out || !Count) {
    return KPL_C_BAD_PARAMS;
  }
  std::vector<uint8_t> Slots;
  kpl::Result Res = kpl::unwrap(Obj)->getValidKeySlots(Slots, TimeoutMS);
  if (Res != kpl::Result::SUCCESS) {
    return wrapRes(Res);
  }
  if (Slots.size() > KPL_SLOT_COUNT) {
    return KPL_LIB_BAD_LENGTH;
  }
  *Count = Slots.size();
  std::copy(Slots.begin(), Slots.end(), Out);
  return KPL_SUCCESS;
}

const char* KPLErrorStr(KPLResult Res)
{
  return kpl::errorStr(kpl::unwrapRes(Res));
}
