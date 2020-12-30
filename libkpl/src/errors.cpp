#include <kpl/errors.h>

namespace kpl {

const char* errorStr(Result Res)
{
  switch (Res) {
    case Result::SUCCESS:
      return "no error";
    case Result::DEVICE_NOT_FOUND:
      return "no device found";
    case Result::APP_INVALID_NAME:
      return "name can only contain ASCII printable characters";
    case Result::APP_UNAUTHORIZED_ACCESS:
      return "user denied access on the device";
    case Result::APP_EMPTY_SLOT:
      return "no key exist in slot";
    case Result::APP_UNKNOWN_ERROR:
      return "unknown application error. Make sure the Keepass application is running!";
    case Result::LIB_BAD_LENGTH:
      return "bad length provided in the library (internal error)";
    case Result::LIB_OUT_OF_MEMORY:
      return "out of memory";
    case Result::LIB_X25519_FAIL:
      return "X25519 key exchange failed";
    case PROTOCOL_BAD_PARAM:
      return "bad parameter used in the communication protocol";
    case PROTOCOL_BAD_LENGTH:
      return "bad length used in the communication protocol";
    case PROTOCOL_BAD_VERSION:
      return "incompatible protocol";
    case TRANSPORT_TCP_UNK_HOST:
      return "unknown host";
    case TRANSPORT_TCP_SOCKET_CREATE_FAIL:
      return "unable to create tcp socket";
    case TRANSPORT_USB_BAD_PACKET:
      return "bad usb packet";
    case TRANSPORT_USB_BAD_SEQ:
      return "bad usb packets sequence";
    case TRANSPORT_TIMEOUT:
      return "communication timeout";
    case TRANSPORT_CONNECTION_FAILED:
      return "connection failed";
    case TRANSPORT_GENERIC_ERROR:
      return "communication error";
  }
  return "";
}

} // kpl
