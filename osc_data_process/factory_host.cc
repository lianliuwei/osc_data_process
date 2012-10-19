#include "osc_data_process/factory_host.h"

#include "base/logging.h"
#include "base/location.h"
#include "base/bind.h"
#include "base/single_thread_task_runner.h"

using namespace base;

namespace {

// this class watch the Factory change and notify the Factory Host.
// when the Detach() is call, the Factory may be Destoryed.
class FactoryStub : public ObserverCrossThread::Stub
                  , public FactoryObserver {
public:
  FactoryStub(Factory* factory, FactoryHost* host)
      : Stub(host)
      , factory_(factory) {
    scoped_refptr<FactoryData> data = factory_->GetLastData();
    // before init the data only be access at this.
    // so cross thread set data may fine.
    AsFactoryHost()->data_ = data;
    factory_->AddObserver(this);
  }
  virtual ~FactoryStub() {}

private:
  // factoryObserver Implement
  virtual void OnDataProduced(scoped_refptr<FactoryData> data) {
    // TODO do some limit and record there.
    observer_->host_thread()->
      PostTask(FROM_HERE, 
      Bind(&FactoryHost::OnDataProduced, AsFactoryHost(), data));
  }

  virtual void OnFactoryDestroy()  {
    factory_->RemoveObserver(this);
    factory_ = NULL;
    observer_->host_thread()->
      PostTask(FROM_HERE, 
        Bind(&FactoryHost::OnFactoryDestory, AsFactoryHost()));
    // obj no exit after Destroy()
    Destroy();
  }
  
  FactoryHost* AsFactoryHost() {
    return static_cast<FactoryHost*>(observer_.get());
  }
private:
  Factory* factory_;
};

}

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
}

FactoryHost::FactoryHost(TaskRunnerType* task_thread, 
                         TaskRunnerType* host_thread)
    : ObserverCrossThread(task_thread, host_thread)
    , receive_(0) {
}
