
// ServerPortDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ServerPort.h"
#include "ServerPortDlg.h"
#include "afxdialogex.h"

#include "ipc_process/process_host_impl.h"
#include "base/message_loop_proxy_impl.h"
#include "base/threading/thread_restrictions.h"
#include "base/command_line.h"
#include "ipc/ipc_switches.h"
#include "ipc/ipc_message_macros.h"
#include "ipc_process/test_messages.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CServerPortDlg dialog




CServerPortDlg::CServerPortDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CServerPortDlg::IDD, pParent)
{
  proc_host_impl_ = new ProcessHostImpl();
  is_initialized_ = false;
  deleting_soon_ = false;
  fast_shutdown_started_ = false;
  id_ = 0;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CServerPortDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CServerPortDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
  ON_WM_SHOWWINDOW()
/*  ON_BN_CLICKED(IDC_BUTTON1, &CServerPortDlg::OnBnClickedButton1)*/
ON_BN_CLICKED(IDC_BUTTON1, &CServerPortDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// CServerPortDlg message handlers

BOOL CServerPortDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
//   message_loop_proxy_ = new base::MessageLoopProxyImpl();
//   thread_task_runner_handle_.reset(
//     new base::ThreadTaskRunnerHandle(message_loop_proxy_));
  // Create a MessageLoop if one does not already exist for the current thread.


  //proc_host_impl_->Init();
  CreateIOThread();
  CreateProcessLaunchThread();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CServerPortDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CServerPortDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CServerPortDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CServerPortDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
  CDialogEx::OnShowWindow(bShow, nStatus);
  // TODO: Add your message handler code here
}

bool CServerPortDlg::CreateProcessLaunchThread(){
  base::Thread::Options io_message_loop_options;
  io_message_loop_options.message_loop_type = MessageLoop::TYPE_DEFAULT;
  base::Thread::Options* options = &io_message_loop_options;
  scoped_ptr<BrowserProcessSubThread>* thread_to_start = &process_launch_thread_;
  (*thread_to_start).reset(new BrowserProcessSubThread(BrowserThread::PROCESS_LAUNCHER));
  bool result = (*thread_to_start)->StartWithOptions(*options);
  return result;
}

bool CServerPortDlg::CreateIOThread(){
  base::Thread::Options io_message_loop_options;
  io_message_loop_options.message_loop_type = MessageLoop::TYPE_IO;
  base::Thread::Options* options = &io_message_loop_options;
  scoped_ptr<BrowserProcessSubThread>* thread_to_start = &io_thread_;
  (*thread_to_start).reset(new BrowserProcessSubThread(BrowserThread::IO));
  bool result = (*thread_to_start)->StartWithOptions(*options);
  return result;
}

bool CServerPortDlg::LaunchClientProc()
{
  if (channel_.get()){
    return true;
  }

  const std::string channel_id =
      IPC::Channel::GenerateVerifiedChannelID(std::string());
  base::FilePath client_exe_path(L"C:\\Users\\applechang\\Desktop\\ipc_win_pipe\\ServerPort\\Debug\\ClientPort.exe");
  CommandLine* cmd_line = new CommandLine(client_exe_path);
  cmd_line->AppendSwitchASCII(switches::kProcessChannelID, channel_id);
  channel_.reset(new IPC::ChannelProxy(
    channel_id, IPC::Channel::MODE_SERVER, this,
    BrowserThread::GetMessageLoopProxyForThread(BrowserThread::IO)));

  child_process_launcher_.reset(new ChildProcessLauncher(cmd_line, GetID(), this));

  fast_shutdown_started_ = false;
  is_initialized_ = true;
  return true;
}

bool CServerPortDlg::Send(IPC::Message* msg) {
  if (!channel_.get()) {
    if (!is_initialized_) {
      queued_messages_.push(msg);
      return true;
    } else {
      delete msg;
      return false;
    }
  }

//   if (child_process_launcher_.get() && child_process_launcher_->IsStarting()) {
//     queued_messages_.push(msg);
//     return true;
//   }

  return channel_->Send(msg);
}

bool CServerPortDlg::OnMessageReceived(const IPC::Message& msg) {
  // If we're about to be deleted, or have initiated the fast shutdown sequence,
  // we ignore incoming messages.

  if (deleting_soon_ || fast_shutdown_started_)
    return false;
  int32 routint_id = msg.routing_id();
  uint32 type  = msg.type();

  //mark_child_process_activity_time();
//  if (msg.routing_id() == MSG_ROUTING_CONTROL) {
    // Dispatch control messages.
  bool msg_is_ok = true;
  IPC_BEGIN_MESSAGE_MAP_EX(CServerPortDlg, msg, msg_is_ok)
    IPC_MESSAGE_HANDLER(ViewMsg_MyTestIpc, OnMyTestIpc)
    IPC_MESSAGE_UNHANDLED_ERROR()
    IPC_END_MESSAGE_MAP_EX()

 if (!msg_is_ok) {
        // The message had a handler, but its de-serialization failed.
        // We consider this a capital crime. Kill the renderer if we have one.
        LOG(ERROR) << "bad message " << msg.type() << " terminating renderer.";
        //RecordAction(UserMetricsAction("BadMessageTerminate_BRPH"));
        //ReceivedBadMessage();
      return true;
  }

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
//     IPC_BEGIN_MESSAGE_MAP_EX(RenderProcessHostImpl, msg, msg_is_ok)
//       IPC_MESSAGE_HANDLER(ViewHostMsg_CompositorSurfaceBuffersSwapped,
//       OnCompositorSurfaceBuffersSwappedNoHost)
//       IPC_END_MESSAGE_MAP_EX()
//       return true;
//   }
//   return RenderWidgetHostImpl::From(rwh)->OnMessageReceived(msg);
  return true;
}

void CServerPortDlg::OnMyTestIpc(const string16& content){
  ::MessageBox(NULL, content.c_str(), content.c_str(), MB_OK);
}

void CServerPortDlg::OnChannelConnected(int32 peer_pid) {
  ::MessageBox(NULL, L"OnChannelConnected", L"OnChannelConnected", MB_OK);
// #if defined(IPC_MESSAGE_LOG_ENABLED)
//   Send(new ChildProcessMsg_SetIPCLoggingEnabled(
//     IPC::Logging::GetInstance()->Enabled()));
// #endif
// 
//   tracked_objects::ThreadData::Status status =
//     tracked_objects::ThreadData::status();
//   Send(new ChildProcessMsg_SetProfilerStatus(status));
}

void CServerPortDlg::OnChannelError() {
  ProcessDied(true /* already_dead */);
}

void CServerPortDlg::ProcessDied(bool already_dead){
}

void CServerPortDlg::OnProcessLaunched() {
  // No point doing anything, since this object will be destructed soon.  We
  // especially don't want to send the RENDERER_PROCESS_CREATED notification,
  // since some clients might expect a RENDERER_PROCESS_TERMINATED afterwards to
  // properly cleanup.
  if (deleting_soon_)
    return;

  if (child_process_launcher_.get()) {
    if (!child_process_launcher_->GetHandle()) {
      OnChannelError();
      return;
    }

    //child_process_launcher_->SetProcessBackgrounded(backgrounded_);
  }

  // NOTE: This needs to be before sending queued messages because
  // ExtensionService uses this notification to initialize the renderer process
  // with state that must be there before any JavaScript executes.
  //
  // The queued messages contain such things as "navigate". If this notification
  // was after, we can end up executing JavaScript before the initialization
  // happens.
//   NotificationService::current()->Notify(
//     NOTIFICATION_RENDERER_PROCESS_CREATED,
//     Source<RenderProcessHost>(this),
//     NotificationService::NoDetails());

  while (!queued_messages_.empty()) {
    Send(queued_messages_.front());
    queued_messages_.pop();
  }
}

int CServerPortDlg::GetID() const {
  return id_;
}

void CServerPortDlg::OnBnClickedButton1(){
  LaunchClientProc();
}
