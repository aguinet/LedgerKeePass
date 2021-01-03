#ifndef KPL_ERRORS_H
#define KPL_ERRORS_H

#include <kpl/exports.h>

#include <cassert>
#include <utility>

namespace kpl {

enum Result : int {
  SUCCESS = 0,
  DEVICE_NOT_FOUND = -1,
  APP_INVALID_NAME = -10,
  APP_UNAUTHORIZED_ACCESS = -11,
  APP_EMPTY_SLOT = -12,
  APP_UNKNOWN_ERROR = -13,
  APP_SECURITY_ERROR = -14,
  LIB_BAD_LENGTH = -20,
  LIB_OUT_OF_MEMORY = -21,
  LIB_X25519_FAIL = -22,
  PROTOCOL_BAD_PARAM = -30,
  PROTOCOL_BAD_LENGTH = -31,
  PROTOCOL_BAD_VERSION = -32,
  TRANSPORT_TCP_UNK_HOST = -40,
  TRANSPORT_TCP_SOCKET_CREATE_FAIL = -41,
  TRANSPORT_USB_BAD_PACKET = -42,
  TRANSPORT_USB_BAD_SEQ = -43,
  TRANSPORT_TIMEOUT = -44,
  TRANSPORT_CONNECTION_FAILED = -45,
  TRANSPORT_GENERIC_ERROR = -46,
};

KPL_API const char *errorStr(Result Res);

struct ErrorTag {};

template <class T> struct ErrorOr {
  ErrorOr() : Valid_(false) {}
  ErrorOr(ErrorTag, Result Err) : Valid_(false) { Data_.Error_ = Err; }
  ~ErrorOr() { reset(); }

  template <class U> ErrorOr(ErrorTag, ErrorOr<U> const &O) : Valid_(false) {
    Data_.Error_ = O.errorValue();
  }

  template <class... Args> ErrorOr(Args &&...args) : Valid_(true) {
    new (&Data_.V_) T{std::forward<Args>(args)...};
  }

  ErrorOr(ErrorOr &&O) {
    if (O.Valid_) {
      new (&Data_.V_) T{std::move(O.get())};
      O.reset();
      Valid_ = true;
    } else {
      Valid_ = false;
      Data_.Error_ = O.errorValue();
    }
  }

  ErrorOr &operator=(ErrorOr &&O) {
    if (&O == this) {
      return *this;
    }
    reset();
    if (O.Valid_) {
      new (&Data_.V_) T{std::move(O.get())};
      O.reset();
      Valid_ = true;
    } else {
      Valid_ = false;
      Data_.Error_ = O.errorValue();
    }
    return *this;
  }

  operator bool() const { return Valid_; };
  bool hasError() const { return !Valid_; };
  T &get() {
    assert(Valid_ && "invalid object!");
    return Data_.V_;
  }
  T const &get() const {
    assert(Valid_ && "invalid object!");
    return Data_.V_;
  }
  Result errorValue() const { return Data_.Error_; }

  T *operator->() { return &get(); }
  T const *operator->() const { return &get(); }

private:
  void reset() {
    if (Valid_) {
      Data_.V_.~T();
    }
  }
  union U {
    ~U() {}
    U() {}
    T V_;
    Result Error_;
  } Data_;
  bool Valid_;
};

} // namespace kpl

#endif
