#include <gtest/gtest.h>

#ifndef UTHREAD_GTEST_HPP_
#define UTHREAD_GTEST_HPP_

#define ASSERT_TAKES_APPROX_MS(EXPR, MS) {                \
auto start = std::chrono::steady_clock::now();            \
EXPR;                                                     \
auto took = std::chrono::steady_clock::now() - start;     \
ASSERT_GE(took, std::chrono::milliseconds(MS));           \
ASSERT_LE(took, std::chrono::milliseconds(MS + 1) * 1.1); \
}

#endif  // UTHREAD_GTEST_HPP_
