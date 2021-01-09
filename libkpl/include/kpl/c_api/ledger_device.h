#ifndef KPL_C_DEVICE_H
#define KPL_C_DEVICE_H

#include <kpl/c_api/exports.h>
#include <kpl/c_api/errors.h>

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _KPLLedgerDevice KPLLedgerDevice;
typedef struct {
  KPLLedgerDevice** Devs;
  size_t Count;
} KPLLedgerDeviceList;

KPL_C_API KPLResult KPLLedgerDeviceListGet(KPLLedgerDeviceList* List);
KPL_C_API void KPLLedgerDeviceListFree(KPLLedgerDeviceList* List, bool FreeDevices = true);

// If Name is nullptr, needed length of string is stored into NameLength
KPL_C_API KPLResult KPLLedgerDeviceName(char* Name, size_t* NameLength, KPLLedgerDevice const* Dev);
KPL_C_API void KPLLedgerDeviceFree(KPLLedgerDevice* Dev);

#ifdef __cplusplus
}
#endif

#endif
