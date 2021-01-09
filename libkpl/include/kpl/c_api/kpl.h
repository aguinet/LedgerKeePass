#ifndef KPL_C_KPL_H
#define KPL_C_KPL_H

#include <stdint.h>
#include <kpl/c_api/ledger_device.h>
#include <kpl/c_api/exports.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _KPLObj KPLObj;

KPL_C_API KPLResult KPLFromDevice(KPLObj** Ret, KPLLedgerDevice* Dev, unsigned TimeoutMS = 0);
KPL_C_API void KPLFree(KPLObj* Obj);

KPL_C_API KPLResult KPLSetKey(KPLObj* Obj, uint8_t Slot, uint8_t const *Key, const size_t KeyLen, unsigned TimeoutMS = 0);
KPL_C_API KPLResult KPLGetKey(KPLObj* Obj, uint8_t Slot, uint8_t *Out, size_t OutLen, unsigned TimeoutMS = 0);
// Out must be able to store at maximum KPL_SLOT_COUNT integers
KPL_C_API KPLResult KPLGetValidKeySlots(KPLObj* Obj, uint8_t* Out, size_t* Count, unsigned TimeoutMS = 0);
KPL_C_API KPLResult KPLGetKeyFromName(KPLObj* Obj, const char *Name, uint8_t *Out, const size_t OutLen, unsigned TimeoutMS = 0);

KPL_C_API const char* KPLErrorStr(KPLResult Res);


#ifdef __cplusplus
}
#endif

#endif
