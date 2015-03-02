// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PROCESS_HOST_IMPL_H_
#define PROCESS_HOST_IMPL_H_

#include <map>
#include <queue>
#include <string>

#include "base/memory/scoped_ptr.h"
#include "base/process.h"
#include "base/atomic_sequence_num.h"
#include "base/synchronization/waitable_event.h"
#include "base/timer.h"
#include "ipc/ipc_channel_proxy.h"
#include "ipc_process/process_host.h"

class ProcessHostImpl : public ProcessHost
                        //public ChildProcessLauncher::Client
{
public:
  ProcessHostImpl();
  virtual ~ProcessHostImpl();
public:
  // RenderProcessHost implementation (public portion).
  virtual void EnableSendQueue() OVERRIDE;
  virtual bool Init() OVERRIDE;
  virtual int GetNextRoutingID() OVERRIDE;
  virtual bool WaitForBackingStoreMsg(int render_widget_id,
    const base::TimeDelta& max_delay,
    IPC::Message* msg) OVERRIDE;
  virtual void ReceivedBadMessage() OVERRIDE;
  virtual bool FastShutdownIfPossible() OVERRIDE;
  virtual void DumpHandles() OVERRIDE;
  virtual base::ProcessHandle GetHandle() const OVERRIDE;

  virtual int GetID() const OVERRIDE;
  virtual bool HasConnection() const OVERRIDE;

  virtual void Cleanup() OVERRIDE;
  virtual IPC::ChannelProxy* GetChannel() OVERRIDE;
  virtual bool FastShutdownStarted() const OVERRIDE;

  // IPC::Sender via RenderProcessHost.
  virtual bool Send(IPC::Message* msg) OVERRIDE;

  // IPC::Listener via RenderProcessHost.
  virtual bool OnMessageReceived(const IPC::Message& msg) OVERRIDE;
  virtual void OnChannelConnected(int32 peer_pid) OVERRIDE;
  virtual void OnChannelError() OVERRIDE;

  // Handle termination of our process.
  void ProcessDied(bool already_dead);
  void OnDumpHandlesDone();
protected:
  // A proxy for our IPC::Channel that lives on the IO thread (see
  // browser_process.h)
  scoped_ptr<IPC::ChannelProxy> channel_;
  bool is_initialized_;

  // The globally-unique identifier for this RPH.
  int id_;
  // The next routing id to use.
  base::AtomicSequenceNumber next_routing_id_;
};

#endif  // CONTENT_BROWSER_RENDERER_HOST_BROWSER_RENDER_PROCESS_HOST_IMPL_H_
