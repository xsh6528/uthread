#include <cpp/hello_world.hpp>

#include <gtest/gtest.h>

namespace cpp {

TEST(HelloWorldTest, ReturnsOne) {
  ASSERT_EQ(hello_world(), 1);
}

}  // namespace cpp
