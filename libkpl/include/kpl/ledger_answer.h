#ifndef KPL_LEDGER_ANSWER_H
#define KPL_LEDGER_ANSWER_H

#include <kpl/exports.h>

#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>

namespace kpl {

using SWTy = uint16_t;

class KPL_API LedgerAnswerBase {
public:
  uint8_t *buf_begin() { return Buf_; }
  uint8_t *buf_end() { return Buf_ + N_; }

  uint8_t const *buf_begin() const { return Buf_; }
  uint8_t const *buf_end() const { return Buf_ + N_; }

  uint8_t *data_begin() { return Buf_; }
  uint8_t *data_end() { return Buf_ + N_ - sizeof(SWTy); }

  uint8_t const *data_begin() const { return Buf_; }
  uint8_t const *data_end() const { return Buf_ + N_ - sizeof(SWTy); }

  size_t bufSize() const { return N_; }
  size_t dataSize() const { return N_ - sizeof(SWTy); }

  inline uint8_t operator[](size_t Idx) const {
    assert(Idx < dataSize());
    return Buf_[Idx];
  }

  SWTy SW() const;

  void resize(size_t Len) {
    assert(Len <= N_);
    N_ = Len;
  }

protected:
  LedgerAnswerBase(size_t N, uint8_t *Buf) : N_(N), Buf_(Buf) {
    assert(N_ >= sizeof(SWTy));
  }

  ~LedgerAnswerBase();

  size_t N_;
  uint8_t *Buf_;
};

template <size_t N> class LedgerAnswer : public LedgerAnswerBase {
  static constexpr size_t LenWithSW = N + sizeof(SWTy);

public:
  LedgerAnswer() : LedgerAnswerBase(LenWithSW, &*std::begin(Buf_)) {}

private:
  std::array<uint8_t, LenWithSW> Buf_;
};

} // namespace kpl

#endif
