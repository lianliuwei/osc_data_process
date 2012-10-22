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

void Factory::Start() {
  factory_thread_->BelongsToCurrentThread();
  if (start_) {
    LOG(WARNING) << "the Factory already started, do nothing.";
    return;
  }

  start_ = true;
  factory_thread_->PostTask(FROM_HERE, 
      Bind(&Factory::Schedule, weak_factory_.GetWeakPtr()));
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
    VLOG(1) << "Schedule after: " <<
      us << " us";

    VLOG_IF(1, us != 0) << "rate is " << 1e6 / us << " Hz";
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
      Bind(&Factory::Schedule, weak_factory_.GetWeakPtr()), 
      TimeDelta::FromMicroseconds(us));

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
  VLOG(1) << "Produce data id: " << id << " time: " <<
    TimeTicks::Now().ToInternalValue();
  NotifyProduced();
  ++id;
}
// speed ad 1000 a second.
Factory::Factory(SingleThreadTaskRunner* factory_thread)
    : factory_thread_(factory_thread)
    , start_(false)
    , speed_(1000)
    , ALLOW_THIS_IN_INITIALIZER_LIST(weak_factory_(this)) {
  DCHECK(factory_thread);
  data_ = new FactoryData();
  data_->set_id(-1);
  g_factory = this;
}

Factory::~Factory() {
  g_factory = NULL;
}



void Factory::Destroy() {
  NotifyDestroy();
  weak_factory_.InvalidateWeakPtrs();
  delete this;
}

FactoryData* Factory::GetLastData() const {
  return data_.get();
}





