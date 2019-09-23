#include "interruptible_thread.h"

#include <iostream>

namespace yela {

InterruptibleFlag::InterruptibleFlag():
  set_(false) {

}

void InterruptibleFlag::Set() {
  set_ = true;
}

bool InterruptibleFlag::IsSet() const {
  return set_;
}

void InterruptionPoint() {
  if (this_thread_interrupt_flag.IsSet()) {
    throw ThreadInterrupted();
  }
}

void InterruptibleThread::Interrupt() {
  if (flag_ != nullptr) {
    flag_->Set();
  }
}

void InterruptibleThread::Join() {
  internal_thread_.join();
}

void InterruptibleThread::Detach() {
  internal_thread_.detach();
}

bool InterruptibleThread::Joinable() const {
  return internal_thread_.joinable();
}

}
