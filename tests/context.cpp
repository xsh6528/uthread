#include <gtest/gtest.h>

#include <uthread/uthread.hpp>

namespace uthread { namespace context {

static Context context_main;
static Context context_f;
static Context context_g;
static Context context_h;

static void f(void *arg) {
  int *x = reinterpret_cast<int *>(arg);
  ASSERT_EQ(*x, 0);
  (*x) = 1;
  swap(&context_f, &context_main);
}

static void g(void *arg) {
  int *x = reinterpret_cast<int *>(arg);
  ASSERT_EQ(*x, 0);
  swap(&context_g, &context_h);
  ASSERT_EQ(*x, 1);
  swap(&context_g, &context_h);
  ASSERT_EQ(*x, 2);
  swap(&context_g, &context_main);
}

static void h(void *arg) {
  int *x = reinterpret_cast<int *>(arg);
  ASSERT_EQ(*x, 0);
  (*x) = 1;
  swap(&context_h, &context_g);
  ASSERT_EQ(*x, 1);
  (*x) = 2;
  swap(&context_h, &context_g);
}

TEST(ContextTest, SwapToSnapshot) {
  int x = 0;

  if (snapshot(&context_main) == Snapshot::SNAPSHOT) {
    ASSERT_EQ(x, 0);
    x = 1;
    run(&context_main);
  }

  ASSERT_EQ(x, 1);
}

TEST(ContextTest, SwapToFunction) {
  int x = 0;
  int stack[1024];

  with_f(&context_f, stack, sizeof(stack), f, reinterpret_cast<void *>(&x));
  swap(&context_main, &context_f);

  ASSERT_EQ(x, 1);
}

TEST(ContextTest, SwapBetweenFunctions) {
  int x = 0;
  int stack_g[1024];
  int stack_h[1024];

  with_f(&context_g, stack_g, sizeof(stack_g), g, reinterpret_cast<void *>(&x));
  with_f(&context_h, stack_h, sizeof(stack_h), h, reinterpret_cast<void *>(&x));
  swap(&context_main, &context_g);

  ASSERT_EQ(x, 2);
}

}}
