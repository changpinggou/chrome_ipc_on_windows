// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "child_process_launcher.h"

#include <utility>  // For std::pair.

#include "base/bind.h"
#include "base/command_line.h"
#include "base/file_util.h"
#include "base/logging.h"
#include "base/memory/scoped_ptr.h"
#include "base/metrics/histogram.h"
#include "base/process_util.h"
#include "base/synchronization/lock.h"
#include "base/threading/thread.h"
#include "browser_thread.h"

#if defined(OS_WIN)
#include "base/files/file_path.h"
#elif defined(OS_MACOSX)
#include "content/browser/mach_broker_mac.h"
#elif defined(OS_ANDROID)
#include "base/android/jni_android.h"
#include "content/browser/android/child_process_launcher_android.h"
#elif defined(OS_POSIX)
#include "base/memory/singleton.h"
#include "content/browser/renderer_host/render_sandbox_host_linux.h"
#include "content/browser/zygote_host/zygote_host_impl_linux.h"
#endif

#if defined(OS_POSIX)
#include "base/posix/global_descriptors.h"
#endif

// Having the functionality of ChildProcessLauncher be in an internal
// ref counted object allows us to automatically terminate the process when the
// parent class destructs, while still holding on to state that we need.
class ChildProcessLauncher::Context
    : public base::RefCountedThreadSafe<ChildProcessLauncher::Context> {
 public:
  Context() : client_(NULL),
              client_thread_id_(BrowserThread::UI),
              termination_status_(base::TERMINATION_STATUS_NORMAL_TERMINATION),
              exit_code_(0),
              starting_(true){
                terminate_child_on_shutdown_ = true;
  }

  void Launch(CommandLine* cmd_line, int child_process_id, ChildProcessLauncher::Client* client) {
    client_ = client;
    //CHECK(BrowserThread::GetCurrentThreadIdentifier(&client_thread_id_));

    base::ProcessHandle handle = 0;
    base::LaunchProcess(*cmd_line, base::LaunchOptions(), &handle);
    starting_ = false;
    process_.set_handle(handle);
    if (!handle)
      LOG(ERROR) << "Failed to launch child process";
    if (client_) {
      client_->OnProcessLaunched();
    } else {
      Terminate();
    }
  }

  void ResetClient() {
    // No need for locking as this function gets called on the same thread that
    // client_ would be used.
    CHECK(BrowserThread::CurrentlyOn(client_thread_id_));
    client_ = NULL;
  }

  void set_terminate_child_on_shutdown(bool terminate_on_shutdown) {
    terminate_child_on_shutdown_ = terminate_on_shutdown;
  }

 private:
  friend class base::RefCountedThreadSafe<ChildProcessLauncher::Context>;
  friend class ChildProcessLauncher;

  ~Context() {
    Terminate();
  }

  static void RecordHistograms(const base::TimeTicks begin_launch_time) {
    base::TimeDelta launch_time = base::TimeTicks::Now() - begin_launch_time;
    if (BrowserThread::CurrentlyOn(BrowserThread::PROCESS_LAUNCHER)) {
      RecordLaunchHistograms(launch_time);
    } else {
      BrowserThread::PostTask(
          BrowserThread::PROCESS_LAUNCHER, FROM_HERE,
          base::Bind(&ChildProcessLauncher::Context::RecordLaunchHistograms,
                     launch_time));
    }
  }

  static void RecordLaunchHistograms(const base::TimeDelta launch_time) {
    // Log the launch time, separating out the first one (which will likely be
    // slower due to the rest of the browser initializing at the same time).
    static bool done_first_launch = false;
    if (done_first_launch) {
      UMA_HISTOGRAM_TIMES("MPArch.ChildProcessLaunchSubsequent", launch_time);
    } else {
      UMA_HISTOGRAM_TIMES("MPArch.ChildProcessLaunchFirst", launch_time);
      done_first_launch = true;
    }
  }

  static void LaunchInternal(
      // |this_object| is NOT thread safe. Only use it to post a task back.
      Context* this_object,
      BrowserThread::ID client_thread_id,
      int child_process_id,
      CommandLine* cmd_line) {
    scoped_ptr<CommandLine> cmd_line_deleter(cmd_line);
    base::TimeTicks begin_launch_time = base::TimeTicks::Now();

    base::ProcessHandle handle = 0;
    base::LaunchProcess(*cmd_line, base::LaunchOptions(), &handle);
    if (handle){
      BrowserThread::PostTask(
        client_thread_id, FROM_HERE,
        base::Bind(
        &Context::Notify,
        this_object,
        handle));
    }
  }

  void Notify(base::ProcessHandle handle) {
    starting_ = false;
    process_.set_handle(handle);
    if (!handle)
      LOG(ERROR) << "Failed to launch child process";
    if (client_) {
      client_->OnProcessLaunched();
    } else {
      Terminate();
    }
  }

  void Terminate() {
    if (!process_.handle())
      return;

    if (!terminate_child_on_shutdown_)
      return;

    // On Posix, EnsureProcessTerminated can lead to 2 seconds of sleep!  So
    // don't this on the UI/IO threads.
    BrowserThread::PostTask(
        BrowserThread::PROCESS_LAUNCHER, FROM_HERE,
        base::Bind(
            &Context::TerminateInternal, process_.handle()));
    process_.set_handle(base::kNullProcessHandle);
  }

  static void SetProcessBackgrounded(base::ProcessHandle handle,
                                     bool background) {
    base::Process process(handle);
    process.SetProcessBackgrounded(background);
  }

  static void TerminateInternal(base::ProcessHandle handle) {
    base::Process process(handle);
    process.Terminate(0);
    process.Close();
  }

  ChildProcessLauncher::Client* client_;
  BrowserThread::ID client_thread_id_;
  base::Process process_;
  base::TerminationStatus termination_status_;
  int exit_code_;
  bool starting_;
  // Controls whether the child process should be terminated on browser
  // shutdown. Default behavior is to terminate the child.
  bool terminate_child_on_shutdown_;
};


ChildProcessLauncher::ChildProcessLauncher(
    CommandLine* cmd_line,
    int child_process_id,
    Client* client) {
  context_ = new Context();
  context_->Launch(
      cmd_line,
      child_process_id,
      client);
}

ChildProcessLauncher::~ChildProcessLauncher() {
  context_->ResetClient();
}

bool ChildProcessLauncher::IsStarting() {
  return context_->starting_;
}

base::ProcessHandle ChildProcessLauncher::GetHandle() {
  DCHECK(!context_->starting_);
  return context_->process_.handle();
}

base::TerminationStatus ChildProcessLauncher::GetChildTerminationStatus(
    bool known_dead,
    int* exit_code) {
  base::ProcessHandle handle = context_->process_.handle();
  if (handle == base::kNullProcessHandle) {
    // Process is already gone, so return the cached termination status.
    if (exit_code)
      *exit_code = context_->exit_code_;
    return context_->termination_status_;
  }
  {
    context_->termination_status_ =
        base::GetTerminationStatus(handle, &context_->exit_code_);
  }

  if (exit_code)
    *exit_code = context_->exit_code_;

  // POSIX: If the process crashed, then the kernel closed the socket
  // for it and so the child has already died by the time we get
  // here. Since GetTerminationStatus called waitpid with WNOHANG,
  // it'll reap the process.  However, if GetTerminationStatus didn't
  // reap the child (because it was still running), we'll need to
  // Terminate via ProcessWatcher. So we can't close the handle here.
  if (context_->termination_status_ != base::TERMINATION_STATUS_STILL_RUNNING)
    context_->process_.Close();

  return context_->termination_status_;
}

void ChildProcessLauncher::SetProcessBackgrounded(bool background) {
  BrowserThread::PostTask(
      BrowserThread::PROCESS_LAUNCHER, FROM_HERE,
      base::Bind(
          &ChildProcessLauncher::Context::SetProcessBackgrounded,
          GetHandle(), background));
}

void ChildProcessLauncher::SetTerminateChildOnShutdown(
  bool terminate_on_shutdown) {
  if (context_)
    context_->set_terminate_child_on_shutdown(terminate_on_shutdown);
}
