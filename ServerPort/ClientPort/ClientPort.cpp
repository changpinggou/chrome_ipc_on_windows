
// ClientPort.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "ClientPort.h"
#include "ClientPortDlg.h"
#include "base/at_exit.h"
#include "base/command_line.h"
#include "base/message_loop.h"
#include "ipc_process/child_process.h"
#include "ipc_process/client_thread_impl.h"
#include "ipc_process/test_messages.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CClientPortApp

BEGIN_MESSAGE_MAP(CClientPortApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CClientPortApp construction

CClientPortApp::CClientPortApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CClientPortApp object

CClientPortApp theApp;


// CClientPortApp initialization

BOOL CClientPortApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	AfxEnableControlContainer();

	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager *pShellManager = new CShellManager;

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));
  base::AtExitManager exit_manager;
  CommandLine::Init(0, NULL);
  MessageLoop main_message_loop(MessageLoop::TYPE_UI);
  base::PlatformThread::SetName("IpcClientMain");

	CClientPortDlg dlg;
	m_pMainWnd = &dlg;
  
  dlg.Create(IDD_CLIENTPORT_DIALOG, NULL);
  dlg.ShowWindow(SW_SHOW);
  ::MessageBox(NULL, L"InitInstance", L"InitInstance", MB_OK);
  new ChildProcess();
  new ClientThreadImpl();


  MessageLoop::current()->Run();

	// Delete the shell manager created above.
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

