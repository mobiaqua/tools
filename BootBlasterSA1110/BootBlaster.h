
#if !defined(AFX_BOOTBLASTER_H__BDCBB719_CD62_11D5_A244_00508BA08C36__INCLUDED_)
#define AFX_BOOTBLASTER_H__BDCBB719_CD62_11D5_A244_00508BA08C36__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"

#define MENU_HEIGHT 26
#define APP_NAME_STR "BootBlaster"

//PROTOS
void initFlash(void);
void init(void);
void DisplayError(TCHAR *pF,UINT l);
BOOL ProgramFlash(HWND hWnd);
BOOL EraseFlash(HWND hWnd);
BOOL LoadImage(HWND hWnd);
void ShowFlash(HWND hWnd);
HWND MakeProgressBar(HWND hWnd);
void VerifyFlash(HWND hWnd);
void SaveFlash(HWND hWnd);
void ProtectWince(HWND hWnd);
void TestFlash(HWND hWnd);
void SaveFlashGZ(HWND hWnd);
void SaveFlashWince(HWND hWnd);
void SystemErrorAlert(HWND hWnd);
#endif // !defined(AFX_BOOTBLASTER_H__BDCBB719_CD62_11D5_A244_00508BA08C36__INCLUDED_)
