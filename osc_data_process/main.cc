#include <stdlib.h>
#include <crtdbg.h>

#include "base/threading/thread.h"
#include "base/at_exit.h"
#include "base/logging.h"
#include "base/command_line.h"
#include "base/memory/scoped_ptr.h"
#include "base/bind.h"
#include "base/tracked_objects.h"

#include "osc_data_process/factory.h"
#include "osc_data_process/factory_host.h"

using namespace base;

scoped_refptr<FactoryHost> g_host;

void CreateFactoryHost(SingleThreadTaskRunner* task) {
  g_host = new FactoryHost(task, MessageLoopProxy::current());
  g_host->Init();
}

int main(int argc, char** argv) {
  _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
  using namespace logging;
  tracked_objects::ThreadData::InitializeAndSetTrackingStatus(
    tracked_objects::ThreadData::DEACTIVATED);

  using namespace logging;

  AtExitManager atexit;
  CommandLine::Init(argc, argv);

  InitLogging(L"debug.log", LOG_TO_BOTH_FILE_AND_SYSTEM_DEBUG_LOG,
    LOCK_LOG_FILE, DELETE_OLD_LOG_FILE,
    DISABLE_DCHECK_FOR_NON_OFFICIAL_RELEASE_BUILDS);

  MessageLoop ml(MessageLoop::TYPE_DEFAULT);

  Thread thread("Factory thread");
  thread.Start();

  MessageLoop* messageloop = thread.message_loop();
  messageloop->PostTask(FROM_HERE, Bind(Factory::CreateFactory));
  messageloop->PostTask(FROM_HERE, Bind(Factory::StartFactory));
  messageloop->PostDelayedTask(FROM_HERE, Bind(Factory::StopFactory), 
    TimeDelta::FromSeconds(2));
  messageloop->PostDelayedTask(FROM_HERE, Bind(Factory::DestroyFactory), 
    TimeDelta::FromSeconds(3));

  MessageLoop::current()->PostTask(FROM_HERE, Bind(CreateFactoryHost, 
    thread.message_loop_proxy()));
  MessageLoop::current()->PostDelayedTask(FROM_HERE, 
     Bind(&MessageLoop::Quit, Unretained(MessageLoop::current())), 
     TimeDelta::FromSeconds(4));

  MessageLoop::current()->Run();
  // no scoped_refptr.release() just leave the obj no dec ref count.
  // g_host.release();
  g_host = NULL;
  thread.Stop();

  return 0;
}
