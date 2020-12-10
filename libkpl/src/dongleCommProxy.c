#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "dongleCommProxy.h"

int getFirstDongleProxy(const char* addr, uint16_t port)
{
  struct in_addr inaddr;
  if (inet_aton(addr, &inaddr) == 0) {
    return -1;
  }
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    return -1;
  }
  struct sockaddr_in addrtcp;
  addrtcp.sin_family = AF_INET;
  addrtcp.sin_addr = inaddr;
  addrtcp.sin_port = htons(port);
  if (connect(sock, (struct sockaddr*)&addrtcp, sizeof(addrtcp)) != 0) {
    close(sock);
    return -1;
  }
  return sock;
}

int sendApduProxy(int sock, uint8_t const* data, const size_t dataLen, uint8_t* out, const size_t outLen, int* sw)
{
  uint32_t len = htonl(dataLen);
  if (write(sock, &len, sizeof(len)) < 0) {
    return -1;
  }
  if (write(sock, data, dataLen) < 0) {
    return -1;
  }

  len = 0;
  if (read(sock, &len, sizeof(len)) != sizeof(len)) {
    return -1;
  }
  len = ntohl(len);

  if (len > outLen) {
    return -1;
  }
  if (len > 0) {
    if (read(sock, out, len) != len) {
      return -1;
    }
  }
  uint16_t rsw;
  if (read(sock, &rsw, sizeof(rsw)) != sizeof(rsw)) {
    return -1;
  }
  if (sw) {
    *sw = ntohs(rsw);
  }
  return len;
}

int closeDongleTransport(int sock)
{
  if (sock >= 0) {
    close(sock);
  }
}
