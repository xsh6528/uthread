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

  context->rdi = reinterpret_cast<uint64_t>(arg);
  context->rbp = reinterpret_cast<uint64_t>(stack) + stack_size - 1;
  context->rsp = reinterpret_cast<uint64_t>(stack) + stack_size - 1;
  context->rip = reinterpret_cast<uint64_t>(f);
}

}
