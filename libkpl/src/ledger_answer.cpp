#include <kpl/ledger_answer.h>
#include <sodium.h>

namespace kpl {

LedgerAnswerBase::~LedgerAnswerBase() {
  sodium_memzero(buf_begin(), bufSize());
}

} // namespace kpl
