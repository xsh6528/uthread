#include <gtest/gtest.h>

#include <uthread/uthread.hpp>

namespace uthread {

TEST(ThreadTest, RunSingleThread) {
  int x = 0;

  auto f = Thread::create([&]() {
    ASSERT_EQ(x, 0);
    x = 1;
  });

  f->run();

  ASSERT_EQ(f->status(), Thread::Status::Finished);
  ASSERT_EQ(x, 1);
}

TEST(ThreadTest, SwitchBetweenThreads) {
  int x = 0;
  std::unique_ptr<Thread> f;
  std::unique_ptr<Thread> g;

  f = Thread::create([&]() {
    ASSERT_EQ(f->status(), Thread::Status::Running);
    ASSERT_EQ(g->status(), Thread::Status::Waiting);
    ASSERT_EQ(x, 0);
    f->swap(g.get());
    ASSERT_EQ(f->status(), Thread::Status::Running);
    ASSERT_EQ(g->status(), Thread::Status::Waiting);
    ASSERT_EQ(x, 1);
    f->swap(g.get());
    ASSERT_EQ(f->status(), Thread::Status::Running);
    ASSERT_EQ(g->status(), Thread::Status::Waiting);
    ASSERT_EQ(x, 2);
  });

  g = Thread::create([&]() {
    ASSERT_EQ(f->status(), Thread::Status::Waiting);
    ASSERT_EQ(g->status(), Thread::Status::Running);
    ASSERT_EQ(x, 0);
    x = 1;
    g->swap(f.get());
    ASSERT_EQ(f->status(), Thread::Status::Waiting);
    ASSERT_EQ(g->status(), Thread::Status::Running);
    ASSERT_EQ(x, 1);
    x = 2;
    g->swap(f.get());
    ASSERT_EQ(f->status(), Thread::Status::Finished);
    ASSERT_EQ(g->status(), Thread::Status::Running);
    ASSERT_EQ(x, 2);
    x = 3;
  });

  f->run();

  // g calls the last swap so f finishes but g doesn't
  ASSERT_EQ(f->status(), Thread::Status::Finished);
  ASSERT_EQ(g->status(), Thread::Status::Waiting);
  ASSERT_EQ(x, 2);

  g->run();
  ASSERT_EQ(g->status(), Thread::Status::Finished);

  ASSERT_EQ(x, 3);
}

}
