#pragma once

#include "base/memory/ref_counted.h"
#include "base/single_thread_task_runner.h"

// ObserverCrossThread is using to create the observer the cross, the 
// ObserverCrossThread must live long then the stub object.
// the ObserverCrossThread is thread-safe refcount. 
// Stub is create and destroy on the task thread.
class ObserverCrossThread 
    : public base::RefCountedThreadSafe<ObserverCrossThread> {
public:
  ObserverCrossThread(base::SingleThreadTaskRunner* task_thread, 
                      base::SingleThreadTaskRunner* host_thread);
  virtual ~ObserverCrossThread();

  virtual void Init();
  void IsInit();
  virtual void Destroy();
  void IsDestroyed();

  base::SingleThreadTaskRunner* task_thread() const { return task_thread_; }

  base::SingleThreadTaskRunner* host_thread() const { return host_thread_; }

  class Stub {
    public:
      Stub(ObserverCrossThread* observer);
      virtual ~Stub() {};

      void Destroy();

    protected:
      scoped_refptr<ObserverCrossThread> observer_;
  };

protected:
  void PostInit();
  void ReplyInit();

  void PostDestroy();
  void ReplyDestroy();

  // call on the task thread.
  // param transfer by ObserverCrossThread.
  virtual Stub* CreateStub() = 0;

  // set task and host before Init
  void set_task_thread(base::SingleThreadTaskRunner* task_thread);

  void set_host_thread(base::SingleThreadTaskRunner* host_thread);
  

  Stub* stub() const { return stub_; }

  bool init() const { return init_; }

  bool destroy() const { return destroy_; }

private:
  Stub* stub_;
  
  base::SingleThreadTaskRunner* task_thread_;

  base::SingleThreadTaskRunner* host_thread_;

  bool init_;
  bool destroy_;
};

