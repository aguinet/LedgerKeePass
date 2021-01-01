#include <kpl/ledger_answer.h>
#include <sodium.h>

#include "intmem.h"

namespace kpl {

LedgerAnswerBase::~LedgerAnswerBase() {
  sodium_memzero(buf_begin(), bufSize());
}

SWTy LedgerAnswerBase::SW() const {
  return intmem::loadu_be<SWTy>(&Buf_[N_ - sizeof(SWTy)]);
}

} // namespace kpl
