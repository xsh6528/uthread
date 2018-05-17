#include <cstddef>

#include <uthread/arch/arch.hpp>
#ifdef UTHREAD_X86_64
#include <uthread/arch/x86_64/context.hpp>
#endif

#ifndef UTHREAD_CONTEXT_HPP_
#define UTHREAD_CONTEXT_HPP_

namespace uthread { namespace context {

enum class Snapshot {
  /**
   * The result of returning from snapshot(...) for the first time. In other
   * words, after the snapshot was taken.
   */
  SNAPSHOT = 0,

  /**
   * The result of returning from snapshot(...) after a call to run(...).
   */
  SWITCH = 1,
};

/**
 * Sets up a context to execute f(arg) in the next run(...).
 *
 * Behavior upon termination of f is undefined. You should switch to a
 * different context or terminate the program within f.
 */
void with_f(Context *context,
            void *stack,
            size_t stack_size,
            void (*f)(void *),
            void *arg);

/**
 * Saves the current context and switches to another.
 *
 * The current context will resume execution after a run(current) call.
 */
void swap(Context *current, Context const *other);

extern "C" {

/**
 * Saves a snapshot of the current execution to this context.
 *
 * This function returns 0 or more times. See Snapshot for more info.
 */
Snapshot snapshot(Context *current);

/**
 * Switches to a context.
 *
 * You MUST initialize a context with a call with(...) or snapshot(...) before
 * using run(...)! Otherwise the behavior of run is undefined.
 */
void run(Context const *context);

}

}}

#endif  // UTHREAD_CONTEXT_HPP_
