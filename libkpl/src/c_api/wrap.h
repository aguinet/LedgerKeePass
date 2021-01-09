#ifndef KPL_C_API_WRAP_IMPL
#define KPL_C_API_WRAP_IMPL

#include <kpl/errors.h>
#include <kpl/c_api/errors.h>

#define KPL_C_API_OBJ_WRAPPERS(CPPClass, CStruct) \
  namespace kpl {\
  namespace {\
  CPPClass* unwrap(CStruct* Obj) {\
    return reinterpret_cast<CPPClass*>(Obj);\
  }\
  CStruct* wrap(CPPClass* Obj) {\
    return reinterpret_cast<CStruct*>(Obj);\
  }\
  CPPClass const* unwrap(CStruct const* Obj) {\
    return reinterpret_cast<CPPClass const*>(Obj);\
  }\
  CStruct const* wrap(CPPClass const* Obj) {\
    return reinterpret_cast<CStruct const*>(Obj);\
  } \
  } }

namespace kpl {
static inline KPLResult wrapRes(kpl::Result Res)
{
  return (KPLResult)(int)(Res);
}
static inline kpl::Result unwrapRes(KPLResult Res)
{
  return (kpl::Result)(int)(Res);
}
} // kpl
#endif
