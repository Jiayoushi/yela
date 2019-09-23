#ifndef YELA_INTERRUPTIBLE_THREAD_H_
#define YELA_INTERRUPTIBLE_THREAD_H_

#include <thread>
#include <future>
#include <exception>

//
// Code from C++ Conccurency in Action
//

namespace yela {

class ThreadInterrupted: public std::exception {
  virtual const char *what() const throw() {
    return "ThreadInterrupted exception";
  }
};

class InterruptibleFlag {
 public:
  InterruptibleFlag();

  void Set();
  bool IsSet() const;

 private:
  bool set_;
};


void InterruptionPoint();

class InterruptibleThread {
 public:
  template <typename FunctionType, typename Object>
  InterruptibleThread(FunctionType f, Object *o);

  void Join();
  void Detach();
  bool Joinable() const;
  void Interrupt();

 private:
  std::thread internal_thread_;
  InterruptibleFlag *flag_;
};

thread_local InterruptibleFlag this_thread_interrupt_flag;

template <typename FunctionType, typename Object>
InterruptibleThread::InterruptibleThread(FunctionType f, Object *o) {
  // A flag is created, invalid and usable future is created
  std::promise<InterruptibleFlag *> p;
 
  internal_thread_ = std::thread([f, &p, o]{

    // Future will become valid when the promise is filled in with the data
    p.set_value(&this_thread_interrupt_flag);
    
    f();
  });

  // Block
  // Return a value or exception
  flag_ = p.get_future().get();
}

}

#endif
