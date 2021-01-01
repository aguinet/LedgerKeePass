#include "ledger_device_usb.h"
#include "intmem.h"
#include <hidapi.h>

#include <codecvt>
#include <iomanip>
#include <locale>

#include <sodium.h>

namespace kpl {

static constexpr uint16_t LEDGER_VENDOR_ID = 0x2C97;
static constexpr size_t USB_PACKET_LEN = 64;

// Header: channel (0x101), tag (0x05), sequence index
static constexpr uint8_t USB_PACKET_HEADER[] = {0x01, 0x01, 0x05};

LedgerDeviceUSB::LedgerDeviceUSB(const char *Path, std::string &&Manufacturer,
                                 std::string &&Product, std::string &&Serial)
    : Path_(Path), Manufacturer_(std::move(Manufacturer)),
      Product_(std::move(Product)), Serial_(std::move(Serial)),
      HIDDev_(nullptr) {}

LedgerDeviceUSB::~LedgerDeviceUSB() { close(); }

void LedgerDeviceUSB::close() {
  if (HIDDev_) {
    hid_close(HIDDev_);
    HIDDev_ = nullptr;
  }
}

VecDevices LedgerDeviceUSB::listDevices() {
  VecDevices Ret;
  int r = hid_init();
  if (r < 0) {
    return Ret;
  }

  hid_device_info *Dev = hid_enumerate(LEDGER_VENDOR_ID, 0);
  if (!Dev) {
    return Ret;
  }
  std::wstring_convert<std::codecvt_utf8<wchar_t>> Conv;
  for (; Dev != nullptr; Dev = Dev->next) {
    if (Dev->interface_number == 0 || Dev->usage_page == 0xFFA0) {
      Ret.emplace_back(new LedgerDeviceUSB{
          Dev->path, Conv.to_bytes(Dev->manufacturer_string),
          Conv.to_bytes(Dev->product_string),
          Conv.to_bytes(Dev->serial_number)});
    }
  }
  return Ret;
}

std::string LedgerDeviceUSB::name() const {
  return "USB<" + Manufacturer_ + " " + Product_ + " (#" + Serial_ + ")>";
}

Result LedgerDeviceUSB::connect() {
  if (HIDDev_) {
    close();
  }
  HIDDev_ = hid_open_path(Path_.c_str());
  return (HIDDev_ == nullptr) ? Result::TRANSPORT_CONNECTION_FAILED
                              : Result::SUCCESS;
}

Result LedgerDeviceUSB::exchange(LedgerAnswerBase &Out, uint8_t const *Data,
                                 const size_t DataLen, unsigned TimeoutMS) {
  auto Res = send(Data, DataLen);
  if (Res != Result::SUCCESS) {
    return Res;
  }
  size_t RecvLen = Out.bufSize();
  Res = read(Out.buf_begin(), &RecvLen, TimeoutMS);
  if (Res != Result::SUCCESS) {
    return Res;
  }
  Out.resize(RecvLen);
  return Result::SUCCESS;
}

Result LedgerDeviceUSB::send(uint8_t const *Data, size_t DataLen) {
  if (!HIDDev_) {
    return Result::TRANSPORT_GENERIC_ERROR;
  }
  if (DataLen > 0xFFFF) {
    return Result::LIB_BAD_LENGTH;
  }
  uint8_t Buf[65];
  memset(Buf, 0, sizeof(Buf));
  auto It = std::begin(Buf);
  *(It++) = 0;
  It =
      std::copy(std::begin(USB_PACKET_HEADER), std::end(USB_PACKET_HEADER), It);
  *(It++) = 0; // Seq number low
  *(It++) = 0; // Seq number high

  intmem::storeu_be<uint16_t>(It, static_cast<uint16_t>(DataLen));
  It += sizeof(uint16_t);

  uint16_t SeqNum = 1;
  do {
    const size_t Len =
        std::min(DataLen, (size_t)std::distance(It, std::end(Buf)));
    It = std::copy(Data, Data + Len, It);
    const size_t PktLen = std::distance(std::begin(Buf), It);
    if (hid_write(HIDDev_, &Buf[0], PktLen) < PktLen) {
      sodium_memzero(&Buf[0], sizeof(Buf));
      return Result::TRANSPORT_GENERIC_ERROR;
    }
    DataLen -= Len;
    Data += Len;

    It = &Buf[1 + sizeof(USB_PACKET_HEADER)];
    intmem::storeu_be(It, SeqNum++);
    It += sizeof(SeqNum);
  } while (DataLen > 0);

  sodium_memzero(&Buf[0], sizeof(Buf));
  return Result::SUCCESS;
}

Result LedgerDeviceUSB::read(uint8_t *Out, size_t *OutLen, unsigned TimeoutMS) {
  if (*OutLen > 0xFFFF) {
    return Result::LIB_BAD_LENGTH;
  }

  uint8_t Buf[64];
  memset(Buf, 0, sizeof(Buf));
  const int ReadTimeout = (TimeoutMS > 0) ? TimeoutMS : -1;
  if (hid_read_timeout(HIDDev_, Buf, sizeof(Buf), ReadTimeout) != sizeof(Buf)) {
    sodium_memzero(Buf, sizeof(Buf));
    return Result::TRANSPORT_TIMEOUT;
  }
  if (Buf[0] != 0x01 || Buf[1] != 0x01 || Buf[2] != 5) {
    return Result::TRANSPORT_USB_BAD_PACKET;
  }
  const uint16_t Seq = intmem::loadu_be<uint16_t>(&Buf[3]);
  if (Seq != 0) {
    return Result::TRANSPORT_USB_BAD_SEQ;
  }
  size_t InLen = intmem::loadu_be<uint16_t>(&Buf[5]);
  if (InLen > *OutLen) {
    return Result::PROTOCOL_BAD_LENGTH;
  }
  *OutLen = InLen;
  auto It = &Buf[7];
  uint16_t SeqRef = 1;
  while (true) {
    const size_t Len =
        std::min(InLen, (size_t)std::distance(It, std::end(Buf)));
    memcpy(Out, It, Len);
    Out += Len;
    InLen -= Len;
    if (InLen == 0) {
      break;
    }
    if (hid_read_timeout(HIDDev_, Buf, sizeof(Buf), ReadTimeout) !=
        sizeof(Buf)) {
      sodium_memzero(Buf, sizeof(Buf));
      return Result::TRANSPORT_TIMEOUT;
    }
    const uint16_t Seq = intmem::loadu_be<uint16_t>(&Buf[3]);
    if (Seq != SeqRef++) {
      sodium_memzero(Buf, sizeof(Buf));
      return Result::TRANSPORT_USB_BAD_SEQ;
    }
    It = &Buf[5];
  }
  sodium_memzero(Buf, sizeof(Buf));
  return Result::SUCCESS;
}

} // namespace kpl
