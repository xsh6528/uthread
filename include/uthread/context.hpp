#include <cstddef>

// TODO(amaximov): We should add Solaris and FreeBSD here in the future.
#if defined(__x86_64__) && (defined(__linux__) || defined(__APPLE__))
#include <uthread/arch/x86_64_sys_v/context.hpp>
#else
#error "Sorry, your platform is not supported!"
#endif

#ifndef UTHREAD_CONTEXT_HPP_
#define UTHREAD_CONTEXT_HPP_

namespace uthread {

/**
 * A result which signifies the cause of a return from context_get(...).
 */
enum class Snapshot {
  /**
   * The first time we return from context_get(...). In other words, after a
   * snapshot of the context was taken.
   */
  SNAPSHOT = 0,

  /**
   * A return from context_get(...) due to a subsequent context_set(...).
   */
  SWITCH = 1,
};

/**
 * Sets up a context to execute f(arg) via context_set(...).
 *
 * Behavior upon termination of f is undefined. You should switch to a
 * different context or terminate the program within f.
 */
void context_with_f(Context *context,
                    void *stack,
                    size_t stack_size,
                    void (*f)(void *),
                    void *arg);

/**
 * Saves the current context and switches to another.
 */
void context_swap(Context *current, Context const *other);

extern "C" {

/**
 * Saves a snapshot of the current execution context.
 *
 * This function returns 1 or more times. See Snapshot for more info.
 */
Snapshot context_get(Context *current) asm("context_get");

/**
 * Switches to a context.
 *
 * You MUST initialize a context via context_with_f(...) or context_get(...)
 * before using context_set(...)! Otherwise the behavior is undefined.
 */
void context_set(Context const *context) asm("context_set");;

}

}

#endif  // UTHREAD_CONTEXT_HPP_
