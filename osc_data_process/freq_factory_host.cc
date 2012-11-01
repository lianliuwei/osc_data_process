#include "osc_data_process/freq_factory_host.h"


FreqFactoryHost::FreqFactoryHost(TaskRunnerType* task_thread, 
                                 TaskRunnerType* host_thread,
                                 int freq)
    : FactoryHost(task_thread, host_thread)
    , init_freq_(freq) {
  DCHECK(freq > 0);
}

ObserverCrossThread::Stub* FreqFactoryHost::CreateStub() {
  Factory* factory = Factory::GetFactory();
  DCHECK(factory) << "need call this after the factory Created";
  FreqFactoryHostStub* stub = new FreqFactoryHostStub(factory, this);
  stub->set_freq(init_freq_);
  return stub;
}

// now just let the OnDataProduced trigger the notify. the delta is no so
// precise because the Factory may no by the Freq multiple, for more precise
// maybe use PostDelayTask().
void FreqFactoryHostStub::OnDataProduced(scoped_refptr<FactoryData> data) {
  bool notify = false;
  if (last_notify_time_.is_null()) {
    notify = true;
    last_notify_time_ = base::Time::Now();
  } else {
    int64 micro_second = static_cast<int64>(1.0 / freq_ * 1e6);
    base::TimeDelta delta = base::TimeDelta::FromMicroseconds(micro_second);
    if (base::Time::Now() >= last_notify_time_ + delta) {
      notify = true;
      // no now + delta, keep the freq.
      last_notify_time_ = last_notify_time_ +  delta;
    }
  }

  if (notify)
    FactoryStub::OnDataProduced(data);
}

FreqFactoryHostStub::FreqFactoryHostStub(Factory* factory, FactoryHost* host) 
    : FactoryStub(factory, host) {
}
