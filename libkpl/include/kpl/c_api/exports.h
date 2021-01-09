#ifndef KPL_C_EXPORTS_H
#define KPL_C_EXPORTS_H

#if defined _WIN32 || defined __CYGWIN__
#define KPL_C_HELPER_IMPORT __declspec(dllimport)
#define KPL_C_HELPER_EXPORT __declspec(dllexport)
#define KPL_C_HELPER_LOCAL
#else
#define KPL_C_HELPER_IMPORT __attribute__((visibility("default")))
#define KPL_C_HELPER_EXPORT __attribute__((visibility("default")))
#define KPL_C_HELPER_LOCAL __attribute__((visibility("hidden")))
#endif

#ifdef kpl_c_STATIC
#define KPL_C_API
#define KPL_C_LOCAL
#elif defined(kpl_c_EXPORTS)
#define KPL_C_API KPL_C_HELPER_EXPORT
#define KPL_C_LOCAL KPL_C_HELPER_LOCAL
#else
#define KPL_C_API KPL_C_HELPER_IMPORT
#define KPL_C_LOCAL KPL_C_HELPER_LOCAL
#endif

#endif
