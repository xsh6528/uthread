/**
 * Some literature on x86-64 context switching...
 *
 * https://samwho.co.uk/blog/2013/06/01/context-switching-on-x86/
 * https://github.com/freebsd/freebsd/blob/master/sys/x86/include/ucontext.h
 * https://en.wikipedia.org/wiki/X86_calling_conventions
 * https://en.wikipedia.org/wiki/FLAGS_register
 *
 * NOTE: We do not snapshot and restore all of the registers for performance
 * sake! As per the System V ABI, "If the callee wishes to use registers RBX,
 * RBP, and R12–R15, it must restore their original values before returning
 * control to the caller". Other registers are volatile so we can ignore them.
 */

.globl context_get
context_get:
  // Save standard registers...
  movq %rbx, 0  (%rdi)
  movq %rbp, 16 (%rdi)
  movq %r12, 40 (%rdi)
  movq %r13, 48 (%rdi)
  movq %r14, 56 (%rdi)
  movq %r15, 64 (%rdi)

  // Save stack address...
  leaq 8 (%rsp), %rax
  movq %rax    , 24 (%rdi)

  // Save return address...
  movq 0 (%rsp), %rax
  movq %rax    , 32 (%rdi)

  // Return Snapshot::SNAPSHOT...
  movq $0, %rax

  ret

.globl context_set
context_set:
  // Load standard registers...
  movq 0  (%rdi), %rbx
  movq 16 (%rdi), %rbp
  movq 24 (%rdi), %rsp
  movq 40 (%rdi), %r12
  movq 48 (%rdi), %r13
  movq 56 (%rdi), %r14
  movq 64 (%rdi), %r15

  // Load return address...
  pushq 32 (%rdi)

  // Load rdi, for the the sake of context_with_f...
  movq 8 (%rdi), %rdi

  // Return Snapshot::SWITCH...
  movq $1, %rax

  ret
