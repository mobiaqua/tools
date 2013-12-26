// BootBlaster.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "BootBlaster1910.h"
#include "FlashUtils.h"
#include <commctrl.h>
#include <aygshell.h>
#include <sipapi.h>
#include <windows.h>
#include <winuser.h>
#include <commdlg.h> 
#include <stdlib.h>

static volatile unsigned long *flashword = (volatile unsigned long*)0x0;
static volatile unsigned long *interrupt_flash_reggie = (volatile unsigned long*)0x0;
static unsigned long old_ICMR_value;

static volatile unsigned long pFile = NULL;
static unsigned long fileSize = 0;

#define MAX_LOADSTRING 1023

static TCHAR gErrorText[1024];
extern BOOL	gSystemErrorHasOccured;	
extern HINSTANCE			hInst;					// The current instance
extern TCHAR szStatus[MAX_LOADSTRING];				// current operation Status

void initFlash()
{
     flashword = (volatile unsigned long *)0xA2000000;
     interrupt_flash_reggie = (volatile unsigned long *)(0xBF700004);	
	 old_ICMR_value = ((unsigned long) *interrupt_flash_reggie) & 0xffff;
}

void SaveFlash(HWND hWnd)
{
     TCHAR szFilename[128];
     TCHAR szText[128];

     if (MessageBox(hWnd, _T("This will save the contents of your bootloader segment to the file \\My Documents\\saved_bootldr.bin. Do you wish to proceed?"), 
		    _T("BootBlaster1910"), 
		    MB_YESNO) != IDYES)
	  return ;

     wsprintf (szFilename, TEXT ("\\My Documents\\saved_bootldr.bin"));
     if (save_flash(hWnd,flashword,256*K_1,szFilename) == TRUE)
     {
	  wsprintf (szText, TEXT ("Bootldr successfully saved to \\My Documents\\saved_bootldr.bin.  Please copy it to your desktop machine for safekeeping."));
	  MessageBox (hWnd, szText, _T("BootBlaster1910"), MB_OK);
     }
	
}

static BOOL save_flash(HWND hWnd,volatile unsigned long *p,unsigned long size,TCHAR *fName)
{
     HANDLE			hHexFile = 0;
     DWORD			dwCount = 0;
     DWORD			dwOpcode;
     DWORD			dwBytesWritten;
     TCHAR szText[128];

     hHexFile = CreateFile(fName, GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
     if (hHexFile != INVALID_HANDLE_VALUE){
	  // the file exists
	  CloseHandle(hHexFile);
	  wsprintf (szText, TEXT ("The file \\My Documents\\saved_bootldr.bin already exists.  Do you want to overwrite it?"));
	  if (MessageBox (hWnd, szText, _T("BootBlaster1910"), MB_YESNO) != IDYES)
	       return FALSE;
     }
	
	
     hHexFile = CreateFile(fName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

     if (hHexFile == INVALID_HANDLE_VALUE){
	  DisplayError(TEXT(__FILE__), __LINE__);
	  return FALSE;
     }
     for (dwCount = 0;(dwCount * sizeof(long)) < size;dwCount++)
     {
	  dwOpcode = p[dwCount];
	  if ( !WriteFile(hHexFile, (void *)&dwOpcode, sizeof(dwOpcode), &dwBytesWritten, 0) )
	  {
	       DisplayError(TEXT(__FILE__), __LINE__);
	       CloseHandle(hHexFile);
	       return FALSE;
	  }
	  if (dwBytesWritten != sizeof(dwOpcode))
	  {
	       DisplayError(TEXT(__FILE__), __LINE__);
	       CloseHandle(hHexFile);
	       return FALSE;
	  }
     }
     CloseHandle(hHexFile);
     return TRUE;
}

static BOOL isValidBootloader(unsigned long p,unsigned long size);

BOOL LoadImage(HWND hWnd)
{
     TCHAR szFileName[MAX_PATH];
     TCHAR szText[128];

     if (MessageBox(hWnd, _T("This will overwrite your bootloader.  The operation takes 6 seconds.  YOU MUST NOT RESET YOUR IPAQ DURING THIS OPERATION!  BE PATIENT! Are you sure you wish to proceed?"), 
		    _T("BootBlaster1910") _T(" Warning"), 
		    MB_ICONEXCLAMATION|MB_YESNO) != IDYES)
	  return FALSE;


     if (FindFile(hWnd, szFileName, dim(szFileName)) == 0)
	  return FALSE;
	
     TCHAR *suffix;
    
	
     suffix = szFileName + lstrlen(szFileName) - 3;
     if (!lstrcmp(suffix-1,_T(".bin"))){
	  if (!(pFile = LoadFile(szFileName,&fileSize))){
	       MessageBox(hWnd, _T("Could not open file \r\n"),
			  _T(APP_NAME_STR) _T(" Error"), MB_OK);
		
	       return FALSE;
	  }
     }
     else{
	  wsprintf (szText, TEXT ("%s is an unsupported file type, try a .bin file, please!") ,szFileName);
	  MessageBox (hWnd, szText, _T("BootBlaster1910") _T(" Error"), MB_OK);
	  return FALSE;
     }

     if (!isValidBootloader(pFile,fileSize)){
          TCHAR szText[128];
          wsprintf (szText, TEXT ("%s is not a valid bootloader image [%x,%x,%x]") ,
                    szFileName,
                    *(long *)(pFile + 0), *(long*)(pFile + 0x40), *(long*)(pFile + 0x1000));
          MessageBox (hWnd, szText, _T("BootBlaster") _T(" Error"), MB_OK);
          return FALSE;
     }
 
	 wsprintf (szText, TEXT ("You have chosen to flash the file: %s.  Is this correct?"),szFileName);
     if (MessageBox(hWnd,szText, 
		    _T("BootBlaster1910"), 
		    MB_YESNO) != IDYES)
	  return FALSE ;
	
     return TRUE;
}

static BOOL isValidBootloader(unsigned long p,unsigned long size)
{
     BOOL ret = TRUE;

     if (size != 256*K_1)
          return FALSE;

	 if (*(unsigned long *)(p + 0x40) != 0x43454345) // ECEC
          return FALSE;

	 if (*(unsigned long *)(p + 0x1004) != 0x746f6f42) // Boot
          return FALSE;

	 if ((*(unsigned long *)(p + 0x1014) != 0x30312e31) && // 1.10
		 (*(unsigned long *)(p + 0x1014) != 0x32302e31))   // 1.02
          return FALSE;

     return ret;
}

static void flashProgramWord(unsigned long address, unsigned long value);

BOOL ProgramFlash(HWND hWnd)
{
     HWND hwndPB = NULL;
     TCHAR szText[128];
     int ret;

     //interrupts_off();
     ret = program_flash(pFile,fileSize,hWnd);
     //interrupts_on();

     if (hwndPB)
	  DestroyWindow(hwndPB);
	
     if (ret){
	  DisplayLLErrorText(hWnd,gErrorText,TEXT(__FILE__),__LINE__);
	  return FALSE;
     }
     else {
	  wsprintf (szText, TEXT ("Bootldr successfully installed."));
	  MessageBox (hWnd, szText, _T("BootBlaster1910"), MB_OK);
	  return TRUE;
	 }
}

static void set_interrupts(int on)
{
     if (on){
	  *interrupt_flash_reggie = old_ICMR_value;

     }
     else{
	  old_ICMR_value = (unsigned long) *interrupt_flash_reggie;
	  *interrupt_flash_reggie = 0x0;
     }
}

static void interrupts_on(void)
{
     set_interrupts(1);
}

static void interrupts_off(void)
{
     set_interrupts(0);
}

static void flashErase(unsigned long block);
static unsigned long queryFlashID();

static int program_flash(unsigned long p,unsigned long size,HWND hWnd)
{
     int i;
     unsigned long remaining_bytes;

	 i = queryFlashID();
	 if (i != 0x22b9) {
		MessageBox (hWnd, _T("Error: it's not iPAQ h1910 !"), _T("BootBlaster1910"), MB_OK);
		return -1;
	 }

     LoadString(hInst, IDS_FLASH_ERASE, szStatus, MAX_LOADSTRING);
     InvalidateRect(hWnd,NULL,FALSE);
     UpdateWindow(hWnd);
     Sleep(100);

	 flashErase(0x00000);
	 flashErase(0x10000);
	 flashErase(0x20000);
	 flashErase(0x30000);

     LoadString(hInst, IDS_FLASH_PROGRAM, szStatus, MAX_LOADSTRING);
     InvalidateRect(hWnd,NULL,FALSE);
     UpdateWindow(hWnd);
     Sleep(100);

     i = 0;
     remaining_bytes = size;
     while(remaining_bytes > 0) {
      flashProgramWord(i, *(unsigned long *)(p+i));
	  i += 4;
	  remaining_bytes -= 4;
     }

	 flash_write_cmd(0, 0xff); /*  flashword[0] = bothbanks(0xFF); */

     LoadString(hInst, IDS_FLASH_VERIFY, szStatus, MAX_LOADSTRING);
     InvalidateRect(hWnd,NULL,FALSE);
     UpdateWindow(hWnd);
     Sleep(100);

     i = 0;
     remaining_bytes = size;
     while(remaining_bytes > 0) {
		if (*(unsigned long *)(flashword + (i/4)) != *(unsigned long *)(p+i)) {
			MessageBox (hWnd, _T("Fatal error: Verify write error"), _T("BootBlaster1910"), MB_OK);
			return -1;
		}
		i += 4;
		remaining_bytes -= 4;
     }

     return 0;
}


static unsigned long LoadFile(const TCHAR *fname,unsigned long *ulSize)
{
     unsigned long lpv;
     HANDLE			hHexFile;
     DWORD			dwCount = 0;
     DWORD			dwOpcode;
     DWORD			dwFileSize ;
     DWORD			dwBytesRead;
     unsigned long *p;

   
     if ((hHexFile = CreateFile(fname, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0)) ==
	 INVALID_HANDLE_VALUE)
	
	  return 0;
    
     dwFileSize = GetFileSize(hHexFile, 0);
     *ulSize = (unsigned long) dwFileSize;
    
     if (!(lpv = (unsigned long) VirtualAlloc(0,256*K_1, MEM_COMMIT,PAGE_READWRITE)))
	  return lpv;
     p = (unsigned long *) lpv;    
     for (dwCount=0; (dwCount*4)<dwFileSize; dwCount++ ){
	  if ( !ReadFile(hHexFile, (void *)&dwOpcode, sizeof(dwOpcode), &dwBytesRead, 0) )	
	       dwCount = dwFileSize;
	  if (dwBytesRead != sizeof(dwOpcode))
	       dwCount = dwFileSize;
	  *p++ = dwOpcode;
     }
     if (dwCount == dwFileSize){ // errored out
		
	  lpv = 0;
     }
     CloseHandle(hHexFile);
     return lpv;
    
}


//----------------------------------------------------------------------
// FindFile - Returns a filename using the common dialog.
//
static INT FindFile (HWND hWnd, LPTSTR szFileName, INT nMax) {
     OPENFILENAME of;
     const LPTSTR pszOpenFilter = TEXT ("All Documents (*.*)\0*.*\0\0");

     szFileName[0] = '\0';                 // Initial filename
     memset (&of, 0, sizeof (of));         // Initial file open structure

     of.lStructSize = sizeof (of);
     of.hwndOwner = hWnd;
     of.lpstrFile = szFileName;
     of.nMaxFile = nMax;
     of.lpstrFilter = pszOpenFilter;
     of.Flags = 0;

     if (GetOpenFileName (&of))
	  return lstrlen (szFileName);
     else
	  return 0;
}

static unsigned long bothbanks(unsigned long w);

static void flash_write_val(unsigned addr,int val)
{
     char *dst;
    
	 dst = (char*)flashword + (addr & ~0x1);
	 *((unsigned short*)dst) = (unsigned short)bothbanks(val);
}

static void flash_write_cmd(unsigned cmd_addr,int cmd)
{
     char *dst;
    
	 dst = (char*)flashword + (cmd_addr << 1);
	 *((unsigned short*)dst) = (unsigned short)bothbanks(cmd);
}

static unsigned long flash_read_val(unsigned addr)
{
     char *dst;

	 dst = (char*)flashword + addr;
	 return (*((unsigned short*)dst));
}

static void flashProgramWord(unsigned long address, unsigned long value)
{
	 /* put flash in query mode */
	 flash_write_cmd(0x555, 0xaa); /*  flashword[0x555] = bothbanks(0xAA); */
	 flash_write_cmd(0x2aa, 0x55); /*  flashword[0x2AA] = bothbanks(0x55); */
	 flash_write_cmd(0x555, 0xa0); /*  flashword[0x555] = bothbanks(0xA0); */
	 flash_write_val(address + 2, value >> 16); /*  flashword[address + 1] = bothbanks(value); */
	 Delay(20);

	 /* put flash in query mode */
	 flash_write_cmd(0x555, 0xaa); /*  flashword[0x555] = bothbanks(0xAA); */
	 flash_write_cmd(0x2aa, 0x55); /*  flashword[0x2AA] = bothbanks(0x55); */
	 flash_write_cmd(0x555, 0xa0); /*  flashword[0x555] = bothbanks(0xA0); */
	 flash_write_val(address, value & 0xffff); /*  flashword[address] = bothbanks(value); */
	 Delay(20);
}

static void flashErase(unsigned long block)
{
	 /* put flash in query mode */
	 flash_write_cmd(0x555, 0xaa); /*  flashword[0x555] = bothbanks(0xAA); */
	 flash_write_cmd(0x2aa, 0x55); /*  flashword[0x2AA] = bothbanks(0x55); */
	 flash_write_cmd(0x555, 0x80); /*  flashword[0x555] = bothbanks(0x80); */
	 flash_write_cmd(0x555, 0xaa); /*  flashword[0x555] = bothbanks(0xAA); */
	 flash_write_cmd(0x2aa, 0x55); /*  flashword[0x2AA] = bothbanks(0x55); */
	 flash_write_val(block, 0x30); /*  flashword[block] = bothbanks(0x30); */

	 int result;
	 for (;;) {
		 result = flash_read_val(block);
		 if ((result & 0x80) != 0)
			 break;
		 if ((result & 0x20) != 0)
			 break;
	 }

	 flash_write_cmd(0, 0xf0); /*  flashword[0] = bothbanks(0xF0); */
}

static unsigned long queryFlashID()
{
     unsigned long result;

	 /* put flash in query mode */
	 flash_write_cmd(0x555, 0xaa); /*  flashword[0x555] = bothbanks(0xAA); */
	 flash_write_cmd(0x2aa, 0x55); /*  flashword[0x2AA] = bothbanks(0x55); */
	 flash_write_cmd(0x555, 0x90); /*  flashword[0x555] = bothbanks(0x90); */

     /* read autoselect word */
     result = flash_read_val(0); /* flashword[0]; */
     result = flash_read_val(2); /* flashword[1]; */

	 return result;
}

static void Delay(unsigned long count)
{
     volatile unsigned long i;
     volatile double faux;

     for(i=0;i < count; i++)
	  faux = (faux + (double) i)/((double) i);
}

static unsigned long bothbanks(unsigned long w)
{
	  return ((((w)&0xFFFF) << 16)|((w)&0xFFFF));
}

static void DisplayLLErrorText(HWND hWnd,TCHAR *pM,TCHAR *pF,UINT line)
{	
     TCHAR szText[1200];

     gSystemErrorHasOccured = TRUE;

     wsprintf(szText,TEXT("error: %s from %s:%d"),
	      pM,pF,line);

     MessageBox (hWnd, szText, _T("BootBlaster1910 Low Level Error"), MB_OK);
}


