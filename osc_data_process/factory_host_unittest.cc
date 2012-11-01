// #include <vld.h>

#include "base/memory/scoped_ptr.h"
#include "base/bind.h"
#include "base/tracked_objects.h"
#include "base/threading/thread.h"

#include "testing/gtest/include/gtest/gtest.h"

#include "osc_data_process/factory.h"
#include "osc_data_process/factory_host.h"
#include "osc_data_process/freq_factory_host.h"

using namespace base;
namespace {

// FreqFactoryHost receive data freq.
static const int kHz = 351;

// factory run second.
static const int kFactoryRun = 2;

// if FactoryHost remove first, remove at this second.
static const int kFactoryHostRemove = 1;

class TestFactoryObserver : public FactoryObserver {
public:
  TestFactoryObserver()
    : count_(0) {}
  virtual ~TestFactoryObserver() {}

  virtual void OnDataProduced(scoped_refptr<FactoryData> data) {
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
  virtual FactoryHost* CreateFactoryHost() {
    return new FactoryHost(factory_loop_, host_loop_);
  }

  virtual ~FactoryHostTest() {}

  void CreateHost() {
    host_ = CreateFactoryHost();
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
          TimeDelta::FromSeconds(kFactoryHostRemove));
    factory_loop_->PostDelayedTask(FROM_HERE, 
        Bind(&FactoryHostTest::StopAndDestroy, Unretained(this)), 
        TimeDelta::FromSeconds(kFactoryRun));
  }

  void RemoveHost() {
    host_->Destroy();
  }

  void StopAndDestroy() {
    Factory::GetFactory()->RemoveObserver(observer_.get());
    Factory::GetFactory()->Destroy();
    if (remove_host_first_)
      host_loop_->PostTask(FROM_HERE, Bind(&FactoryHostTest::AssertHostRemoveFirst, 
          Unretained(this)));
    else
      host_loop_->PostTask(FROM_HERE, Bind(&FactoryHostTest::AssertFactoryDestroyFirst, 
          Unretained(this)));
    host_loop_->PostTask(FROM_HERE, MessageLoop::QuitClosure());
  } 

  virtual void AssertFactoryDestroyFirst() {
    LOG(INFO) << "Factory create " << observer_->count_ << " data";
    EXPECT_EQ(observer_->count_, host_->receive());
    EXPECT_EQ(host_->destroy_time()== 0 || host_->destroy_time() == 1, true);
    EXPECT_EQ(host_->factory_destroy(), 1);
    host_ = NULL;
  }

  virtual void AssertHostRemoveFirst() {
    EXPECT_EQ(host_->destroy_time(), 1);
    EXPECT_EQ(host_->factory_destroy(), 0);
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

  scoped_refptr<FactoryHost> host_;
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

class FreqFactoryHostTest : public FactoryHostTest {
public:
  virtual FactoryHost* CreateFactoryHost() {
    // receive the data at 60Hz
    return new FreqFactoryHost(factory_loop_, host_loop_, kHz);
  }
  
  virtual ~FreqFactoryHostTest() {}

  virtual void AssertFactoryDestroyFirst() {
    LOG(INFO) << "Factory create " << observer_->count_ << " data.";
    LOG(INFO) << "FreqFactoryHost receive: " << host_->receive() << " at " 
      << kHz << "Hz.";
    // give or take 3 data.
    EXPECT_LE(kFactoryRun*kHz - 3, host_->receive());
    EXPECT_GE(kFactoryRun*kHz + 3, host_->receive());
    EXPECT_EQ(host_->destroy_time() == 0 || host_->destroy_time() == 1, true);
    EXPECT_EQ(host_->factory_destroy(), 1);
    host_ = NULL;
  }

};

TEST_F(FreqFactoryHostTest, TestFreqReceiveProductData) {
  remove_host_first_ = false;
  RunFactoryHostTest();
}

TEST_F(FreqFactoryHostTest, TestFactoryHostRemoveObserverFirst) {
  remove_host_first_ = true;
  RunFactoryHostTest();
}