#include <glog/logging.h>

#include <uthread/context.hpp>

namespace uthread {

void context_with_f(Context *context,
                    void *stack,
                    size_t stack_size,
                    void (*f)(void *),
                    void *arg) {
  CHECK_NOTNULL(context);
  CHECK_NOTNULL(stack);
  CHECK_GT(stack_size, 0);
  CHECK_NOTNULL(f);

  // Let's make sure stack is 16-byte aligned.
  uint64_t stack_addr = reinterpret_cast<uint64_t>(stack) + stack_size - 1;
  uint64_t shift_addr = stack_addr % 16;
  stack_addr -= shift_addr;

  CHECK_LT(shift_addr, stack_size);

  context->rdi = reinterpret_cast<uint64_t>(arg);
  context->rbp = stack_addr;
  context->rsp = stack_addr;
  context->rip = reinterpret_cast<uint64_t>(f);
}

}
