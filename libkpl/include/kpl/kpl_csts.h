// This file is also included by the app, so that everything is coherent.

#ifndef KPL_CSTS_H
#define KPL_CSTS_H

#ifdef __cplusplus
extern "C" {
#endif

#define KPL_KEY_SIZE 32
#define KPL_SLOT_COUNT 8
#define KPL_MAX_NAME_SIZE 20

// WARNING: changing the major version means that the protocol changed, which
// means that both Ledger apps and userland libraries will need to be updated!
// This must be synchronized with the LEDGER_MAJOR_VERSION variable defined in
// the app's Makefile (CCASSERT is used there to verify this at compile time).
#define KPL_VERSION_MAJOR 1
#define KPL_VERSION_MINOR 0
#define KPL_VERSION_PATCH 0
#define KPL_VERSION_PROTOCOL KPL_VERSION_MAJOR

#ifdef __cplusplus
}
#endif

#endif
