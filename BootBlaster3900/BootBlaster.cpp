// BootBlaster.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "BootBlaster.h"
#include <commctrl.h>
#include <aygshell.h>
#include <sipapi.h>
#include <windows.h>
#include <winuser.h>


TCHAR *idle_strings[] = {
  _T("\n\n"),
  _T("Welcome to BootBlaster3900-2.6 \n\n"),
  _T("Please follow these steps:     \n"),
  _T("1) Flash->Save Bootldr .gz     \n"),
  _T("     (backup the bootloader).  \n"),
  _T("2) Flash->Save Wince .gz       \n"),
  _T("     (backup Wince and Assets) \n"),
  _T("3) Flash->Program              \n"),
  _T("     (install new bootloader)  \n"),
  _T("4) Flash->Verify               \n"),
  _T("     (verify the bootloader)   \n"),
  _T("5) File->exit                  \n"),
  _T("     (exit BootBlaster)        \n\n"),
  NULL
};

#define MAX_LOADSTRING 1023

// Global Variables:
HINSTANCE			hInst;					// The current instance
HWND				hwndCB;					// The command bar handle
TCHAR szStatus[MAX_LOADSTRING];				// current operation Status
UINT gTimerID;
BOOL	gSystemErrorHasOccured = FALSE;					// prevent destructive actions in the presence of an error.
HANDLE gLogFileHandle;
enum timerOperations{
     BB_PROGRAM_FLASH=1,
     BB_ERASE_FLASH=2,
     BB_LOAD_IMAGE=3,
     BB_TEST_FLASH=4,
     BB_PROTECT_WINCE=5};



static SHACTIVATEINFO s_sai;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass	(HINSTANCE, LPTSTR);
BOOL				InitInstance	(HINSTANCE, LPTSTR, int);
LRESULT CALLBACK	WndProc			(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About			(HWND, UINT, WPARAM, LPARAM);
HWND				CreateRpCommandBar(HWND);


int WINAPI WinMain(	HINSTANCE hInstance,
			HINSTANCE hPrevInstance,
			LPTSTR    lpCmdLine,
			int       nCmdShow)
{
     MSG msg;
     HACCEL hAccelTable;

     gSystemErrorHasOccured = FALSE;

     // Perform application initialization:
     if (!InitInstance (hInstance, lpCmdLine, nCmdShow)) 
     {
	  return FALSE;
     }

     InitCommonControls();
     hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_BOOTBLASTER);

     init();
     // Main message loop:
     while (GetMessage(&msg, NULL, 0, 0)) 
     {
	  if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
	  {
	       TranslateMessage(&msg);
	       DispatchMessage(&msg);
	  }
     }

     return msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    It is important to call this function so that the application 
//    will get 'well formed' small icons associated with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance, LPTSTR szWindowClass)
{
     WNDCLASS	wc;

     wc.style			= CS_HREDRAW | CS_VREDRAW;
     wc.lpfnWndProc		= (WNDPROC) WndProc;
     wc.cbClsExtra		= 0;
     wc.cbWndExtra		= 0;
     wc.hInstance		= hInstance;
     //wc.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_BOOTBLASTER));
     wc.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CHIP));
     wc.hCursor			= 0;
     wc.hbrBackground	= (HBRUSH) GetStockObject(WHITE_BRUSH);
     wc.lpszMenuName		= 0;
     wc.lpszClassName	= szWindowClass;

     return RegisterClass(&wc);
}

//
//  FUNCTION: InitInstance(HANDLE, int)
//
//  PURPOSE: Saves instance handle and creates main window
//
//  COMMENTS:
//
//    In this function, we save the instance handle in a global variable and
//    create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, LPTSTR lpCmdLine, int nCmdShow)
{
     HWND	hWnd = NULL;
     TCHAR	szTitle[MAX_LOADSTRING];			// The title bar text
     TCHAR	szWindowClass[MAX_LOADSTRING];		// The window class name

     hInst = hInstance;		// Store instance handle in our global variable
     // Initialize global strings
     LoadString(hInstance, IDC_BOOTBLASTER, szWindowClass, MAX_LOADSTRING);
     LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);

     //If it is already running, then focus on the window
     hWnd = FindWindow(szWindowClass, szTitle);	
     if (hWnd) 
     {
	  SetForegroundWindow ((HWND) (((DWORD)hWnd) | 0x01));    
	  return 0;
     } 

     MyRegisterClass(hInstance, szWindowClass);
	
     RECT	rect;
     GetClientRect(hWnd, &rect);
	
     hWnd = CreateWindow(szWindowClass, szTitle, WS_VISIBLE,
			 CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
     if (!hWnd)
     {	
	  return FALSE;
     }
     //When the main window is created using CW_USEDEFAULT the height of the menubar (if one
     // is created is not taken into account). So we resize the window after creating it
     // if a menubar is present
     {
	  RECT rc;
	  GetWindowRect(hWnd, &rc);
	  rc.bottom -= MENU_HEIGHT;
	  if (hwndCB)
	       MoveWindow(hWnd, rc.left, rc.top, rc.right, rc.bottom, FALSE);
     }


     ShowWindow(hWnd, nCmdShow);
     //LoadString(hInst, IDS_IDLE, szStatus, MAX_LOADSTRING);
     if (lpCmdLine != NULL && lpCmdLine[0] != 0) {
       wsprintf(szStatus,TEXT ("%s"),(const unsigned short *) lpCmdLine);
     } else {
       TCHAR *szTmp = szStatus;
       int i;
       for (i = 0; idle_strings[i]; i++) {
		szTmp += wsprintf(szTmp,TEXT ("    %s"), idle_strings[i]);
	   }
	 }
     UpdateWindow(hWnd);

     return TRUE;
}

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
     HDC hdc;
     int wmId, wmEvent;
     PAINTSTRUCT ps;

     switch (message) 
     {
     case WM_COMMAND:
	  wmId    = LOWORD(wParam); 
	  wmEvent = HIWORD(wParam); 
	  // Parse the menu selections:
	  switch (wmId)
	  {	
	  case IDM_HELP_ABOUT:
	       DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
	       break;
	  case IDM_FILE_EXIT:
	       SendMessage (hWnd, WM_CLOSE, 0, 0);
	       break;
	  case IDM_FLASH_ERASE:
	       if (gSystemErrorHasOccured != TRUE){
		    LoadString(hInst, IDS_FLASH_ERASE, szStatus, MAX_LOADSTRING);
		    InvalidateRect(hWnd,NULL,TRUE);
		    UpdateWindow(hWnd);
		    Sleep(100);
		    EraseFlash(hWnd);
		    LoadString(hInst, IDS_IDLE, szStatus, MAX_LOADSTRING);
		    InvalidateRect(hWnd,NULL,TRUE);
		    UpdateWindow(hWnd);
		    Sleep(100);
	       }
	       else
		    SystemErrorAlert(hWnd);
	       break;
	  case IDM_FLASH_TEST_TIMED:
	       LoadString(hInst, IDS_FLASH_TEST_TIMED, szStatus, MAX_LOADSTRING);
	       InvalidateRect(hWnd,NULL,TRUE);
	       gTimerID = SetTimer(hWnd,BB_TEST_FLASH,50,NULL);
	       break;
	  case IDM_FLASH_TEST_STRAIGHT:
	       TestFlash(hWnd);
	       break;
	  case IDM_FLASH_SAVE:
	       LoadString(hInst, IDS_FLASH_SAVE_BL, szStatus, MAX_LOADSTRING);					
	       InvalidateRect(hWnd,NULL,TRUE);
	       UpdateWindow(hWnd);
	       Sleep(100);
	       SaveFlash(hWnd);
	       LoadString(hInst, IDS_IDLE, szStatus, MAX_LOADSTRING);
	       InvalidateRect(hWnd,NULL,TRUE);
	       UpdateWindow(hWnd);
	       Sleep(100);
	       break;
	  case IDM_FLASH_SAVE_WINCE:
	       LoadString(hInst, IDS_FLASH_SAVE_WINCE, szStatus, MAX_LOADSTRING);					
	       InvalidateRect(hWnd,NULL,TRUE);
	       UpdateWindow(hWnd);
	       Sleep(100);
	       SaveFlashWince(hWnd);
	       LoadString(hInst, IDS_IDLE, szStatus, MAX_LOADSTRING);
	       InvalidateRect(hWnd,NULL,TRUE);
	       UpdateWindow(hWnd);
	       Sleep(100);
	       break;
	  case IDM_FLASH_PROTECT_WINCE:
	       LoadString(hInst, IDS_FLASH_PROTECT_WINCE, szStatus, MAX_LOADSTRING);					
	       InvalidateRect(hWnd,NULL,TRUE);
	       UpdateWindow(hWnd);
	       Sleep(100);
	       ProtectWince(hWnd);
	       LoadString(hInst, IDS_IDLE, szStatus, MAX_LOADSTRING);
	       InvalidateRect(hWnd,NULL,TRUE);
	       UpdateWindow(hWnd);
	       Sleep(100);
	       break;
	  case IDM_FLASH_SAVE_GZ:
	       LoadString(hInst, IDS_FLASH_SAVE_BL_GZ, szStatus, MAX_LOADSTRING);					
	       InvalidateRect(hWnd,NULL,TRUE);
	       UpdateWindow(hWnd);
	       Sleep(100);
	       SaveFlashGZ(hWnd);
	       LoadString(hInst, IDS_IDLE, szStatus, MAX_LOADSTRING);
	       InvalidateRect(hWnd,NULL,TRUE);
	       UpdateWindow(hWnd);
	       Sleep(100);
	       break;
	  case IDM_REGDUMP:
	       LoadString(hInst, IDS_REGDUMP, szStatus, MAX_LOADSTRING);					
	       InvalidateRect(hWnd,NULL,TRUE);
	       UpdateWindow(hWnd);
	       Sleep(100);
	       RegDump(hWnd);
	       LoadString(hInst, IDS_IDLE, szStatus, MAX_LOADSTRING);
	       InvalidateRect(hWnd,NULL,TRUE);
	       UpdateWindow(hWnd);
	       Sleep(100);
	       break;
	  case IDM_FLASH_PROGRAM:
	       if (gSystemErrorHasOccured != TRUE){
		    if (LoadImage(hWnd) != TRUE) //loaded, so program it
			 return 0;

		    LoadString(hInst, IDS_FLASH_PROTECT_WINCE, szStatus, MAX_LOADSTRING);
		    InvalidateRect(hWnd,NULL,TRUE);
		    UpdateWindow(hWnd);
		    Sleep(100);
		    ProtectWince(hWnd);

		    if (gSystemErrorHasOccured == TRUE)
			    return 0;

		    LoadString(hInst, IDS_FLASH_ERASE, szStatus, MAX_LOADSTRING);
		    InvalidateRect(hWnd,NULL,TRUE);
		    UpdateWindow(hWnd);
		    Sleep(100);
		    EraseFlash(hWnd);

		    LoadString(hInst, IDS_FLASH_PROGRAM, szStatus, MAX_LOADSTRING);
		    InvalidateRect(hWnd,NULL,FALSE);
		    UpdateWindow(hWnd);
		    Sleep(100);
		    if (ProgramFlash(hWnd) == TRUE)
			 VerifyFlash(hWnd);						
		    LoadString(hInst, IDS_IDLE, szStatus, MAX_LOADSTRING);
		    InvalidateRect(hWnd,NULL,TRUE);
		    UpdateWindow(hWnd);
		    Sleep(100);
	       }
	       else
		    SystemErrorAlert(hWnd);
	       break;
	  case IDM_FLASH_PROGRAM_XPLE:
	       if (gSystemErrorHasOccured != TRUE){
		    if (LoadImage(hWnd) != TRUE) //loaded, so program it
			 return 0;
		    for (int i=0; i < 50; i++){

			 LoadString(hInst, IDS_FLASH_PROTECT_WINCE, szStatus, MAX_LOADSTRING);
			 InvalidateRect(hWnd,NULL,TRUE);
			 UpdateWindow(hWnd);
			 Sleep(100);
			 ProtectWince(hWnd);

			 if (gSystemErrorHasOccured == TRUE)
				 return 0;

			 LoadString(hInst, IDS_FLASH_ERASE, szStatus, MAX_LOADSTRING);
			 InvalidateRect(hWnd,NULL,TRUE);
			 UpdateWindow(hWnd);
			 Sleep(100);
			 EraseFlash(hWnd);

			 LoadString(hInst, IDS_FLASH_PROGRAM, szStatus, MAX_LOADSTRING);
			 InvalidateRect(hWnd,NULL,FALSE);
			 UpdateWindow(hWnd);
			 Sleep(100);
			 ProgramFlash(hWnd);
			 LoadString(hInst, IDS_IDLE, szStatus, MAX_LOADSTRING);
			 InvalidateRect(hWnd,NULL,TRUE);
			 UpdateWindow(hWnd);
			 Sleep(100);
		    }
		    VerifyFlash(hWnd);
	       }
	       else
		    SystemErrorAlert(hWnd);
	       break;
				
	  case IDM_FLASH_SHOW:
	       ShowFlash(hWnd);
	       break;
	  case IDM_FLASH_VERIFY:
	       VerifyFlash(hWnd);
	       break;
	  case IDOK:
	       //SendMessage(hWnd, WM_ACTIVATE, MAKEWPARAM(WA_INACTIVE, 0), (LPARAM)hWnd);
	       //SendMessage (hWnd, WM_CLOSE, 0, 0);
	       break;
	  default:
	       return DefWindowProc(hWnd, message, wParam, lParam);
	  }
	  break;
     case WM_TIMER:
	  // this method isn;t used anymore, really
	  KillTimer(hWnd,wParam);
	  if (wParam == BB_TEST_FLASH){
	       TestFlash(hWnd);
	       LoadString(hInst, IDS_IDLE, szStatus, MAX_LOADSTRING);
	       InvalidateRect(hWnd,NULL,TRUE);
	  }
	  else{}

     case WM_CREATE:
	  hwndCB = CreateRpCommandBar(hWnd);
	  break;
     case WM_PAINT:
	  RECT rt;
	  hdc = BeginPaint(hWnd, &ps);
	  GetClientRect(hWnd, &rt);
	  
	  DrawText(hdc, szStatus, _tcslen(szStatus), &rt, 
		   ((szStatus[0] == '\n') ? DT_LEFT : DT_CENTER));
	  EndPaint(hWnd, &ps);
	  break; 
     case WM_DESTROY:
	  CommandBar_Destroy(hwndCB);
	  PostQuitMessage(0);
	  break;
     case WM_SETTINGCHANGE:
	  SHHandleWMSettingChange(hWnd, wParam, lParam, &s_sai);
	  break;
     default:
	  return DefWindowProc(hWnd, message, wParam, lParam);
     }
     return 0;
}

HWND CreateRpCommandBar(HWND hwnd)
{
     SHMENUBARINFO mbi;

     memset(&mbi, 0, sizeof(SHMENUBARINFO));
     mbi.cbSize     = sizeof(SHMENUBARINFO);
     mbi.hwndParent = hwnd;
     mbi.nToolBarId = IDM_MENU;
     mbi.hInstRes   = hInst;
     mbi.nBmpId     = 0;
     mbi.cBmpImages = 0;

     if (!SHCreateMenuBar(&mbi)) 
	  return NULL;

     return mbi.hwndMB;
}

// Mesage handler for the About box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
     SHINITDLGINFO shidi;

     switch (message)
     {
     case WM_INITDIALOG:
	  // Create a Done button and size it.  
	  shidi.dwMask = SHIDIM_FLAGS;
	  shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN;
	  shidi.hDlg = hDlg;
	  SHInitDialog(&shidi);
	  return TRUE; 

     case WM_COMMAND:
	  if (LOWORD(wParam) == IDOK) {
	       EndDialog(hDlg, LOWORD(wParam));
	       return TRUE;
	  }
	  break;
     }
     return FALSE;
}


void init()
{
#if 0
     TCHAR *logfilename = (TCHAR *)"BootBlaster3900.log"
     gLogFileHandle = CreateFile(logfilename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
#endif
     initFlash(); // set up initial flash info
}
void DisplayError(TCHAR * pFile,UINT line)
{
     DWORD error;
     LPVOID lpMsgBuf;
     int ret;    
     TCHAR szText[256];


     gSystemErrorHasOccured = TRUE;

     error = GetLastError();
	
     ret = FormatMessage( 
	  FORMAT_MESSAGE_ALLOCATE_BUFFER | 
	  FORMAT_MESSAGE_FROM_SYSTEM | 
	  FORMAT_MESSAGE_IGNORE_INSERTS,
	  NULL,
	  error,
	  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
	  (LPTSTR) &lpMsgBuf,
	  0,
	  NULL 
	  );

     //doesn't work, don't know why
     //bRet = SendMessage(hwndCB, TB_ENABLEBUTTON , (WPARAM) IDM_FLASH_PROGRAM, 
     //	(LPARAM) MAKELONG(FALSE,0));
     //DrawMenuBar(hwndCB);

     wsprintf (szText, TEXT ("Error #%d gives error string %s from %s:%d"),error,
	       lpMsgBuf,pFile,line);
     if (gLogFileHandle != INVALID_HANDLE_VALUE) {
       DWORD dwBytesWritten;
       WriteFile(gLogFileHandle, (void *)&szText, _tcslen(szText) * sizeof(TCHAR), &dwBytesWritten, 0);
     }

     MessageBox(NULL, szText, _T("Error"), MB_OK);
     // Free the buffer.
     LocalFree( lpMsgBuf );
}

HWND MakeProgressBar(HWND hWnd)
{
     RECT rcClient;  // client area of parent window
     int cyVScroll;  // height of scroll bar arrow
     HWND hwndPB = NULL;    // handle of progress bar
     HANDLE hFile;   // handle of file
     DWORD cb;       // size of file and count of bytes read
     LPCH pch;       // address of data read from file
     LPCH pchTmp;    

     GetClientRect(hWnd, &rcClient);

     cyVScroll = GetSystemMetrics(SM_CYVSCROLL);
     hwndPB = CreateWindowEx(0, PROGRESS_CLASS, (unsigned short *) NULL,
			     WS_CHILD | WS_VISIBLE, rcClient.left,
			     rcClient.bottom - 100,
			     rcClient.right, 30,
			     hWnd, (HMENU) 0, hInst, NULL);


     return hwndPB;

}

void SystemErrorAlert(HWND hWnd)
{
     TCHAR szText[256];

     wsprintf (szText, 
	       TEXT ("Since BootBlaster has encountered a  system error, destructive actions are disabled for this session."));

     MessageBox(NULL, szText, _T("Warning"), MB_OK|MB_ICONEXCLAMATION);
     // Free the buffer.

}
