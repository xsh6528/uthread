#include <chrono>

#include <gtest/gtest.h>

#ifndef UTHREAD_GTEST_HPP_
#define UTHREAD_GTEST_HPP_

#define ASSERT_TAKES_APPROX_MS(EXPR, MS) {                     \
auto start = std::chrono::steady_clock::now();                 \
EXPR;                                                          \
auto took = std::chrono::steady_clock::now() - start;          \
auto took_ms =                                                 \
  std::chrono::duration_cast<std::chrono::milliseconds>(took)  \
    .count();                                                  \
ASSERT_GE(took_ms, (MS));                                      \
ASSERT_LE(took_ms, (MS) * 1.1);                                \
}

#endif  // UTHREAD_GTEST_HPP_
