#ifndef KPL_SECBUF_H
#define KPL_SECBUF_H

#include <cstdint>
#include <cstdlib>
#include <algorithm>
#include <cstring>

namespace kpl {

struct SecBuf
{
  SecBuf() = default;
  SecBuf(size_t Len):
    Buf_((uint8_t*)malloc(Len)),
    Len_(Len)
  { }

  ~SecBuf() {
    if (Buf_) {
      // TODO
      memset(Buf_, 0, Len_);
      free(Buf_);
    }
  }

  SecBuf(SecBuf&& O):
    SecBuf()
  {
    swap(std::move(O));
  }

  SecBuf& operator=(SecBuf&& O) {
    swap(std::move(O));
    return *this;
  }

  operator bool() const { return Buf_ != nullptr; }

  uint8_t* begin() { return Buf_; }
  uint8_t* end() { return Buf_ + Len_; }
  uint8_t const* begin() const { return Buf_; }
  uint8_t const* end() const { return Buf_ + Len_; }
  size_t size() const { return Len_; }

private:
  void swap(SecBuf&& O) {
    std::swap(Buf_, O.Buf_);
    std::swap(Len_, O.Len_);
  }
  uint8_t* Buf_ = nullptr;
  size_t Len_ = 0;
};

} // kpl

#endif
