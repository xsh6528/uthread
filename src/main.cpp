#include <cpp/hello_world.hpp>

#include <glog/logging.h>

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);
  cpp::hello_world();
}
