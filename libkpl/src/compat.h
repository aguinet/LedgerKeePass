#ifndef KPL_COMPAT_H
#define KPL_COMPAT_H

#if defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif

#endif