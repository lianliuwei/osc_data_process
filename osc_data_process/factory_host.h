#pragma once

#include "osc_data_process/observer_cross_thread.h"

#include "osc_data_process/factory.h"

namespace {
class FactoryStub;
}
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
  virtual ~FactoryHost() {}

  int GetReceiveNum() const {
    return receive_;
  }

private:
  void OnDataProduced(scoped_refptr<FactoryData> data);

  // the Factory is be Destroyed.
  void OnFactoryDestory();

  virtual Stub* CreateStub();

  scoped_refptr<FactoryData> data_;

  int receive_;
};

