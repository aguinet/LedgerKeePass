#ifndef APP_CRYPTO_H
#define APP_CRYPTO_H

#include <stdbool.h>
#include <stdint.h>

#define X25519_SCALARSIZE 32
#define X25519_PTSIZE 32

bool X25519EncryptKPKey(uint8_t *kpkey, uint8_t *own_pubkey,
                        uint8_t const *caller_pubkey);

#endif
