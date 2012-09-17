#include "osc_data_process/factory.h"

#include "base/logging.h"
#include "base/stringprintf.h"
#include "base/bind.h"
#include "base/location.h"
#include "base/message_loop_proxy.h"

using namespace std;
using namespace base;

namespace {
static Factory* g_factory = NULL;
}


Factory* Factory::GetFactory() {
  DCHECK(g_factory) << "need create Factory first.";
  return g_factory;
}

void Factory::CreateFactory() {
  DCHECK(g_factory == NULL) << "CreateFactory must be call only once.";
  g_factory = new Factory();
  g_factory->factory_thread_ = MessageLoopProxy::current();
}

void Factory::DestroyFactory() {
  DCHECK(g_factory);
  g_factory->NotifyDestroy();
  delete g_factory;
  g_factory = NULL;
}

void Factory::StartFactory() {
  DCHECK(g_factory);
  g_factory->Start();
}

void Factory::StopFactory() {
  DCHECK(g_factory);
  g_factory->Stop();
}

void Factory::Start() {
  factory_thread_->BelongsToCurrentThread();
  if (start_) {
    LOG(WARNING) << "the Factory already started, do nothing.";
    return;
  }
  
  start_ = true;
  factory_thread_->PostTask(FROM_HERE, Bind(&Factory::Schedule, AsWeakPtr()));
}

void Factory::Stop() {
  factory_thread_->BelongsToCurrentThread();
  if (!start_) {
    LOG(WARNING) << "the Factory already stopped, do nothing.";
    return;
  }
  start_ = false;
}

void Factory::Schedule() {
  factory_thread_->BelongsToCurrentThread();

  if (!last_ticks_.is_null()) {
    TimeTicks now = TimeTicks::Now();
    int64 us = (now - last_ticks_).InMicroseconds();
    LOG(INFO) << "Schedule after: " << 
      us << " us";

    LOG_IF(INFO, us != 0) << "rate is " << 1e6 / us << " Hz";
  }
  last_ticks_ = TimeTicks::Now();

  // check if we need to stop Schedule.
  if (!start_)
    return;

  // post task before Produce(), so no delay is no add up Produce() time.
  // wait and run again.
  DCHECK(speed_ > 0);
  int64 us = static_cast<int64>(1e6 / speed_);
  factory_thread_->PostDelayedTask(FROM_HERE, 
    Bind(&Factory::Schedule, AsWeakPtr()), TimeDelta::FromMicroseconds(us));

  // now Produce one data.
  Produce();
}

void Factory::Produce() {
  static int id = 0;
  data_ = new FactoryData();
  data_->set_id(id);
  string data_string = StringPrintf("Data id:%d.", id);
  vector<char> data(data_string.begin(), data_string.end());
  data_->set_data(data);
  LOG(INFO) << "Produce data id: " << id << " time: " << 
    TimeTicks::Now().ToInternalValue();
  NotifyProduced();
  ++id;
}
// speed ad 1000 a second.
Factory::Factory()
    : start_(false)
    , speed_(1000) {
  data_ = new FactoryData();
  data_->set_id(-1);
}

Factory::~Factory() {}

FactoryData* Factory::GetLastData() const {
  return data_.get();
}





