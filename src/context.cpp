#include <uthread/context.hpp>

namespace uthread {

void context_swap(Context *current, Context const *other) {
  if (context_get(current) == Snapshot::SNAPSHOT) {
    context_set(other);
  }
}

}
