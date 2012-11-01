#pragma once

#include "osc_data_process/observer_cross_thread.h"

#include "osc_data_process/factory.h"

// run on each thread the listen to the Factory.
// can only be call on the listener thread.
// has a ref to data. the data may be the latest or may not. it depend on the 
// FactoryHost subclass.
// using the same observer as the factory.
class FactoryHost : public ObserverCrossThread {
public:
  friend class FactoryStub;

  FactoryHost(TaskRunnerType* task_thread, 
              TaskRunnerType* host_thread);
  virtual ~FactoryHost();

  int receive() const {
    return receive_;
  }
  int destroy_time() const { return destroy_time_; }
  int factory_destroy() const { return factory_destroy_; }


protected:
  // ObserverCrossThread Implement
  virtual Stub* CreateStub() OVERRIDE;

  virtual void OnDataProduced(scoped_refptr<FactoryData> data);

  // the Factory is be Destroyed.
  virtual void OnFactoryDestory();

  virtual void OnDestroy();
private:

  scoped_refptr<FactoryData> data_;

  int receive_;
  int destroy_time_;
  int factory_destroy_;
};

// this class watch the Factory change and notify the Factory Host.
// when the Detach() is call, the Factory may be Destoryed.
class FactoryStub : public ObserverCrossThread::Stub
                  , public FactoryObserver {
public:
  FactoryStub(Factory* factory, FactoryHost* host);
  virtual ~FactoryStub() {
    if (factory_ && factory_->HasObserver(this))
      factory_->RemoveObserver(this);
  }

protected:
  // factoryObserver Implement
  virtual void OnDataProduced(scoped_refptr<FactoryData> data);

  virtual void OnFactoryDestroy();
  
private:
  FactoryHost* AsFactoryHost() {
    return static_cast<FactoryHost*>(observer_.get());
  }

  Factory* factory_;
};
