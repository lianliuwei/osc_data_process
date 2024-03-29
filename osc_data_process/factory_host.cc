#include "osc_data_process/factory_host.h"

#include "base/logging.h"
#include "base/location.h"
#include "base/bind.h"
#include "base/single_thread_task_runner.h"

using namespace base;

ObserverCrossThread::Stub* FactoryHost::CreateStub() {
  Factory* factory = Factory::GetFactory();
  DCHECK(factory) << "need call this after the factory Created";
  FactoryStub* stub = new FactoryStub(factory, this);
  return stub;
}

void FactoryHost::OnDataProduced(scoped_refptr<FactoryData> data) {
  VLOG(1) << "Get Data. id: " << data->id();
  data_ = data;
  ++receive_;
}

void FactoryHost::OnFactoryDestory() {
  VLOG(1) << "Factory Destroy.";
  ++factory_destroy_;
}

void FactoryHost::OnDestroy() {
  ++destroy_time_;
}

FactoryHost::FactoryHost(TaskRunnerType* task_thread, 
                         TaskRunnerType* host_thread)
    : ObserverCrossThread(task_thread, host_thread)
    , receive_(0)
    , destroy_time_(0)
    , factory_destroy_(0) {
}

FactoryHost::~FactoryHost() {
}


FactoryStub::FactoryStub(Factory* factory, FactoryHost* host) 
    : Stub(host)
    , factory_(factory) {
  scoped_refptr<FactoryData> data = factory_->GetLastData();
  // before init the data only be access at this.
  // so cross thread set data may fine.
  AsFactoryHost()->data_ = data;
  factory_->AddObserver(this);
}

void FactoryStub::OnDataProduced(scoped_refptr<FactoryData> data) {
  // TODO do some limit and record there.
  observer_->host_thread()->PostTask(FROM_HERE, 
      Bind(&FactoryHost::OnDataProduced, AsFactoryHost(), data));
}

void FactoryStub::OnFactoryDestroy() {
  factory_->RemoveObserver(this);
  factory_ = NULL;
  observer_->host_thread()->PostTask(FROM_HERE, 
      Bind(&FactoryHost::OnFactoryDestory, AsFactoryHost()));
  // obj no exit after Destroy()
  Destroy();
}
