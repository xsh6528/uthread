#include <cpp/hello_world.hpp>

#include <glog/logging.h>

namespace cpp {

int hello_world() {
  LOG(INFO) << "Hello World!";
  return 1;
}

}
