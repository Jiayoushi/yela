#ifndef YELA_STOPPABLE_H_
#define YELA_STOPPABLE_H_

#include <future>

namespace yela {

class Stoppable {
 public:
  Stoppable();

  virtual void Run() = 0;
  void Stop();
  bool StopRequested();
 private:
  std::promise<void> exit_signal_;
  std::future<void> future_object_;
};

}

#endif
