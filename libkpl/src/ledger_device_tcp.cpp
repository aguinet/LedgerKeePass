#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <cassert>

#include <kpl/ledger_answer.h>
#include "ledger_device_tcp.h"

namespace kpl {

LedgerDeviceTCP::LedgerDeviceTCP(const char* Host, uint16_t Port):
  FD_(-1),
  Host_(Host),
  Port_(Port)
{ }

LedgerDeviceTCP::~LedgerDeviceTCP()
{
  close();
}

void LedgerDeviceTCP::close()
{
  if (FD_ >= 0) {
    ::close(FD_);
    FD_ = -1;
  }
}

std::unique_ptr<LedgerDevice> LedgerDeviceTCP::create(const char* IP, uint16_t Port)
{
  return std::unique_ptr<LedgerDevice>{new LedgerDeviceTCP{IP, Port}};
}

std::string LedgerDeviceTCP::name() const
{
  std::string Name = "TCP proxy<" + Host_ + ":" + std::to_string(Port_) + ">";
  return Name;
}

Result LedgerDeviceTCP::connect()
{
  if (FD_ >= 0) {
    close();
  }
  struct in_addr inaddr;
  if (inet_aton(Host_.c_str(), &inaddr) == 0) {
    return Result::TRANSPORT_TCP_UNK_HOST;
  }
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    return Result::TRANSPORT_TCP_SOCKET_CREATE_FAIL;
  }
  struct sockaddr_in addrtcp;
  addrtcp.sin_family = AF_INET;
  addrtcp.sin_addr = inaddr;
  addrtcp.sin_port = htons(Port_);
  if (::connect(sock, (struct sockaddr*)&addrtcp, sizeof(addrtcp)) != 0) {
    ::close(sock);
    return Result::TRANSPORT_CONNECTION_FAILED;
  }
  FD_ = sock;
  return Result::SUCCESS;
}

Result LedgerDeviceTCP::send(uint8_t const* Data, size_t DataLen)
{
  assert(FD_ >= 0);
  return (::write(FD_, Data, DataLen) == DataLen) ? Result::SUCCESS : Result::TRANSPORT_GENERIC_ERROR;
}

Result LedgerDeviceTCP::read(uint8_t* Out, size_t OutLen, unsigned TimeoutMS)
{
  assert(FD_ >= 0);

  struct timeval tv;
  // Fast path
  if (TimeoutMS == 0) {
    memset(&tv, 0, sizeof(tv));
  }
  else {
    tv.tv_sec = TimeoutMS / 1000;
    tv.tv_usec = (TimeoutMS - tv.tv_sec*1000)*1000;
  }
  setsockopt(FD_, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
  ssize_t ret = ::read(FD_, Out, OutLen);
  if (ret >= 0 && ret < OutLen) {
    return Result::TRANSPORT_TIMEOUT;
  }
  if (ret < 0) {
    if (errno == EWOULDBLOCK || errno == EAGAIN) {
      return Result::TRANSPORT_TIMEOUT;
    }
    return Result::TRANSPORT_GENERIC_ERROR;
  }
  return (((size_t)ret) == OutLen) ? Result::SUCCESS : Result::TRANSPORT_GENERIC_ERROR;
}

Result LedgerDeviceTCP::exchange(LedgerAnswerBase& Out,
    uint8_t const* Data, const size_t DataLen,
    unsigned TimeoutMS)
{
  uint32_t PktLen = htonl(DataLen);
  auto Res = send(reinterpret_cast<uint8_t const*>(&PktLen), sizeof(PktLen));
  if (Res != Result::SUCCESS) {
    return Res;
  }
  Res = send(Data, DataLen);
  if (Res != Result::SUCCESS) {
    return Res;
  }
  uint32_t RecvLen;
  Res = read(reinterpret_cast<uint8_t*>(&RecvLen), sizeof(RecvLen), TimeoutMS);
  if (Res != Result::SUCCESS) {
    return Res;
  }
  RecvLen = ntohl(RecvLen)+sizeof(SWTy);
  if (RecvLen > Out.bufSize()) {
    return Result::PROTOCOL_BAD_LENGTH;
  }
  Out.resize(RecvLen);
  return read(Out.buf_begin(), RecvLen, TimeoutMS);
}

VecDevices LedgerDeviceTCP::listDevices()
{
  char* proxy_addr = getenv("LEDGER_PROXY_ADDRESS");
  char* proxy_port = getenv("LEDGER_PROXY_PORT");
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

} // kpl
