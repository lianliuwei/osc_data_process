// #include <vld.h>

#include "base/memory/scoped_ptr.h"
#include "base/bind.h"
#include "base/tracked_objects.h"
#include "base/threading/thread.h"

#include "testing/gtest/include/gtest/gtest.h"

#include "osc_data_process/factory.h"
#include "osc_data_process/factory_host.h"

using namespace base;
namespace {

class TestFactoryObserver : public FactoryObserver {
public:
  TestFactoryObserver()
    : count_(0) {}
  virtual ~TestFactoryObserver() {}

  virtual void OnDataProduced(scoped_refptr<FactoryData> data)  {
    ++count_;
  }

  virtual void OnFactoryDestroy() {
  }

int count_;
};

void CreateFactory() {
  new Factory(MessageLoopProxy::current().get());
  EXPECT_TRUE(Factory::GetFactory());
}
}

class FactoryHostStub : public FactoryHost {
public:
  FactoryHostStub (TaskRunnerType* task_thread, 
      TaskRunnerType* host_thread) 
      : FactoryHost(task_thread, host_thread)
      , destroy_time_(0) {}

  virtual ~FactoryHostStub() {}

  virtual void OnDestroy()  {
    ++destroy_time_;
  }

  int destroy_time_;
};

class FactoryHostTest : public testing::Test {

protected:
  virtual void SetUp() {
    observer_.reset(new TestFactoryObserver);
  }

  virtual void TearDown() {
    ASSERT_TRUE(host_.get() == NULL);
    ASSERT_TRUE(host_loop_.get() == NULL);
    ASSERT_TRUE(factory_loop_.get() == NULL);
  }

public:
  void CreateHost() {
    host_ = new FactoryHostStub(factory_loop_, host_loop_);
    host_->Init();
    factory_loop_->PostTask(FROM_HERE, 
        Bind(&FactoryHostTest::StartAndWatch, Unretained(this)));
  }

  void StartAndWatch() {
    ASSERT_TRUE(observer_.get());
    Factory::GetFactory()->AddObserver(observer_.get());
    Factory::GetFactory()->Start();
    if (remove_host_first_)
      host_loop_->PostDelayedTask(FROM_HERE,
          Bind(&FactoryHostTest::RemoveHost, Unretained(this)),
          TimeDelta::FromSeconds(1));
    factory_loop_->PostDelayedTask(FROM_HERE, 
        Bind(&FactoryHostTest::StopAndDestroy, Unretained(this)), 
        TimeDelta::FromSeconds(2));
  }

  void RemoveHost() {
    host_->Destroy();
  }

  void StopAndDestroy() {
    Factory::GetFactory()->RemoveObserver(observer_.get());
    Factory::GetFactory()->Destroy();
    if (remove_host_first_)
      host_loop_->PostTask(FROM_HERE, Bind(&FactoryHostTest::AssertHostDestroy, 
          Unretained(this)));
    else
      host_loop_->PostTask(FROM_HERE, Bind(&FactoryHostTest::AssertDataNum, 
          Unretained(this)));
    host_loop_->PostTask(FROM_HERE, MessageLoop::QuitClosure());
  } 

  void AssertDataNum() {
    LOG(INFO) << "Factory create " << observer_->count_ << " data";
    EXPECT_EQ(observer_->count_, host_->GetReceiveNum());
    EXPECT_EQ(host_->destroy_time_ == 0 || host_->destroy_time_ == 1, true);
    host_ = NULL;
  }

  void AssertHostDestroy() {
    EXPECT_EQ(host_->destroy_time_, 1);
    host_ = NULL;
  }

  void RunFactoryHostTest() {
    MessageLoop loop(MessageLoop::TYPE_DEFAULT);
    host_loop_ = loop.message_loop_proxy();

    Thread thread("Factory thread");
    thread.Start();
    factory_loop_ = thread.message_loop_proxy();
    factory_loop_->PostTaskAndReply(FROM_HERE, 
      Bind(&CreateFactory), 
      Bind(&FactoryHostTest::CreateHost, Unretained(this)));

    loop.Run();

    factory_loop_ = NULL;
    host_loop_ = NULL;
  }

  scoped_refptr<FactoryHostStub> host_;
  scoped_ptr<TestFactoryObserver> observer_;
  scoped_refptr<MessageLoopProxy> host_loop_;
  scoped_refptr<MessageLoopProxy> factory_loop_;
  // for test host remove itself first.
  bool remove_host_first_;
};

// NOTE start one thread leak 196 - 36 - 48 - 24 byte memory.
TEST_F(FactoryHostTest, TestNoLostProductData) {
  remove_host_first_ = false;
  RunFactoryHostTest();
}

TEST_F(FactoryHostTest, TestFactoryHostRemoveObserverFirst) {
  remove_host_first_ = true;
  RunFactoryHostTest();
}
