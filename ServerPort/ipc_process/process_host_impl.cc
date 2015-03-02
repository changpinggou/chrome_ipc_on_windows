// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Represents the browser side of the browser <--> renderer communication
// channel. There will be one RenderProcessHost per renderer process.

#include "process_host_impl.h"

#include <algorithm>
#include <limits>
#include <vector>

#if defined(OS_POSIX)
#include <utility>  // for pair<>
#endif

#include "base/base_switches.h"
#include "base/bind.h"
#include "base/bind_helpers.h"
#include "base/callback.h"
#include "base/command_line.h"
#include "base/debug/trace_event.h"
#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/metrics/field_trial.h"
#include "base/metrics/histogram.h"
#include "base/path_service.h"
#include "base/platform_file.h"
#include "base/process_util.h"
#include "base/rand_util.h"
#include "base/stl_util.h"
#include "base/string_util.h"
#include "base/supports_user_data.h"
#include "base/sys_info.h"
#include "base/threading/thread.h"
#include "base/threading/thread_restrictions.h"
#include "base/tracked_objects.h"
#include "ipc_process/browser_thread.h"
#include "ipc/ipc_switches.h"

#if defined(OS_WIN)
#include "base/win/scoped_com_initializer.h"
#endif


extern bool g_exited_main_message_loop;

static const char* kSiteProcessMapKeyName = "content_site_process_map";
// static
bool g_run_renderer_in_process_ = false;

// static
// void RenderProcessHost::SetMaxRendererProcessCount(size_t count) {
//   g_max_renderer_count_override = count;
// }

ProcessHostImpl::ProcessHostImpl() : is_initialized_(false){
}

ProcessHostImpl::~ProcessHostImpl() {
}

void ProcessHostImpl::EnableSendQueue() {
  //is_initialized_ = false;
}

bool ProcessHostImpl::Init() {
  // calling Init() more than once does nothing, this makes it more convenient
  // for the view host which may not be sure in some cases
  if (channel_.get())
    return true;

  CommandLine::StringType renderer_prefix;
#if defined(OS_POSIX)
  // A command prefix is something prepended to the command line of the spawned
  // process. It is supported only on POSIX systems.
  const CommandLine& browser_command_line = *CommandLine::ForCurrentProcess();
  renderer_prefix =
      browser_command_line.GetSwitchValueNative(switches::kRendererCmdPrefix);
#endif  // defined(OS_POSIX)


  // Setup the IPC channel.
  const std::string channel_id =
      IPC::Channel::GenerateVerifiedChannelID(std::string());
  channel_.reset(
#if defined(OS_ANDROID)
      // Android WebView needs to be able to wait from the UI thread to support
      // the synchronous legacy APIs.
      browser_command_line.HasSwitch(switches::kEnableWebViewSynchronousAPIs) ?
          new IPC::SyncChannel(
              channel_id, IPC::Channel::MODE_SERVER, this,
              BrowserThread::GetMessageLoopProxyForThread(BrowserThread::IO),
              true, &dummy_shutdown_event_) :
#endif
      new IPC::ChannelProxy(
          channel_id, IPC::Channel::MODE_SERVER, this,
          BrowserThread::GetMessageLoopProxyForThread(BrowserThread::IO)));

  // Call the embedder first so that their IPC filters have priority.
  //GetContentClient()->browser()->RenderProcessHostCreated(this);

  //CreateMessageFilters();
     base::FilePath renderer_path(L"D:\\chrome27\\src\\build\\Debug\\kanpian.exe");
    // Build command line for renderer.  We call AppendRendererCommandLine()
    // first so the process type argument will appear first.
    CommandLine* cmd_line = new CommandLine(renderer_path);
//     if (!renderer_prefix.empty())
//       cmd_line->PrependWrapper(renderer_prefix);
//     AppendRendererCommandLine(cmd_line);
    cmd_line->AppendSwitchASCII(switches::kProcessChannelID, channel_id);

    // Spawn the child process asynchronously to avoid blocking the UI thread.
    // As long as there's no renderer prefix, we can use the zygote process
    // at this stage.

  is_initialized_ = true;
  return true;
}

// void ProcessHostImpl::CreateRenderViewMessageFilter(){
//   DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
//   MediaInternals* media_internals = MediaInternals::GetInstance();
//   scoped_refptr<RenderMessageFilter> render_message_filter(
//     new RenderMessageFilter(
//     GetID(),
//     NULL,
//     GetBrowserContext(),
//     GetBrowserContext()->GetRequestContextForRenderProcess(GetID()),
//     widget_helper_,
//     media_internals,
//     storage_partition_impl_->GetDOMStorageContext()));
//   channel_->AddFilter(render_message_filter);
// }

// void ProcessHostImpl::CreateMessageFilters() {
//   DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
//   channel_->AddFilter(new ResourceSchedulerFilter(GetID()));
//   MediaInternals* media_internals = MediaInternals::GetInstance();;
//   // Add BrowserPluginMessageFilter to ensure it gets the first stab at messages
//   // from guests.
//   if (supports_browser_plugin_) {
//     scoped_refptr<BrowserPluginMessageFilter> bp_message_filter(
//         new BrowserPluginMessageFilter(GetID(), IsGuest()));
//     channel_->AddFilter(bp_message_filter);
//   }
// 
//   scoped_refptr<RenderMessageFilter> render_message_filter(
//       new RenderMessageFilter(
//           GetID(),
// #if defined(ENABLE_PLUGINS)
//           PluginServiceImpl::GetInstance(),
// #else
//           NULL,
// #endif
//           GetBrowserContext(),
//           GetBrowserContext()->GetRequestContextForRenderProcess(GetID()),
//           widget_helper_,
//           media_internals,
//           storage_partition_impl_->GetDOMStorageContext()));
//   channel_->AddFilter(render_message_filter);
//   BrowserContext* browser_context = GetBrowserContext();
//   ResourceContext* resource_context = browser_context->GetResourceContext();
// 
//   ResourceMessageFilter* resource_message_filter = new ResourceMessageFilter(
//       GetID(), PROCESS_TYPE_RENDERER, resource_context,
//       storage_partition_impl_->GetAppCacheService(),
//       ChromeBlobStorageContext::GetFor(browser_context),
//       storage_partition_impl_->GetFileSystemContext(),
//       new RendererURLRequestContextSelector(browser_context, GetID()));
// 
//   channel_->AddFilter(resource_message_filter);
//   media::AudioManager* audio_manager = BrowserMainLoop::GetAudioManager();
//   MediaStreamManager* media_stream_manager =
//       BrowserMainLoop::GetMediaStreamManager();
//   channel_->AddFilter(new AudioInputRendererHost(audio_manager,
//                                                  media_stream_manager));
//   channel_->AddFilter(new AudioRendererHost(
//       GetID(), audio_manager, BrowserMainLoop::GetAudioMirroringManager(),
//       media_internals));
//   channel_->AddFilter(new VideoCaptureHost());
//   channel_->AddFilter(new AppCacheDispatcherHost(
//       storage_partition_impl_->GetAppCacheService(),
//       GetID()));
//   channel_->AddFilter(new ClipboardMessageFilter(browser_context));
//   channel_->AddFilter(
//       new DOMStorageMessageFilter(
//           GetID(),
//           storage_partition_impl_->GetDOMStorageContext()));
//   channel_->AddFilter(
//       new IndexedDBDispatcherHost(
//           GetID(),
//           storage_partition_impl_->GetIndexedDBContext()));
//   if (IsGuest()) {
//     channel_->AddFilter(GeolocationDispatcherHost::New(
//         GetID(), new BrowserPluginGeolocationPermissionContext()));
//   } else {
//     channel_->AddFilter(GeolocationDispatcherHost::New(
//         GetID(), browser_context->GetGeolocationPermissionContext()));
//   }
//   gpu_message_filter_ = new GpuMessageFilter(GetID(), widget_helper_.get());
//   channel_->AddFilter(gpu_message_filter_);
// #if defined(ENABLE_WEBRTC)
//   peer_connection_tracker_host_ = new PeerConnectionTrackerHost(GetID());
//   channel_->AddFilter(peer_connection_tracker_host_);
//   channel_->AddFilter(new MediaStreamDispatcherHost(GetID()));
// #endif
// #if defined(ENABLE_PLUGINS)
//   channel_->AddFilter(new PepperMessageFilter(GetID(), browser_context));
// #endif
// #if defined(ENABLE_INPUT_SPEECH)
//   channel_->AddFilter(new InputTagSpeechDispatcherHost(
//       IsGuest(), GetID(), storage_partition_impl_->GetURLRequestContext(),
//       browser_context->GetSpeechRecognitionPreferences()));
//   channel_->AddFilter(new SpeechRecognitionDispatcherHost(
//       GetID(), storage_partition_impl_->GetURLRequestContext(),
//       browser_context->GetSpeechRecognitionPreferences()));
// #endif
//   channel_->AddFilter(new FileAPIMessageFilter(
//       GetID(),
//       storage_partition_impl_->GetURLRequestContext(),
//       storage_partition_impl_->GetFileSystemContext(),
//       ChromeBlobStorageContext::GetFor(browser_context)));
//   channel_->AddFilter(new OrientationMessageFilter());
//   channel_->AddFilter(new FileUtilitiesMessageFilter(GetID()));
//   channel_->AddFilter(new MimeRegistryMessageFilter());
//   channel_->AddFilter(new DatabaseMessageFilter(
//       storage_partition_impl_->GetDatabaseTracker()));
// #if defined(OS_MACOSX)
//   channel_->AddFilter(new TextInputClientMessageFilter(GetID()));
// #elif defined(OS_WIN)
//   channel_->AddFilter(new FontCacheDispatcher());
// #endif
// 
//   SocketStreamDispatcherHost* socket_stream_dispatcher_host =
//       new SocketStreamDispatcherHost(GetID(),
//           new RendererURLRequestContextSelector(browser_context, GetID()),
//           resource_context);
//   channel_->AddFilter(socket_stream_dispatcher_host);
// 
//   channel_->AddFilter(
//       new WorkerMessageFilter(
//           GetID(),
//           resource_context,
//           WorkerStoragePartition(
//               storage_partition_impl_->GetURLRequestContext(),
//               storage_partition_impl_->GetMediaURLRequestContext(),
//               storage_partition_impl_->GetAppCacheService(),
//               storage_partition_impl_->GetQuotaManager(),
//               storage_partition_impl_->GetFileSystemContext(),
//               storage_partition_impl_->GetDatabaseTracker(),
//               storage_partition_impl_->GetIndexedDBContext()),
//           base::Bind(&RenderWidgetHelper::GetNextRoutingID,
//                      base::Unretained(widget_helper_.get()))));
// 
// #if defined(ENABLE_WEBRTC)
//   channel_->AddFilter(new P2PSocketDispatcherHost(resource_context));
// #endif
// 
//   channel_->AddFilter(new TraceMessageFilter());
//   channel_->AddFilter(new ResolveProxyMsgHelper(
//       browser_context->GetRequestContextForRenderProcess(GetID())));
//   channel_->AddFilter(new QuotaDispatcherHost(
//       GetID(),
//       storage_partition_impl_->GetQuotaManager(),
//       GetContentClient()->browser()->CreateQuotaPermissionContext()));
//   channel_->AddFilter(new GamepadBrowserMessageFilter());
//   channel_->AddFilter(new ProfilerMessageFilter(PROCESS_TYPE_RENDERER));
//   channel_->AddFilter(new HistogramMessageFilter());
//   channel_->AddFilter(new HyphenatorMessageFilter(this));
// }

int ProcessHostImpl::GetNextRoutingID() {
  return next_routing_id_.GetNext() + 1;
}


bool ProcessHostImpl::WaitForBackingStoreMsg(
    int render_widget_id,
    const base::TimeDelta& max_delay,
    IPC::Message* msg) {
  // The post task to this thread with the process id could be in queue, and we
  // don't want to dispatch a message before then since it will need the handle.
//   if (child_process_launcher_.get() && child_process_launcher_->IsStarting())
//     return false;
// 
//   return widget_helper_->WaitForBackingStoreMsg(render_widget_id,
//                                                 max_delay, msg);
      return true;
}

void ProcessHostImpl::ReceivedBadMessage() {
  if (run_renderer_in_process()) {
    // In single process mode it is better if we don't suicide but just
    // crash.
    CHECK(false);
  }
  NOTREACHED();
  base::KillProcess(GetHandle(), 3,
                    false);
}


base::ProcessHandle ProcessHostImpl::GetHandle() const {
  if (run_renderer_in_process())
    return base::Process::Current().handle();

  //if (!child_process_launcher_.get() || child_process_launcher_->IsStarting())
    return base::kNullProcessHandle;

 // return child_process_launcher_->GetHandle();
}

//static
bool ProcessHost::run_renderer_in_process(){
  return false;
}

bool ProcessHostImpl::FastShutdownIfPossible() {
//   if (run_renderer_in_process())
//     return false;  // Single process mode never shutdown the renderer.
// 
//   if (!GetContentClient()->browser()->IsFastShutdownPossible())
//     return false;
// 
//   if (!child_process_launcher_.get() ||
//       child_process_launcher_->IsStarting() ||
//       !GetHandle())
//     return false;  // Render process hasn't started or is probably crashed.
// 
//   // Test if there's an unload listener.
//   // NOTE: It's possible that an onunload listener may be installed
//   // while we're shutting down, so there's a small race here.  Given that
//   // the window is small, it's unlikely that the web page has much
//   // state that will be lost by not calling its unload handlers properly.
//   if (!SuddenTerminationAllowed())
//     return false;
// 
//   ProcessDied(false /* already_dead */);
//   fast_shutdown_started_ = true;
  return true;
}

void ProcessHostImpl::DumpHandles() {
// #if defined(OS_WIN)
//   Send(new ChildProcessMsg_DumpHandles());
//   return;
// #endif

  NOTIMPLEMENTED();
}


bool ProcessHostImpl::Send(IPC::Message* msg) {
//   if (!channel_.get()) {
//     if (!is_initialized_) {
//       queued_messages_.push(msg);
//       return true;
//     } else {
//       delete msg;
//       return false;
//     }
//   }
// 
//   if (child_process_launcher_.get() && child_process_launcher_->IsStarting()) {
//     queued_messages_.push(msg);
//     return true;
//   }

  return channel_->Send(msg);
}

bool ProcessHostImpl::OnMessageReceived(const IPC::Message& msg) {
  // If we're about to be deleted, or have initiated the fast shutdown sequence,
  // we ignore incoming messages.

//   if (deleting_soon_ || fast_shutdown_started_)
//     return false;
// 
//   mark_child_process_activity_time();
//   if (msg.routing_id() == MSG_ROUTING_CONTROL) {
//     // Dispatch control messages.
//     bool msg_is_ok = true;
//     IPC_BEGIN_MESSAGE_MAP_EX(ProcessHostImpl, msg, msg_is_ok)
//       IPC_MESSAGE_HANDLER(ChildProcessHostMsg_ShutdownRequest,
//                           OnShutdownRequest)
//       IPC_MESSAGE_HANDLER(ChildProcessHostMsg_DumpHandlesDone,
//                           OnDumpHandlesDone)
//       IPC_MESSAGE_HANDLER(ViewHostMsg_SuddenTerminationChanged,
//                           SuddenTerminationChanged)
//       IPC_MESSAGE_HANDLER(ViewHostMsg_UserMetricsRecordAction,
//                           OnUserMetricsRecordAction)
//       IPC_MESSAGE_HANDLER(ViewHostMsg_SavedPageAsMHTML, OnSavedPageAsMHTML)
//       // Adding single handlers for your service here is fine, but once your
//       // service needs more than one handler, please extract them into a new
//       // message filter and add that filter to CreateMessageFilters().
//       IPC_MESSAGE_UNHANDLED_ERROR()
//     IPC_END_MESSAGE_MAP_EX()
// 
//     if (!msg_is_ok) {
//       // The message had a handler, but its de-serialization failed.
//       // We consider this a capital crime. Kill the renderer if we have one.
//       LOG(ERROR) << "bad message " << msg.type() << " terminating renderer.";
//       RecordAction(UserMetricsAction("BadMessageTerminate_BRPH"));
//       ReceivedBadMessage();
//     }
//     return true;
//   }
// 
//   // Dispatch incoming messages to the appropriate RenderView/WidgetHost.
//   RenderWidgetHost* rwh = render_widget_hosts_.Lookup(msg.routing_id());
//   if (!rwh) {
//     if (msg.is_sync()) {
//       // The listener has gone away, so we must respond or else the caller will
//       // hang waiting for a reply.
//       IPC::Message* reply = IPC::SyncMessage::GenerateReply(&msg);
//       reply->set_reply_error();
//       Send(reply);
//     }
// 
//     // If this is a SwapBuffers, we need to ack it if we're not going to handle
//     // it so that the GPU process doesn't get stuck in unscheduled state.
//     bool msg_is_ok = true;
//     IPC_BEGIN_MESSAGE_MAP_EX(ProcessHostImpl, msg, msg_is_ok)
//       IPC_MESSAGE_HANDLER(ViewHostMsg_CompositorSurfaceBuffersSwapped,
//                           OnCompositorSurfaceBuffersSwappedNoHost)
//     IPC_END_MESSAGE_MAP_EX()
//     return true;
//   }
//   return RenderWidgetHostImpl::From(rwh)->OnMessageReceived(msg);
  return true;
}

void ProcessHostImpl::OnChannelConnected(int32 peer_pid) {
// #if defined(IPC_MESSAGE_LOG_ENABLED)
//   Send(new ChildProcessMsg_SetIPCLoggingEnabled(
//       IPC::Logging::GetInstance()->Enabled()));
// #endif
// 
//   tracked_objects::ThreadData::Status status =
//       tracked_objects::ThreadData::status();
//   Send(new ChildProcessMsg_SetProfilerStatus(status));
}

void ProcessHostImpl::OnChannelError() {
  ProcessDied(true /* already_dead */);
}

void ProcessHostImpl::ProcessDied(bool already_dead) {

}


int ProcessHostImpl::GetID() const {
  return id_;
}

bool ProcessHostImpl::HasConnection() const {
  return channel_.get() != NULL;
}


// void ProcessHostImpl::Attach(RenderWidgetHost* host,
//                                    int routing_id) {
//   render_widget_hosts_.AddWithID(host, routing_id);
// }


void ProcessHostImpl::Cleanup() {
  // When no other owners of this object, we can delete ourselves
    MessageLoop::current()->DeleteSoon(FROM_HERE, this);
    channel_.reset();
}

IPC::ChannelProxy* ProcessHostImpl::GetChannel() {
  return channel_.get();
}

bool ProcessHostImpl::FastShutdownStarted() const {
  //return fast_shutdown_started_;
  return true;
}

void ProcessHostImpl::OnDumpHandlesDone() {
  Cleanup();
}

// void ProcessHostImpl::OnProcessLaunched() {
//   // No point doing anything, since this object will be destructed soon.  We
//   // especially don't want to send the RENDERER_PROCESS_CREATED notification,
//   // since some clients might expect a RENDERER_PROCESS_TERMINATED afterwards to
//   // properly cleanup.
//   if (deleting_soon_)
//     return;
// 
//   if (child_process_launcher_.get()) {
//     if (!child_process_launcher_->GetHandle()) {
//       OnChannelError();
//       return;
//     }
// 
//     child_process_launcher_->SetProcessBackgrounded(backgrounded_);
//   }
// 
//   // NOTE: This needs to be before sending queued messages because
//   // ExtensionService uses this notification to initialize the renderer process
//   // with state that must be there before any JavaScript executes.
//   //
//   // The queued messages contain such things as "navigate". If this notification
//   // was after, we can end up executing JavaScript before the initialization
//   // happens.
//   NotificationService::current()->Notify(
//       NOTIFICATION_RENDERER_PROCESS_CREATED,
//       Source<RenderProcessHost>(this),
//       NotificationService::NoDetails());
// 
//   while (!queued_messages_.empty()) {
//     Send(queued_messages_.front());
//     queued_messages_.pop();
//   }
// }