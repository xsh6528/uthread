#include <uthread/gtest.hpp>
#include <uthread/uthread.hpp>

namespace uthread {

class IoTest : public ::testing::TestWithParam<Io::Timer> {};

TEST(IoTest, IoShutsDownIfNoReadyThreads) {
  Executor exe;
  Io io(&exe);
  exe.run();
}

TEST_P(IoTest, SleepsForApproximateTime) {
  static constexpr auto kSleepTime =  std::chrono::milliseconds(100);

  Executor exe;
  Io io(&exe);

  exe.add([&]() {
    ASSERT_TAKES_APPROX_MS(io.sleep_for(kSleepTime, GetParam()), 100);
  });

  ASSERT_TAKES_APPROX_MS(exe.run(), 100);
}

INSTANTIATE_TEST_CASE_P(
  SleepTimers,
  IoTest,
  ::testing::Values(Io::Timer::Libevent, Io::Timer::CpuLoop));

}
