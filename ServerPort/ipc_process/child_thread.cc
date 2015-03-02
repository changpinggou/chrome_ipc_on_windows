// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "child_thread.h"
#include "child_process.h"
#include "base/allocator/allocator_extension.h"
#include "base/command_line.h"
#include "base/message_loop.h"
#include "base/process.h"
#include "base/process_util.h"
#include "base/string_util.h"
#include "base/tracked_objects.h"
#include "ipc/ipc_logging.h"
#include "ipc/ipc_switches.h"
#include "ipc/ipc_sync_channel.h"
#include "ipc/ipc_sync_message_filter.h"
#include "ipc/ipc_message_macros.h"
#include "ipc/ipc_sync_channel.h"


using tracked_objects::ThreadData;



// How long to wait for a connection to the browser process before giving up.
const int kConnectionTimeoutS = 15;

ChildThread::ChildThread()
    : ALLOW_THIS_IN_INITIALIZER_LIST(channel_connected_factory_(this)) {
  channel_name_ = CommandLine::ForCurrentProcess()->GetSwitchValueASCII(
      switches::kProcessChannelID);
  Init();
}

ChildThread::ChildThread(const std::string& channel_name)
    : channel_name_(channel_name),
      ALLOW_THIS_IN_INITIALIZER_LIST(channel_connected_factory_(this)) {
  Init();
}

void ChildThread::Init() {
  on_channel_error_called_ = false;
  message_loop_ = MessageLoop::current();
  channel_.reset(new IPC::SyncChannel(channel_name_,
      IPC::Channel::MODE_CLIENT, this,
      ChildProcess::current()->io_message_loop_proxy(), true,
      ChildProcess::current()->GetShutDownEvent()));
#ifdef IPC_MESSAGE_LOG_ENABLED
  IPC::Logging::GetInstance()->SetIPCSender(this);
#endif

//   resource_dispatcher_.reset(new ResourceDispatcher(this));
//   socket_stream_dispatcher_.reset(new SocketStreamDispatcher());
//   file_system_dispatcher_.reset(new FileSystemDispatcher());
//   quota_dispatcher_.reset(new QuotaDispatcher());

//   sync_message_filter_ =
//       new IPC::SyncMessageFilter(ChildProcess::current()->GetShutDownEvent());
//   thread_safe_sender_ =
//       new ThreadSafeSender(base::MessageLoopProxy::current(),
//                            sync_message_filter_);
//   histogram_message_filter_ = new ChildHistogramMessageFilter();

//   channel_->AddFilter(histogram_message_filter_.get());
//   channel_->AddFilter(sync_message_filter_.get());
//   channel_->AddFilter(new components::ChildTraceMessageFilter(
//       ChildProcess::current()->io_message_loop_proxy()));

  MessageLoop::current()->PostDelayedTask(
      FROM_HERE,
      base::Bind(&ChildThread::EnsureConnected,
                 channel_connected_factory_.GetWeakPtr()),
      base::TimeDelta::FromSeconds(kConnectionTimeoutS));
}

ChildThread::~ChildThread() {
#ifdef IPC_MESSAGE_LOG_ENABLED
  IPC::Logging::GetInstance()->SetIPCSender(NULL);
#endif

//   channel_->RemoveFilter(histogram_message_filter_.get());
//   channel_->RemoveFilter(sync_message_filter_.get());

  // The ChannelProxy object caches a pointer to the IPC thread, so need to
  // reset it as it's not guaranteed to outlive this object.
  // NOTE: this also has the side-effect of not closing the main IPC channel to
  // the browser process.  This is needed because this is the signal that the
  // browser uses to know that this process has died, so we need it to be alive
  // until this process is shut down, and the OS closes the handle
  // automatically.  We used to watch the object handle on Windows to do this,
  // but it wasn't possible to do so on POSIX.
  channel_->ClearIPCTaskRunner();
}

void ChildThread::OnChannelConnected(int32 peer_pid) {
  channel_connected_factory_.InvalidateWeakPtrs();
}

void ChildThread::OnChannelError() {
  set_on_channel_error_called(true);
  MessageLoop::current()->Quit();
}

bool ChildThread::Send(IPC::Message* msg) {
  DCHECK(MessageLoop::current() == message_loop());
  if (!channel_.get()) {
    delete msg;
    return false;
  }

  return channel_->Send(msg);
}

void ChildThread::AddRoute(int32 routing_id, IPC::Listener* listener) {
  DCHECK(MessageLoop::current() == message_loop());

  //router_.AddRoute(routing_id, listener);
}

void ChildThread::RemoveRoute(int32 routing_id) {
  DCHECK(MessageLoop::current() == message_loop());

  //router_.RemoveRoute(routing_id);
}

IPC::Listener* ChildThread::ResolveRoute(int32 routing_id) {
  DCHECK(MessageLoop::current() == message_loop());

  //return router_.ResolveRoute(routing_id);
  return NULL;
}

// webkit_glue::ResourceLoaderBridge* ChildThread::CreateBridge(
//     const webkit_glue::ResourceLoaderBridge::RequestInfo& request_info) {
//   return resource_dispatcher()->CreateBridge(request_info);
// }

base::SharedMemory* ChildThread::AllocateSharedMemory(size_t buf_size) {
  return AllocateSharedMemory(buf_size, this);
}

// static
base::SharedMemory* ChildThread::AllocateSharedMemory(
    size_t buf_size,
    IPC::Sender* sender) {
  scoped_ptr<base::SharedMemory> shared_buf;
#if defined(OS_WIN)
  shared_buf.reset(new base::SharedMemory);
  if (!shared_buf->CreateAndMapAnonymous(buf_size)) {
    NOTREACHED();
    return NULL;
  }
#else
  // On POSIX, we need to ask the browser to create the shared memory for us,
  // since this is blocked by the sandbox.
  base::SharedMemoryHandle shared_mem_handle;
  if (sender->Send(new ChildProcessHostMsg_SyncAllocateSharedMemory(
                           buf_size, &shared_mem_handle))) {
    if (base::SharedMemory::IsHandleValid(shared_mem_handle)) {
      shared_buf.reset(new base::SharedMemory(shared_mem_handle, false));
      if (!shared_buf->Map(buf_size)) {
        NOTREACHED() << "Map failed";
        return NULL;
      }
    } else {
      NOTREACHED() << "Browser failed to allocate shared memory";
      return NULL;
    }
  } else {
    NOTREACHED() << "Browser allocation request message failed";
    return NULL;
  }
#endif
  return shared_buf.release();
}

bool ChildThread::OnMessageReceived(const IPC::Message& msg) {
  // Resource responses are sent to the resource dispatcher.
//   if (resource_dispatcher_->OnMessageReceived(msg))
//     return true;
//   if (socket_stream_dispatcher_->OnMessageReceived(msg))
//     return true;
//   if (file_system_dispatcher_->OnMessageReceived(msg))
//     return true;
//   if (quota_dispatcher_->OnMessageReceived(msg))
//     return true;

  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(ChildThread, msg)
//     IPC_MESSAGE_HANDLER(ChildProcessMsg_Shutdown, OnShutdown)
// #if defined(IPC_MESSAGE_LOG_ENABLED)
//     IPC_MESSAGE_HANDLER(ChildProcessMsg_SetIPCLoggingEnabled,
//                         OnSetIPCLoggingEnabled)
// #endif
//     IPC_MESSAGE_HANDLER(ChildProcessMsg_SetProfilerStatus,
//                         OnSetProfilerStatus)
//     IPC_MESSAGE_HANDLER(ChildProcessMsg_GetChildProfilerData,
//                         OnGetChildProfilerData)
//     IPC_MESSAGE_HANDLER(ChildProcessMsg_DumpHandles, OnDumpHandles)
// #if defined(USE_TCMALLOC)
//     IPC_MESSAGE_HANDLER(ChildProcessMsg_GetTcmallocStats, OnGetTcmallocStats)
// #endif
//     IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  if (handled)
    return true;

  if (msg.routing_id() == MSG_ROUTING_CONTROL)
    return OnControlMessageReceived(msg);

  return true;

  //return router_.OnMessageReceived(msg);
}

bool ChildThread::OnControlMessageReceived(const IPC::Message& msg) {
  return false;
}

void ChildThread::OnShutdown() {
  MessageLoop::current()->Quit();
}

#if defined(IPC_MESSAGE_LOG_ENABLED)
void ChildThread::OnSetIPCLoggingEnabled(bool enable) {
  if (enable)
    IPC::Logging::GetInstance()->Enable();
  else
    IPC::Logging::GetInstance()->Disable();
}
#endif  //  IPC_MESSAGE_LOG_ENABLED

void ChildThread::OnSetProfilerStatus(ThreadData::Status status) {
  ThreadData::InitializeAndSetTrackingStatus(status);
}

void ChildThread::OnGetChildProfilerData(int sequence_number) {
  tracked_objects::ProcessDataSnapshot process_data;
  ThreadData::Snapshot(false, &process_data);

//   Send(new ChildProcessHostMsg_ChildProfilerData(sequence_number,
//                                                  process_data));
}

void ChildThread::OnDumpHandles() {
// #if defined(OS_WIN)
//   scoped_refptr<HandleEnumerator> handle_enum(
//       new HandleEnumerator(
//           CommandLine::ForCurrentProcess()->HasSwitch(
//               switches::kAuditAllHandles)));
//   handle_enum->EnumerateHandles();
//   Send(new ChildProcessHostMsg_DumpHandlesDone);
//   return;
// #endif

  NOTIMPLEMENTED();
}

// #if defined(USE_TCMALLOC)
// void ChildThread::OnGetTcmallocStats() {
//   std::string result;
//   char buffer[1024 * 32];
//   base::allocator::GetStats(buffer, sizeof(buffer));
//   result.append(buffer);
//   //Send(new ChildProcessHostMsg_TcmallocStats(result));
// }
// #endif

ChildThread* ChildThread::current() {
  return ChildProcess::current() ?
      ChildProcess::current()->main_thread() : NULL;
}

void ChildThread::OnProcessFinalRelease() {
  if (on_channel_error_called_) {
    MessageLoop::current()->Quit();
    return;
  }

  // The child process shutdown sequence is a request response based mechanism,
  // where we send out an initial feeler request to the child process host
  // instance in the browser to verify if it's ok to shutdown the child process.
  // The browser then sends back a response if it's ok to shutdown. This avoids
  // race conditions if the process refcount is 0 but there's an IPC message
  // inflight that would addref it.
  //Send(new ChildProcessHostMsg_ShutdownRequest);
}

void ChildThread::EnsureConnected() {
  LOG(INFO) << "ChildThread::EnsureConnected()";
  base::KillProcess(base::GetCurrentProcessHandle(), 0, false);
}