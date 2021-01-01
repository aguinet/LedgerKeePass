#ifndef KPL_ERRORS_H
#define KPL_ERRORS_H

#include <cassert>
#include <utility>

namespace kpl {

enum Result : int {
  SUCCESS = 0,
  DEVICE_NOT_FOUND = -1,
  APP_INVALID_NAME = -2,
  APP_UNAUTHORIZED_ACCESS = -3,
  APP_EMPTY_SLOT = -4,
  APP_UNKNOWN_ERROR = -5,
  LIB_BAD_LENGTH = -6,
  LIB_OUT_OF_MEMORY = -7,
  LIB_X25519_FAIL = -8,
  PROTOCOL_BAD_PARAM = -9,
  PROTOCOL_BAD_LENGTH = -10,
  PROTOCOL_BAD_VERSION = -11,
  TRANSPORT_TCP_UNK_HOST = -12,
  TRANSPORT_TCP_SOCKET_CREATE_FAIL = -13,
  TRANSPORT_USB_BAD_PACKET = -14,
  TRANSPORT_USB_BAD_SEQ = -15,
  TRANSPORT_TIMEOUT = -16,
  TRANSPORT_CONNECTION_FAILED = -17,
  TRANSPORT_GENERIC_ERROR = -18,
};

const char *errorStr(Result Res);

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
