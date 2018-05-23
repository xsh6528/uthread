/**
 * Some literature on x86-64 context switching...
 *
 * https://samwho.co.uk/blog/2013/06/01/context-switching-on-x86/
 * https://github.com/freebsd/freebsd/blob/master/sys/x86/include/ucontext.h
 * https://en.wikipedia.org/wiki/X86_calling_conventions
 * https://en.wikipedia.org/wiki/FLAGS_register
 */

.globl context_get
context_get:
  movq %rbx, 8  (%rdi)
  movq %rcx, 16 (%rdi)
  movq %rdx, 24 (%rdi)
  movq %rdi, 32 (%rdi)
  movq %rsi, 40 (%rdi)
  movq %rbp, 48 (%rdi)

  // %rsp...
  leaq 8(%rsp), %rax
  movq %rax   , 56(%rdi)

  // %rip...
  movq (%rsp), %rax
  movq %rax  , 64(%rdi)

  movq %r8,  72 (%rdi)
  movq %r9,  80 (%rdi)
  movq %r10, 88 (%rdi)
  movq %r11, 96 (%rdi)
  movq %r12, 104(%rdi)
  movq %r13, 112(%rdi)
  movq %r14, 120(%rdi)
  movq %r15, 128(%rdi)

  // Return Snapshot::SWITCH after a run(...)...
  movq $1, 0(%rdi)

  // Return Snapshot::SNAPSHOT...
  movq $0, %rax

  ret

.globl context_set
context_set:
  movq 0  (%rdi), %rax
  movq 8  (%rdi), %rbx
  movq 16 (%rdi), %rcx
  movq 24 (%rdi), %rdx

  // Skip %rdi...

  movq 40 (%rdi), %rsi
  movq 48 (%rdi), %rbp
  movq 56 (%rdi), %rsp

  // Skip %rip...

  movq 72 (%rdi), %r8
  movq 80 (%rdi), %r9
  movq 88 (%rdi), %r10
  movq 96 (%rdi), %r11
  movq 104(%rdi), %r12
  movq 112(%rdi), %r13
  movq 120(%rdi), %r14
  movq 128(%rdi), %r15

  // Set %rip...
  pushq 64(%rdi)

  // Set %rdi..
  movq 32 (%rdi), %rdi

  ret
