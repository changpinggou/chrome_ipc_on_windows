
// ServerPortDlg.h : header file
//

#pragma once

#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/message_loop_proxy.h"
#include "base/thread_task_runner_handle.h"
#include "ipc_process/browser_process_sub_thread.h"
#include "ipc/ipc_channel_proxy.h"
#include "ipc/ipc_sender.h"
#include "ipc_process/child_process_launcher.h"

class ProcessHostImpl;

class CServerPortDlg : public CDialogEx,
                       public IPC::Sender,
                       public IPC::Listener,
                       public ChildProcessLauncher::Client

{
// Construction
public:
	CServerPortDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_SERVERPORT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
  afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
  bool CreateIOThread();
  bool CreateProcessLaunchThread();
  void OnMyTestIpc(const string16& content);
  void ExecuteTask();
  bool LaunchClientProc();

  // IPC::Sender via IPC::Sender
  virtual bool Send(IPC::Message* msg) OVERRIDE;

  // IPC::Listener via IPC::Listener.
  virtual bool OnMessageReceived(const IPC::Message& msg) OVERRIDE;
  virtual void OnChannelConnected(int32 peer_pid) OVERRIDE;
  virtual void OnChannelError() OVERRIDE;

  void ProcessDied(bool already_dead);
  int GetID() const;

  // ChildProcessLauncher::Client implementation.
  virtual void OnProcessLaunched() OVERRIDE;
private:
  scoped_ptr<MessageLoop> main_message_loop_;
  scoped_ptr<BrowserProcessSubThread> io_thread_;
  scoped_ptr<BrowserProcessSubThread> process_launch_thread_;

  ProcessHostImpl* proc_host_impl_;
  // The message loop proxy associated with this message loop, if one exists.
  scoped_refptr<base::MessageLoopProxy> message_loop_proxy_;
  scoped_ptr<base::ThreadTaskRunnerHandle> thread_task_runner_handle_;

  // A proxy for our IPC::Channel that lives on the IO thread (see
  // browser_process.h)
  scoped_ptr<IPC::ChannelProxy> channel_;

  // Used to launch and terminate the process without blocking the UI thread.
  scoped_ptr<ChildProcessLauncher> child_process_launcher_;

  // Messages we queue while waiting for the process handle.  We queue them here
  // instead of in the channel so that we ensure they're sent after init related
  // messages that are sent once the process handle is available.  This is
  // because the queued messages may have dependencies on the init messages.
  std::queue<IPC::Message*> queued_messages_;

  bool is_initialized_;
  bool deleting_soon_;
  bool fast_shutdown_started_;
  // The globally-unique identifier for this RPH.
  int id_;
public:
  afx_msg void OnBnClickedButton1();
};
