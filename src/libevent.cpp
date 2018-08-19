#include <uthread/libevent.hpp>

#include <glog/logging.h>

namespace uthread {

LibeventBase::LibeventBase() {
  auto config = event_config_new();
  CHECK_NOTNULL(config);

  CHECK_EQ(event_config_set_flag(config, EVENT_BASE_FLAG_NOLOCK), 0);
  CHECK_EQ(event_config_set_flag(config, EVENT_BASE_FLAG_PRECISE_TIMER), 0);

  base_ = event_base_new_with_config(config);
  CHECK_NOTNULL(base_);

  event_config_free(config);
}

LibeventBase::~LibeventBase() {
  event_base_free(base_);
}

event_base *LibeventBase::raw() {
  return base_;
}

}
