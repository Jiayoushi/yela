#include "stoppable.h"

#include <iostream>

namespace yela {

Stoppable::Stoppable():
  future_object_(exit_signal_.get_future()) {

}

bool Stoppable::StopRequested() {
  return future_object_.wait_for(std::chrono::milliseconds(0)) != 
         std::future_status::timeout;
}

void Stoppable::Stop() {
  exit_signal_.set_value();
}

}
