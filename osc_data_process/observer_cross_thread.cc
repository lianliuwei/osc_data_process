#include "osc_data_process/observer_cross_thread.h"

#include "base/logging.h"
#include "base/location.h"
#include "base/bind.h"
#include "base/single_thread_task_runner.h"

using namespace base;

ObserverCrossThread::ObserverCrossThread(TaskRunnerType* task_thread, 
                                         TaskRunnerType* host_thread)
    : stub_(NULL) 
    , task_thread_(task_thread)
    , host_thread_(host_thread)
    , init_(false) 
    , destroy_(false) {
  DCHECK(task_thread_ != NULL) << "need task thread to create stub on it.";
  DCHECK(host_thread_ != NULL) << "need host thread to be notify by the stub";
}

void ObserverCrossThread::Init() {
  DCHECK(init_ == false) << "call init more then once.";
  task_thread_->PostTaskAndReply(FROM_HERE, 
    Bind(&ObserverCrossThread::PostInit, this), 
    Bind(&ObserverCrossThread::ReplyInit, this));
}

void ObserverCrossThread::PostInit() {
  DCHECK(stub_ == NULL) << "call init more then once.";
  stub_ = CreateStub();
  DCHECK(stub_ != NULL) << "create stub false.";
}

void ObserverCrossThread::ReplyInit() {
  DCHECK(destroy_ == false) << "destroy before init. very bad happen";
  init_ = true;
  // notify initialized.
  OnInit();
}

void ObserverCrossThread::Destroy() {
  DCHECK(init_ == true && destroy_ == false) 
    << "Destroy before Init or call Destroy more then once.";
  task_thread_->PostTaskAndReply(FROM_HERE, 
    Bind(&ObserverCrossThread::PostDestroy, this), 
    Bind(&ObserverCrossThread::ReplyDestroy, this));
}

void ObserverCrossThread::PostDestroy() {
  DCHECK(stub_ != NULL) << "no create stub before.";
  delete stub_;
  stub_ = NULL;
}

void ObserverCrossThread::ReplyDestroy() {
  destroy_ = true;
  // notify Destroyed.
  OnDestroy();
}

ObserverCrossThread::~ObserverCrossThread() {
  DCHECK((init_ == true && destroy_ == true) 
    || (init_ == false && destroy_ == false));
}

void ObserverCrossThread::set_task_thread(TaskRunnerType* task_thread) {
  DCHECK(init_ == false) << "can no set the task thread after init.";
  task_thread_ = task_thread;
}

void ObserverCrossThread::set_host_thread(TaskRunnerType* host_thread) {
  DCHECK(init_ == false) << "can no set the host thread after init.";
  host_thread_ = host_thread;
}


// let the host destroy us.
void ObserverCrossThread::Stub::Destroy() {
  observer_->host_thread()->PostTask(FROM_HERE,
    Bind(&ObserverCrossThread::Destroy, observer_));
}

ObserverCrossThread::Stub::Stub(ObserverCrossThread* observer)
    : observer_(observer) {
  DCHECK(observer) << "need observer to notify.";
}
