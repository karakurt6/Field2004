// Field.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include <lm_attr.h>
#include <mapi.h>
#include <Inventor\Win\SoWin.h>
#include <Inventor\SoDB.h>
#include <Inventor\Nodekits\SoNodekit.h>
#include <Inventor\SoInteraction.h>
#include <DataViz\PoDataViz.h>
#include <DialogViz\dialog\SoDialogViz.h>
#include "Field.h"
#include "MainFrm.h"
#include "FieldDoc.h"
#include "FieldView.h"
#include "OverView.h"
#include "Redirect.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// #define USE_RESOLVE_CALL
#define MAPI_USERNAME "chupeev alexander"
#define MAPI_ADDRESS "chupeev@tmn.ru"

/////////////////////////////////////////////////////////////////////////////
// CFieldApp

BEGIN_MESSAGE_MAP(CFieldApp, CWinApp)
	//{{AFX_MSG_MAP(CFieldApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFieldApp construction

CFieldApp::CFieldApp(): m_pDebugRedirect(0)
{
	m_jobLicense = (LM_HANDLE*) NULL;
	m_bDemoMode = false;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CFieldApp object

CFieldApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CFieldApp initialization

BOOL CFieldApp::InitInstance()
{
  if (!AfxOleInit())
  {
		AfxMessageBox(_T("Failed to initialize OLE libraries"));
		return FALSE;
  }

  AfxGetModuleState()->m_dwVersion = 0x0601;
	AfxEnableControlContainer();
	AfxInitRichEdit();

  m_pDebugRedirect = new CDebugRedirect;
  if (!m_pDebugRedirect->Redirect(STD_ERROR_HANDLE, stderr))
  {
		AfxMessageBox(_T("Failed to start redirect stderr\n"));
    delete m_pDebugRedirect;
    m_pDebugRedirect = 0;
		return FALSE;
  }

	SoDB::init();
	SoNodeKit::init();
	SoInteraction::init();
	PoDataViz::init();
  SoDialogViz::init();

  GXInit();
	GXSetNewGridLineMode();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

	/*
#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif
	*/

	// Change the registry key under which our settings are stored.
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Field"));

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	VENDORCODE code;
	int status = lc_new_job(0, lc_new_job_arg2, &code, &m_jobLicense);
	if (status != 0)
	{
#ifdef _DEBUG
		lc_perror(m_jobLicense, "lc_new_job() failed");
#endif
		lc_free_job(m_jobLicense);
		m_jobLicense = (LM_HANDLE*) NULL;
		return FALSE;
	}
	{
		TCHAR path[0x1000];
		GetModuleFileName(NULL, path, _MAX_PATH);
		PathRemoveFileSpec(path);
		strcat(path, ";START_LICENSE\nfield-application-24193-38891-01025-32928-"
			"08017-12313-00060-41408-32052-52813-62414-10729-23821-02236-49174-"
			"17533-20468-53622-23917-43762-48491-13486-9\nEND_LICENSE");
		lc_set_attr(m_jobLicense, LM_A_LICENSE_DEFAULT, (LM_A_VAL_TYPE) path);
		lc_set_attr(m_jobLicense, LM_A_LCM, (LM_A_VAL_TYPE) 0);
	}
	LM_CHAR_PTR feature = "application";
	status = lc_checkout(m_jobLicense, feature, \
		"2004.0", 1, LM_CO_NOWAIT, &code, LM_DUP_NONE);
	if (status != 0)
	{
		if (AfxMessageBox(TEXT("There is no valid licenses to start Field application\n")
			TEXT("Would you like to contact with developer\n")
			TEXT("to order license for you system?"), MB_YESNO) == IDYES)
		{
			char buff[MAX_CONFIG_LINE];
			status = lc_hostid(m_jobLicense, HOSTID_DEFAULT, buff);
			if (status == 0)
			{
				HMODULE hModule = LoadLibrary(TEXT("MAPI32"));
				if (hModule)
				{
					LPMAPIRESOLVENAME lpMAPIResolveName = (LPMAPIRESOLVENAME) \
						GetProcAddress(hModule, TEXT("MAPIResolveName"));
					if (lpMAPIResolveName)
					{
#ifdef USE_RESOLVE_CALL
						MapiRecipDesc *recip;
						status = lpMAPIResolveName(0, 0, \
							MAPI_ADDRESS, 0, 0, &recip);
#else
						MapiRecipDesc desc, *recip = &desc;
						ZeroMemory(&desc, sizeof(desc));
						desc.ulRecipClass = MAPI_TO;
						desc.lpszName = MAPI_ADDRESS;
#endif
						if (status == SUCCESS_SUCCESS)
						{
							LPMAPISENDMAIL lpMAPISendMail = (LPMAPISENDMAIL) \
								GetProcAddress(hModule, TEXT("MAPISendMail"));
							if (lpMAPISendMail)
							{
								MapiMessage note;
								ZeroMemory(&note, sizeof(note));
								note.lpszSubject = "license order";
								note.lpszNoteText = buff;
								note.nRecipCount = 1;
								note.lpRecips = recip;
								status = lpMAPISendMail(0, 0, &note, MAPI_LOGON_UI | MAPI_DIALOG, 0);
								if (status != SUCCESS_SUCCESS)
								{
									char text[0x100];
									strcat(strcat(strcpy(text, "Send your hardware profile string \n\t"), buff),
										"\nto " MAPI_USERNAME "(" MAPI_ADDRESS ")");
									AfxMessageBox(text);
								}
							}
#ifdef USE_RESOLVE_CALL
							LPMAPIFREEBUFFER lpMAPIFreeBuffer = (LPMAPIFREEBUFFER) \
								GetProcAddress(hModule, TEXT("MAPIFreeBuffer"));
							if (lpMAPIFreeBuffer)
							{
								lpMAPIFreeBuffer(recip);
							}
#endif
						}
					}
					FreeLibrary(hModule);
				}
			}
		}
#ifdef _DEBUG
		lc_perror(m_jobLicense, "lc_checkout() failed for application");
#endif
		return FALSE;
	}

	CONFIG* conf = lc_auth_data(m_jobLicense, feature);
	m_bDemoMode = (conf->idptr && conf->idptr->type == HOSTID_DEMO);

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CActField),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CFieldView));
	AddDocTemplate(pDocTemplate);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it.
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

  // fprintf(stderr, "this is a test\n");

	return TRUE;
}

int CFieldApp::ExitInstance() 
{
	if (m_jobLicense)
	{
		lc_checkin(m_jobLicense, LM_CI_ALL_FEATURES, 0);
		lc_free_job(m_jobLicense);
		m_jobLicense = (LM_HANDLE*) NULL;
	}

  if (m_pDebugRedirect)
  {
    m_pDebugRedirect->StopRedirect();
    delete m_pDebugRedirect;
    m_pDebugRedirect = 0;
  }
	
	return CWinApp::ExitInstance();
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CFieldApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CFieldApp message handlers


BOOL CFieldApp::OnIdle(LONG lCount)
{
	SoWin::doIdleTasks();
	return CWinApp::OnIdle(lCount);
}
