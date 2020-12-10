#ifndef DONGLECOMM_PROXY_H
#define DONGLECOMM_PROXY_H

#include <stdint.h>

int getFirstDongleProxy(const char* addr, uint16_t port);
int sendApduProxy(int sock, uint8_t const* data, const size_t dataLen, uint8_t* out, const size_t outLen, int* sw);
int closeDongleTransport(int sock);

#endif
