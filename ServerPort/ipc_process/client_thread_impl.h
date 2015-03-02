// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_TRIDENT_THREAD_IMPL_H_
#define CONTENT_TRIDENT_THREAD_IMPL_H_

#include <set>
#include <string>
#include <vector>
#include <map>

#include "base/observer_list.h"
#include "base/string16.h"
#include "base/timer.h"
#include "build/build_config.h"
#include "child_process.h"
#include "child_thread.h"

#include "ipc/ipc_channel_proxy.h"





namespace base {
class MessageLoopProxy;
class Thread;
}

#if defined(OS_WIN)
namespace win {
class ScopedCOMInitializer;
}
#endif



namespace IPC {
class ForwardingMessageFilter;
}

// The RenderThreadImpl class represents a background thread where RenderView
// instances live.  The RenderThread supports an API that is used by its
// consumer to talk indirectly to the RenderViews and supporting objects.
// Likewise, it provides an API for the RenderViews to talk back to the main
// process (i.e., their corresponding WebContentsImpl).
//
// Most of the communication occurs in the form of IPC messages.  They are
// routed to the RenderThread according to the routing IDs of the messages.
// The routing IDs correspond to RenderView instances.
class  ClientThreadImpl : public ChildThread {
 public:
  static ClientThreadImpl* current();

  ClientThreadImpl();
  // Constructor that's used when running in single process mode.
  explicit ClientThreadImpl(const std::string& channel_name);
  virtual ~ClientThreadImpl();
  void Init();
  DISALLOW_COPY_AND_ASSIGN(ClientThreadImpl);
};

#endif  // CONTENT_TRIDENT_THREAD_IMPL_H_
