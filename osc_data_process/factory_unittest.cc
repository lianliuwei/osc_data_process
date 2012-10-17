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

  virtual void OnDataProduced(scoped_refptr<FactoryData> data)  {
    ++count_;
  }

  virtual void OnFactoryDestroy()  {
  }

int count_;
};

void CreateFactory() {
  new Factory(MessageLoopProxy::current().get());
  EXPECT_TRUE(Factory::GetFactory());
}
}

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
    host_ = new FactoryHost(factory_loop_, host_loop_);
    host_->Init();
    factory_loop_->PostTask(FROM_HERE, 
        Bind(&FactoryHostTest::StartAndWatch, Unretained(this)));
    factory_loop_->PostDelayedTask(FROM_HERE, 
        Bind(&FactoryHostTest::StopAndDestroy, Unretained(this)), 
        TimeDelta::FromSeconds(2));
  }

  void StartAndWatch() {
    ASSERT_TRUE(observer_.get());
    Factory::GetFactory()->AddObserver(observer_.get());
    Factory::GetFactory()->Start();
  }

  void StopAndDestroy() {
    Factory::GetFactory()->RemoveObserver(observer_.get());
    Factory::GetFactory()->Destroy();
    host_loop_->PostTask(FROM_HERE, Bind(&FactoryHostTest::AssertDataNum, 
      Unretained(this)));
  } 

  void AssertDataNum() {
    EXPECT_EQ(observer_->count_, host_->GetReceiveNum());
    host_ = NULL;
    observer_.release();
    MessageLoop::current()->Quit();
  }

  scoped_refptr<FactoryHost> host_;
  scoped_ptr<TestFactoryObserver> observer_;
  scoped_refptr<MessageLoopProxy> host_loop_;
  scoped_refptr<MessageLoopProxy> factory_loop_;
};


TEST_F(FactoryHostTest, TestNoLostProductData) {
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
