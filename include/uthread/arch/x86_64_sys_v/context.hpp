#include <cstdint>

#ifndef UTHREAD_ARCH_X86_64_SYS_V_CONTEXT_HPP_
#define UTHREAD_ARCH_X86_64_SYS_V_CONTEXT_HPP_

namespace uthread {

struct Context {
  uint64_t rbx;
  uint64_t rdi;  /** Stores arg pointer passed to context_with_f(...) */
  uint64_t rbp;
  uint64_t rsp;
  uint64_t rip;
  uint64_t r12;
  uint64_t r13;
  uint64_t r14;
  uint64_t r15;
};

}

#endif  // UTHREAD_ARCH_X86_64_SYS_V_CONTEXT_HPP_
