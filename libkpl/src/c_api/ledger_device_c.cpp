#include <kpl/c_api/ledger_device.h>
#include <kpl/ledger_device.h>

#include "wrap.h"

KPL_C_API_OBJ_WRAPPERS(kpl::LedgerDevice, KPLLedgerDevice)

KPLResult KPLLedgerDeviceListGet(KPLLedgerDeviceList* List)
{
  auto Devs = kpl::LedgerDevice::listDevices();
  const size_t Size = Devs.size();
  List->Count = Size;
  if (Size == 0) {
    List->Devs = nullptr;
    return KPL_SUCCESS;
  }
  List->Devs = (KPLLedgerDevice**)malloc(Size * sizeof(KPLLedgerDevice*));
  if (!List->Devs) {
    List->Count = 0;
    return KPL_LIB_OUT_OF_MEMORY;
  }
  for (size_t i = 0; i < Size; ++i) {
    List->Devs[i] = kpl::wrap(Devs[i].release());
  }
  return KPL_SUCCESS;
}

void KPLLedgerDeviceListFree(KPLLedgerDeviceList* List, bool FreeDevices)
{
  if (List->Devs == nullptr) {
    return; 
  }
  if (FreeDevices) {
    for (size_t i = 0; i < List->Count; ++i) {
      delete kpl::unwrap(List->Devs[i]);
    }
  }
  free(List->Devs);
  List->Devs = nullptr;
}

void KPLLedgerDeviceFree(KPLLedgerDevice* Dev)
{
  if (Dev) {
    delete kpl::unwrap(Dev);
  }
}

KPL_API KPLResult KPLLedgerDeviceName(char* Name, size_t* NameLength, KPLLedgerDevice const* Dev)
{
  if (!Dev || !NameLength) {
    return KPL_C_BAD_PARAMS;
  }
  auto* CXXDev = kpl::unwrap(Dev);
  auto CXXName = CXXDev->name();
  if (!Name) {
    *NameLength = CXXName.length()+1;
    return KPL_SUCCESS;
  }
  const size_t NL = *NameLength;
  if (NL < 1) {
    return KPL_C_BAD_PARAMS;
  }
  strncpy(Name, CXXName.c_str(), NL-1);
  return KPL_SUCCESS;
}
