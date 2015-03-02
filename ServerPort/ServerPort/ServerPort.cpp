
// ServerPort.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "ServerPort.h"
#include "ServerPortDlg.h"
#include "base/at_exit.h"
#include "ipc_process/server_main_thread_impl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CServerPortApp

BEGIN_MESSAGE_MAP(CServerPortApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CServerPortApp construction

CServerPortApp::CServerPortApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CServerPortApp object

CServerPortApp theApp;


// CServerPortApp initialization

BOOL CServerPortApp::InitInstance()
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
	CServerPortDlg dlg;
	m_pMainWnd = &dlg;
  //dlg.DoModal();

  if (!MessageLoop::current()){
    new MessageLoop(MessageLoop::TYPE_UI);
  }
  
  dlg.Create(IDD_SERVERPORT_DIALOG, NULL);
  dlg.ShowWindow(SW_SHOW);
  MessageLoop::current()->Run();
	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

