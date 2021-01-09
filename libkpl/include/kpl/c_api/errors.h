#ifndef KPL_C_ERRORS_H
#define KPL_C_ERRORS_H

#include <kpl/c_api/exports.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
#define KPL_RES_NAME(Name) KPL_##Name
#include <kpl/errors_vals.def>
#undef KPL_RES_NAME
  KPL_C_BAD_PARAMS = -100
} KPLResult;

KPL_C_API const char* KPLErrorMsg(KPLResult Res);

#ifdef __cplusplus
}
#endif

#endif
