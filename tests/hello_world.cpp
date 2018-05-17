#include <cpp/hello_world.h>

#include <gtest/gtest.h>

namespace cpp {

TEST(HelloWorldTest, One) {
  ASSERT_EQ(hello_world_1(), 1);
}

TEST(HelloWorldTest, Two) {
  ASSERT_EQ(hello_world_2(), 2);
}

}  // namespace cpp
