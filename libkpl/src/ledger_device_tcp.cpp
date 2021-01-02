#ifdef _WIN32
#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#ifdef __APPLE__
#include <sys/errno.h>
#else
#include <errno.h>
#endif

#include <limits>
#include <cassert>

#include "compat.h"
#include "ledger_device_tcp.h"
#include <kpl/ledger_answer.h>

#ifndef _WIN32
static int closesocket(int fd) { return close(fd); }
#endif

static ssize_t sock_send(kpl::SockTy fd, const void *buf, size_t count, int flags) {
#ifdef _WIN32
  return ::send(fd, reinterpret_cast<const char*>(buf), (int) count, flags);
#else
  return ::send(fd, buf, count, flags);
#endif
}

static ssize_t sock_recv(kpl::SockTy sockfd, void* buf, size_t len, int flags) {
#ifdef _WIN32
  return ::recv(sockfd, reinterpret_cast<char *>(buf), (int)len, flags);
#else
  return ::recv(sockfd, buf, len, flags);
#endif
}

namespace kpl {

LedgerDeviceTCP::LedgerDeviceTCP(const char *Host, uint16_t Port)
    : FD_(-1), Host_(Host), Port_(Port) {}

LedgerDeviceTCP::~LedgerDeviceTCP() { close(); }

void LedgerDeviceTCP::close() {
  if (FD_ >= 0) {
    ::closesocket(FD_);
    FD_ = -1;
  }
}

std::unique_ptr<LedgerDevice> LedgerDeviceTCP::create(const char *IP,
                                                      uint16_t Port) {
  return std::unique_ptr<LedgerDevice>{new LedgerDeviceTCP{IP, Port}};
}

std::string LedgerDeviceTCP::name() const {
  std::string Name = "TCP proxy<" + Host_ + ":" + std::to_string(Port_) + ">";
  return Name;
}

Result LedgerDeviceTCP::connect() {
  if (FD_ >= 0) {
    close();
  }
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  struct addrinfo *addrs = nullptr;
  std::string PortStr = std::to_string(Port_);
  if (getaddrinfo(Host_.c_str(), PortStr.c_str(), &hints, &addrs) != 0) {
    return Result::TRANSPORT_TCP_UNK_HOST;
  }
  Result Res = Result::TRANSPORT_CONNECTION_FAILED;
  auto* cur_addr = addrs;
  while (cur_addr != nullptr) {
    SockTy sock = socket(cur_addr->ai_family, SOCK_STREAM, 0);
    if (sock < 0) {
      Res = Result::TRANSPORT_TCP_SOCKET_CREATE_FAIL;
      cur_addr = cur_addr->ai_next;
      continue;
    }
    if (::connect(sock, cur_addr->ai_addr, cur_addr->ai_addrlen) != 0) {
      ::closesocket(sock);
      Res = Result::TRANSPORT_CONNECTION_FAILED;
      cur_addr = cur_addr->ai_next;
      continue;
    }
    Res = Result::SUCCESS;
    FD_ = sock;
    break;
  }
  freeaddrinfo(addrs);
  return Res;
}

Result LedgerDeviceTCP::send(uint8_t const *Data, size_t DataLen) {
  assert(FD_ >= 0);
  return (sock_send(FD_, Data, DataLen, 0) == DataLen)
             ? Result::SUCCESS
             : Result::TRANSPORT_GENERIC_ERROR;
}

Result LedgerDeviceTCP::read(uint8_t *Out, size_t OutLen, unsigned TimeoutMS) {
  assert(FD_ >= 0);

  struct timeval tv;
  // Fast path
  if (TimeoutMS == 0) {
    memset(&tv, 0, sizeof(tv));
  } else {
    tv.tv_sec = TimeoutMS / 1000;
    tv.tv_usec = (TimeoutMS - tv.tv_sec * 1000) * 1000;
  }
  setsockopt(FD_, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof tv);
  ssize_t ret = sock_recv(FD_, Out, OutLen, 0);
  if (ret >= 0 && ((size_t)ret) < OutLen) {
    return Result::TRANSPORT_TIMEOUT;
  }
  if (ret < 0) {
    if (errno == EWOULDBLOCK || errno == EAGAIN) {
      return Result::TRANSPORT_TIMEOUT;
    }
    return Result::TRANSPORT_GENERIC_ERROR;
  }
  return (((size_t)ret) == OutLen) ? Result::SUCCESS
                                   : Result::TRANSPORT_GENERIC_ERROR;
}

Result LedgerDeviceTCP::exchange(LedgerAnswerBase &Out, uint8_t const *Data,
                                 const size_t DataLen, unsigned TimeoutMS) {
  if (DataLen > std::numeric_limits<uint32_t>::max()) {
    return Result::LIB_BAD_LENGTH;
  }
  uint32_t PktLen = htonl((uint32_t)DataLen);
  auto Res = send(reinterpret_cast<uint8_t const *>(&PktLen), sizeof(PktLen));
  if (Res != Result::SUCCESS) {
    return Res;
  }
  Res = send(Data, DataLen);
  if (Res != Result::SUCCESS) {
    return Res;
  }
  uint32_t RecvLen;
  Res = read(reinterpret_cast<uint8_t *>(&RecvLen), sizeof(RecvLen), TimeoutMS);
  if (Res != Result::SUCCESS) {
    return Res;
  }
  RecvLen = ntohl(RecvLen) + sizeof(SWTy);
  if (RecvLen > Out.bufSize()) {
    return Result::PROTOCOL_BAD_LENGTH;
  }
  Out.resize(RecvLen);
  return read(Out.buf_begin(), RecvLen, TimeoutMS);
}

VecDevices LedgerDeviceTCP::listDevices() {
  char *proxy_addr = getenv("LEDGER_PROXY_ADDRESS");
  char *proxy_port = getenv("LEDGER_PROXY_PORT");
  if (!proxy_addr || !proxy_port) {
    return {};
  }
  const int Port = atoi(proxy_port);
  if (Port < 0 || Port > 0xFFFF) {
    return {};
  }
  VecDevices Ret;
  auto Dev = create(proxy_addr, Port);
  if (Dev) {
    Ret.emplace_back(std::move(Dev));
  }
  return Ret;
}

} // namespace kpl
