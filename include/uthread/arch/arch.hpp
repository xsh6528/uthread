#ifndef UTHREAD_ARCH_ARCH_HPP_
#define UTHREAD_ARCH_ARCH_HPP_

#if __GNUC__
#ifdef __x86_64__
#define UTHREAD_X86_64
#else
#error "Unsupported architecture!"
#endif
#else
#error "Unsupported compiler!"
#endif

#endif  // UTHREAD_ARCH_ARCH_HPP_
