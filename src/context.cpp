#include <uthread/context.hpp>

namespace uthread { namespace context {

void swap(Context *current, Context const *other) {
  if (snapshot(current) == Snapshot::SNAPSHOT) {
    run(other);
  }
}

}}
