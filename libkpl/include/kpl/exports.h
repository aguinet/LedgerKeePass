#ifndef KPL_EXPORTS_H
#define KPL_EXPORTS_H

#if defined _WIN32 || defined __CYGWIN__
#define KPL_HELPER_IMPORT __declspec(dllimport)
#define KPL_HELPER_EXPORT __declspec(dllexport)
#define KPL_HELPER_LOCAL
#else
#define KPL_HELPER_IMPORT __attribute__((visibility("default")))
#define KPL_HELPER_EXPORT __attribute__((visibility("default")))
#define KPL_HELPER_LOCAL __attribute__((visibility("hidden")))
#endif

#ifdef kpl_STATIC
#define KPL_API
#define KPL_LOCAL
#elif defined(kpl_EXPORTS)
#define KPL_API KPL_HELPER_EXPORT
#define KPL_LOCAL KPL_HELPER_LOCAL
#else
#define KPL_API KPL_HELPER_IMPORT
#define KPL_LOCAL KPL_HELPER_LOCAL
#endif

#endif
