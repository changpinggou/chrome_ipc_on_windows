// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PROCESS_HOST_H_
#define PROCESS_HOST_H_

#include "base/basictypes.h"
#include "base/id_map.h"
#include "base/process.h"
#include "base/process_util.h"
#include "ipc/ipc_channel_proxy.h"
#include "ipc/ipc_sender.h"


class GURL;
struct ViewMsg_SwapOut_Params;

namespace content {
class BrowserContext;
class RenderWidgetHost;
class StoragePartition;
}

namespace base {
class TimeDelta;
}

namespace gfx {
  class Size;
}

class ProcessHost : public IPC::Sender,
                          public IPC::Listener {
 public:
  //typedef IDMap<RenderProcessHost>::iterator iterator;
  //typedef IDMap<RenderWidgetHost>::const_iterator RenderWidgetHostsIterator;

  // Details for RENDERER_PROCESS_CLOSED notifications.
  struct RendererClosedDetails {
    RendererClosedDetails(base::ProcessHandle handle,
                          base::TerminationStatus status,
                          int exit_code) {
      this->handle = handle;
      this->status = status;
      this->exit_code = exit_code;
    }
    base::ProcessHandle handle;
    base::TerminationStatus status;
    int exit_code;
  };

  virtual ~ProcessHost() {}

  // Initialize the new renderer process, returning true on success. This must
  // be called once before the object can be used, but can be called after
  // that with no effect. Therefore, if the caller isn't sure about whether
  // the process has been created, it should just call Init().
  virtual bool Init() = 0;

  // Gets the next available routing id.
  virtual int GetNextRoutingID() = 0;

  // Called to wait for the next UpdateRect message for the specified render
  // widget.  Returns true if successful, and the msg out-param will contain a
  // copy of the received UpdateRect message.
  virtual bool WaitForBackingStoreMsg(int render_widget_id,
                                      const base::TimeDelta& max_delay,
                                      IPC::Message* msg) = 0;

  // Called when a received message cannot be decoded.
  virtual void ReceivedBadMessage() = 0;

  // Returns the storage partition associated with this process.
  //
  // TODO(nasko): Remove this function from the public API once
  // URLRequestContextGetter's creation is moved into StoragePartition.
  // http://crbug.com/158595
  //virtual StoragePartition* GetStoragePartition() const = 0;

  // Try to shutdown the associated renderer process as fast as possible.
  // If this renderer has any RenderViews with unload handlers, then this
  // function does nothing.  The current implementation uses TerminateProcess.
  // Returns True if it was able to do fast shutdown.
  virtual bool FastShutdownIfPossible() = 0;

  // Returns true if fast shutdown was started for the renderer.
  virtual bool FastShutdownStarted() const = 0;

  // Dump the child process' handle table before shutting down.
  virtual void DumpHandles() = 0;

  // Returns the process object associated with the child process.  In certain
  // tests or single-process mode, this will actually represent the current
  // process.
  //
  // NOTE: this is not necessarily valid immediately after calling Init, as
  // Init starts the process asynchronously.  It's guaranteed to be valid after
  // the first IPC arrives.
  virtual base::ProcessHandle GetHandle() const = 0;

  // Transport DIB functions ---------------------------------------------------

  // Return the TransportDIB for the given id. On Linux, this can involve
  // mapping shared memory. On Mac, the shared memory is created in the browser
  // process and the cached metadata is returned. On Windows, this involves
  // duplicating the handle from the remote process.  The RenderProcessHost
  // still owns the returned DIB.
  //virtual TransportDIB* GetTransportDIB(TransportDIB::Id dib_id) = 0;

  // Returns whether this process is using the same StoragePartition as
  // |partition|.
  //virtual bool InSameStoragePartition(StoragePartition* partition) const = 0;

  // Returns the unique ID for this child process. This can be used later in
  // a call to FromID() to get back to this object (this is used to avoid
  // sending non-threadsafe pointers to other threads).
  //
  // This ID will be unique for all child processes, including workers, plugins,
  // etc.
  virtual int GetID() const = 0;

  // Returns true iff channel_ has been set to non-NULL. Use this for checking
  // if there is connection or not. Virtual for mocking out for tests.
  virtual bool HasConnection() const = 0;

  // Call this to allow queueing of IPC messages that are sent before the
  // process is launched.
  virtual void EnableSendQueue() = 0;

  // Returns the renderer channel.
  virtual IPC::ChannelProxy* GetChannel() = 0;

  // Schedules the host for deletion and removes it from the all_hosts list.
  virtual void Cleanup() = 0;

  // Static management functions -----------------------------------------------

  // Flag to run the renderer in process.  This is primarily
  // for debugging purposes.  When running "in process", the
  // browser maintains a single RenderProcessHost which communicates
  // to a RenderProcess which is instantiated in the same process
  // with the Browser.  All IPC between the Browser and the
  // Renderer is the same, it's just not crossing a process boundary.

  static bool run_renderer_in_process();

  // This also calls out to ContentBrowserClient::GetApplicationLocale and
  // modifies the current process' command line.
  static void SetRunRendererInProcess(bool value);

//   // Allows iteration over all the RenderProcessHosts in the browser. Note
//   // that each host may not be active, and therefore may have NULL channels.
//   static iterator AllHostsIterator();


  // Returns true if the caller should attempt to use an existing
  // RenderProcessHost rather than creating a new one.
  static bool ShouldTryToUseExistingProcessHost(
      content::BrowserContext* browser_context, const GURL& site_url);



  // Overrides the default heuristic for limiting the max renderer process
  // count.  This is useful for unit testing process limit behaviors.  It is
  // also used to allow a command line parameter to configure the max number of
  // renderer processes and should only be called once during startup.
  // A value of zero means to use the default heuristic.
  static void SetMaxRendererProcessCount(size_t count);

  // Returns the current max number of renderer processes used by the content
  // module.
  static size_t GetMaxRendererProcessCount();
};

#endif  // PROCESS_HOST_H_
