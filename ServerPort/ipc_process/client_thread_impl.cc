// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "client_thread_impl.h"
#include <algorithm>
#include <limits>
#include <map>
#include <vector>

#include "base/allocator/allocator_extension.h"
#include "base/command_line.h"
#include "base/debug/trace_event.h"
#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/metrics/field_trial.h"
#include "base/metrics/histogram.h"
#include "base/metrics/stats_table.h"
#include "base/path_service.h"
#include "base/shared_memory.h"
#include "base/string16.h"
#include "base/string_number_conversions.h"  // Temporary
#include "base/threading/thread_local.h"
#include "base/utf_string_conversions.h"
#include "base/values.h"

#include "ipc/ipc_channel_handle.h"
#include "ipc/ipc_forwarding_message_filter.h"
#include "ipc/ipc_platform_file.h"

#if defined(OS_WIN)
#include <windows.h>
#include <objbase.h>
#include "base/win/scoped_com_initializer.h"
#else
// TODO(port)
#include "base/memory/scoped_handle.h"
#include "content/common/np_channel_base.h"
#endif


namespace {

// Keep the global TridentThreadImpl in a TLS slot so it is impossible to access
// incorrectly from the wrong thread.
base::LazyInstance<base::ThreadLocalPointer<ClientThreadImpl> >
    lazy_tls = LAZY_INSTANCE_INITIALIZER;
}// namespace


ClientThreadImpl* ClientThreadImpl::current() {
  return lazy_tls.Pointer()->Get();
}

// When we run plugins in process, we actually run them on the render thread,
// which means that we need to make the render thread pump UI events.
ClientThreadImpl::ClientThreadImpl() {
  Init();
}

ClientThreadImpl::ClientThreadImpl(const std::string& channel_name) : ChildThread(channel_name) {
  Init();
}

void ClientThreadImpl::Init() {
  lazy_tls.Pointer()->Set(this);
  // Register this object as the main thread.
  ChildProcess::current()->set_main_thread(this);
}

ClientThreadImpl::~ClientThreadImpl() {
  lazy_tls.Pointer()->Set(NULL);
}
