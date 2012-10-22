#pragma once

#include "base/time.h"
#include "base/observer_list.h"
#include "base/memory/weak_ptr.h"
#include "base/single_thread_task_runner.h"

#include "osc_data_process/factory_data.h"

class FactoryObserver {
public:
  // call when the data produce
  virtual void OnDataProduced(scoped_refptr<FactoryData> data) = 0;

  // call when the Factory is going to Destroy.
  // the Factory is exist when the function is call.
  virtual void OnFactoryDestroy() = 0;

protected:
  virtual ~FactoryObserver() {}
};

// produce the osc data at a very high speed, more then 1 thousand a second.
// it run on the factory thread.
// only can call on the factory thread.
class Factory {
public:
  static Factory* GetFactory();

  Factory(base::SingleThreadTaskRunner* factory_thread);

  void Start();
  void Stop();

  // after call this the factory is delete itself.
  void Destroy();
  void set_speed(int speed);

  void AddObserver(FactoryObserver* obs) {
    observer_list_.AddObserver(obs);
  }

  void RemoveObserver(FactoryObserver* obs) {
    observer_list_.RemoveObserver(obs);
  }

  bool HasObserver(FactoryObserver* obs) {
    return observer_list_.HasObserver(obs);
  }
  FactoryData* GetLastData() const;

private:
  virtual ~Factory();

  // call produce or do nothing (go to sleep).
  void Schedule();
  void Produce();

  // notify the observer the data is produced.
  // the observer is run on the same thread.
  void NotifyProduced() {
    FOR_EACH_OBSERVER(FactoryObserver, observer_list_, OnDataProduced(data_));
  }

  void NotifyDestroy() {
    FOR_EACH_OBSERVER(FactoryObserver, observer_list_, OnFactoryDestroy());
  }

  bool start_;
  int speed_;

  // to log the real produce rate.
  base::TimeTicks last_ticks_;

  base::SingleThreadTaskRunner* factory_thread_;

  // the last produce data.
  scoped_refptr<FactoryData> data_;

  ObserverList<FactoryObserver> observer_list_;

  base::WeakPtrFactory<Factory> weak_factory_;
};
