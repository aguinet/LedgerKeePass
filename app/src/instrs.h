#ifndef APP_INSTRS_H
#define APP_INSTRS_H

#include <stdint.h>

enum AppInstrs {
  INS_GET_APP_CONFIGURATION = 0,
  INS_STORE_KEY = 1,
  INS_GET_KEY = 2,
  INS_GET_KEY_FROM_NAME = 3,
  INS_GET_VALID_SLOTS = 4,
  INS_LAST
};

typedef uint16_t (*handleInstrFunTy)(
    // p1,p2,data,datalen
    uint8_t, uint8_t, uint8_t const *, uint8_t,
    // flags,tx
    volatile unsigned int *, volatile unsigned int *);

handleInstrFunTy getHandleInstr(uint8_t Ins);

#endif
