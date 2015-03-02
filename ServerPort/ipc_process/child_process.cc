// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "child_process.h"
#include "base/message_loop.h"
#include "base/metrics/statistics_recorder.h"
#include "base/process_util.h"
#include "base/string_number_conversions.h"
#include "base/threading/thread.h"
#include "base/utf_string_conversions.h"
#include "child_thread.h"


ChildProcess* ChildProcess::child_process_;


ChildProcess::ChildProcess()
    : ref_count_(0),
      shutdown_event_(true, false),
      io_thread_("Chrome_ChildIOThread") {
  DCHECK(!child_process_);
  child_process_ = this;

  base::StatisticsRecorder::Initialize();

  // We can't recover from failing to start the IO thread.
  CHECK(io_thread_.StartWithOptions(
            base::Thread::Options(MessageLoop::TYPE_IO, 0)));

#if defined(OS_ANDROID)
  // TODO(epenner): Move thread priorities to base. (crbug.com/170549)
  io_thread_.message_loop()->PostTask(FROM_HERE,
                                      base::Bind(&SetHighThreadPriority));
#endif
}

ChildProcess::~ChildProcess() {
  DCHECK(child_process_ == this);

  // Signal this event before destroying the child process.  That way all
  // background threads can cleanup.
  // For example, in the renderer the RenderThread instances will be able to
  // notice shutdown before the render process begins waiting for them to exit.
  shutdown_event_.Signal();

  // Kill the main thread object before nulling child_process_, since
  // destruction code might depend on it.
  main_thread_.reset();

  child_process_ = NULL;
}

ChildThread* ChildProcess::main_thread() {
  return main_thread_.get();
}

void ChildProcess::set_main_thread(ChildThread* thread) {
  main_thread_.reset(thread);
}

void ChildProcess::AddRefProcess() {
  DCHECK(!main_thread_.get() ||  // null in unittests.
         MessageLoop::current() == main_thread_->message_loop());
  ref_count_++;
}

void ChildProcess::ReleaseProcess() {
  DCHECK(!main_thread_.get() ||  // null in unittests.
         MessageLoop::current() == main_thread_->message_loop());
  DCHECK(ref_count_);
  DCHECK(child_process_);
  if (--ref_count_)
    return;

  if (main_thread_.get())  // null in unittests.
    main_thread_->OnProcessFinalRelease();
}

base::WaitableEvent* ChildProcess::GetShutDownEvent() {
  DCHECK(child_process_);
  return &child_process_->shutdown_event_;
}
