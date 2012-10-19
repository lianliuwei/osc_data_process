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
  typedef base::SingleThreadTaskRunner TaskRunnerType;

  ObserverCrossThread(TaskRunnerType* task_thread, 
                      TaskRunnerType* host_thread);
  virtual ~ObserverCrossThread();

  void Init();
  void Destroy();

  TaskRunnerType* task_thread() const { return task_thread_; }

  TaskRunnerType* host_thread() const { return host_thread_; }

  class Stub {
   public:
    Stub(ObserverCrossThread* observer);
    virtual ~Stub() {}

    // obj no exist after Destroy()
    void Destroy();

   protected:
    scoped_refptr<ObserverCrossThread> observer_;
   
   private:
    DISALLOW_COPY_AND_ASSIGN(Stub);
  };

protected:
  virtual void OnInit() {}
  virtual void OnDestroy() {}

  void PostInit();
  void ReplyInit();

  void PostDestroy();
  void ReplyDestroy();

  // call on the task thread.
  // param transfer by ObserverCrossThread.
  virtual Stub* CreateStub() = 0;

  // set task and host before Init
  void set_task_thread(TaskRunnerType* task_thread);

  void set_host_thread(TaskRunnerType* host_thread);
  

  Stub* stub() const { return stub_; }

  bool init() const { return init_; }

  bool destroy() const { return destroy_; }

private:
  Stub* stub_;
  
  TaskRunnerType* task_thread_;

  TaskRunnerType* host_thread_;

  bool init_;
  bool destroy_;

  DISALLOW_COPY_AND_ASSIGN(ObserverCrossThread);
};

