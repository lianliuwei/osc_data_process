#pragma once

#include "osc_data_process/factory_host.h"

class FreqFactoryHost : public FactoryHost {
 public:
  FreqFactoryHost(TaskRunnerType* task_thread, 
                  TaskRunnerType* host_thread,
                  int freq);
  virtual ~FreqFactoryHost() {}

 protected:
   virtual Stub* CreateStub() OVERRIDE;

 private:
  int init_freq_;
};

class FreqFactoryHostStub : public FactoryStub {
 public:
  FreqFactoryHostStub(Factory* factory, FactoryHost* host);
  virtual ~FreqFactoryHostStub() {}

  void set_freq(int freq) { freq_ = freq; }
  int freq() const { return freq_; }

 protected:
   // FactoryStub Implement
   virtual void OnDataProduced(scoped_refptr<FactoryData> data) OVERRIDE;

   // virtual void OnFactoryDestroy() OVERRIDE;

 private:
  // unit is Hz.
  int freq_;
  base::Time last_notify_time_;
};
