// BootBlaster.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "BootBlaster.h"
#include "FlashUtils.h"
#include "sa1100.h"
#include <commctrl.h>
#include <aygshell.h>
#include <sipapi.h>
#include <windows.h>
#include <winuser.h>
#include <commdlg.h> 
#include <stdlib.h>
#include "btflash.h"
//#include "oemioctl.h"
//#include "pkfuncs.h"

//zlib
#include "zlib.h"

static int  bt_flash_organization = BT_FLASH_ORGANIZATION_2x16;
static volatile unsigned long *flashword = (volatile unsigned long*)0x0;
static volatile unsigned long *interrupt_flash_reggie = (volatile unsigned long*)0x0;
static unsigned long old_ICMR_value;
static volatile unsigned short *flashvpp = (volatile unsigned short*)0x0;
static volatile unsigned short *gpio_base = (volatile unsigned short*)0x0;

static unsigned long flash_size = 0;
static unsigned long flash_address_mask = -1;
static int nsectors = 0;
static int bootloader_sector_end = 0; // in array, where bootldr ends. (1 for 32 bit,2 for 16)
static unsigned long *flashSectors = NULL;
static volatile unsigned long pFile = NULL;
static unsigned long fileSize = 0;
static BOOL hasProgrammed = 0;
static unsigned long mach_type = MACH_TYPE_H3600;

static TCHAR gErrorText[1024];
extern BOOL	gSystemErrorHasOccured;	


static unsigned long flashSectors_28F128J3A_1x16[] = {
  0x00000000, 
  0x00040000/2, 
  0x00080000/2, 
  0x000c0000/2, 
  0x00100000/2, 
  0x00140000/2, 
  0x00180000/2, 
  0x001c0000/2, 
  0x00200000/2, 
  0x00240000/2, 
  0x00280000/2, 
  0x002c0000/2, 
  0x00300000/2, 
  0x00340000/2, 
  0x00380000/2, 
  0x003c0000/2, 
  0x00400000/2, 
  0x00440000/2, 
  0x00480000/2, 
  0x004c0000/2, 
  0x00500000/2, 
  0x00540000/2, 
  0x00580000/2, 
  0x005c0000/2, 
  0x00600000/2, 
  0x00640000/2, 
  0x00680000/2, 
  0x006c0000/2, 
  0x00700000/2, 
  0x00740000/2, 
  0x00780000/2, 
  0x007c0000/2, 
  0x00800000/2, 
  0x00840000/2, 
  0x00880000/2, 
  0x008c0000/2, 
  0x00900000/2, 
  0x00940000/2, 
  0x00980000/2, 
  0x009c0000/2, 
  0x00a00000/2, 
  0x00a40000/2, 
  0x00a80000/2, 
  0x00ac0000/2, 
  0x00b00000/2, 
  0x00b40000/2, 
  0x00b80000/2, 
  0x00bc0000/2, 
  0x00c00000/2, 
  0x00c40000/2, 
  0x00c80000/2, 
  0x00cc0000/2, 
  0x00d00000/2, 
  0x00d40000/2, 
  0x00d80000/2, 
  0x00dc0000/2, 
  0x00e00000/2, 
  0x00e40000/2, 
  0x00e80000/2, 
  0x00ec0000/2, 
  0x00f00000/2, 
  0x00f40000/2, 
  0x00f80000/2, 
  0x00fc0000/2, 
  0x01000000/2, 
  0x01040000/2, 
  0x01080000/2, 
  0x010c0000/2, 
  0x01100000/2, 
  0x01140000/2, 
  0x01180000/2, 
  0x011c0000/2, 
  0x01200000/2, 
  0x01240000/2, 
  0x01280000/2, 
  0x012c0000/2, 
  0x01300000/2, 
  0x01340000/2, 
  0x01380000/2, 
  0x013c0000/2, 
  0x01400000/2, 
  0x01440000/2, 
  0x01480000/2, 
  0x014c0000/2, 
  0x01500000/2, 
  0x01540000/2, 
  0x01580000/2, 
  0x015c0000/2, 
  0x01600000/2, 
  0x01640000/2, 
  0x01680000/2, 
  0x016c0000/2, 
  0x01700000/2, 
  0x01740000/2, 
  0x01780000/2, 
  0x017c0000/2, 
  0x01800000/2, 
  0x01840000/2, 
  0x01880000/2, 
  0x018c0000/2, 
  0x01900000/2, 
  0x01940000/2, 
  0x01980000/2, 
  0x019c0000/2, 
  0x01a00000/2, 
  0x01a40000/2, 
  0x01a80000/2, 
  0x01ac0000/2, 
  0x01b00000/2, 
  0x01b40000/2, 
  0x01b80000/2, 
  0x01bc0000/2, 
  0x01c00000/2, 
  0x01c40000/2, 
  0x01c80000/2, 
  0x01cc0000/2, 
  0x01d00000/2, 
  0x01d40000/2, 
  0x01d80000/2, 
  0x01dc0000/2, 
  0x01e00000/2, 
  0x01e40000/2, 
  0x01e80000/2, 
  0x01ec0000/2, 
  0x01f00000/2, 
  0x01f40000/2, 
  0x01f80000/2, 
  0x01fc0000/2, 
  0x02000000/2 /* guard sector */
};


static FlashAlgorithm intelFlashAlgorithm_1x16 = {
   intelFlashReset,
   intelFlashProgramWord_1x16,
   intelFlashProgramBlock_1x16,
   intelFlashEraseChip,		/* not implemented anyway */
   intelFlashEraseSector, /*  intelFlashEraseSector_1x16, */
   intelFlashEraseRange, /*  intelFlashEraseRange_1x16,	*/ /* will be ok if we fix erase sector */
   intelFlashProtectRange_1x16 
};

static FlashDescriptor flashDescriptor_28F128J3A_1x16 = {
   "28F128J3A_1x16", 
   0x89, 0x18, 
   &intelFlashAlgorithm_1x16,
   sizeof(flashSectors_28F128J3A_1x16)/sizeof(dword) - 1,
   flashSectors_28F128J3A_1x16,
   { "bootldr",   0x00000000, 0x00040000, LFR_PATCH_BOOTLDR },
   //{ "params",    0x00020000, 0x00020000, 0 },
   //{ "kernel",    0x00040000, 0x000c0000, 0 }
   { "root",    0x00040000, 0x0, LFR_EXPAND|LFR_JFFS2 },
};


static unsigned long flashSectors_28F128J3A[] = {
  0x00000000, 
  0x00040000, 
  0x00080000, 
  0x000c0000, 
  0x00100000, 
  0x00140000, 
  0x00180000, 
  0x001c0000, 
  0x00200000, 
  0x00240000, 
  0x00280000, 
  0x002c0000, 
  0x00300000, 
  0x00340000, 
  0x00380000, 
  0x003c0000, 
  0x00400000, 
  0x00440000, 
  0x00480000, 
  0x004c0000, 
  0x00500000, 
  0x00540000, 
  0x00580000, 
  0x005c0000, 
  0x00600000, 
  0x00640000, 
  0x00680000, 
  0x006c0000, 
  0x00700000, 
  0x00740000, 
  0x00780000, 
  0x007c0000, 
  0x00800000, 
  0x00840000, 
  0x00880000, 
  0x008c0000, 
  0x00900000, 
  0x00940000, 
  0x00980000, 
  0x009c0000, 
  0x00a00000, 
  0x00a40000, 
  0x00a80000, 
  0x00ac0000, 
  0x00b00000, 
  0x00b40000, 
  0x00b80000, 
  0x00bc0000, 
  0x00c00000, 
  0x00c40000, 
  0x00c80000, 
  0x00cc0000, 
  0x00d00000, 
  0x00d40000, 
  0x00d80000, 
  0x00dc0000, 
  0x00e00000, 
  0x00e40000, 
  0x00e80000, 
  0x00ec0000, 
  0x00f00000, 
  0x00f40000, 
  0x00f80000, 
  0x00fc0000, 
  0x01000000, 
  0x01040000, 
  0x01080000, 
  0x010c0000, 
  0x01100000, 
  0x01140000, 
  0x01180000, 
  0x011c0000, 
  0x01200000, 
  0x01240000, 
  0x01280000, 
  0x012c0000, 
  0x01300000, 
  0x01340000, 
  0x01380000, 
  0x013c0000, 
  0x01400000, 
  0x01440000, 
  0x01480000, 
  0x014c0000, 
  0x01500000, 
  0x01540000, 
  0x01580000, 
  0x015c0000, 
  0x01600000, 
  0x01640000, 
  0x01680000, 
  0x016c0000, 
  0x01700000, 
  0x01740000, 
  0x01780000, 
  0x017c0000, 
  0x01800000, 
  0x01840000, 
  0x01880000, 
  0x018c0000, 
  0x01900000, 
  0x01940000, 
  0x01980000, 
  0x019c0000, 
  0x01a00000, 
  0x01a40000, 
  0x01a80000, 
  0x01ac0000, 
  0x01b00000, 
  0x01b40000, 
  0x01b80000, 
  0x01bc0000, 
  0x01c00000, 
  0x01c40000, 
  0x01c80000, 
  0x01cc0000, 
  0x01d00000, 
  0x01d40000, 
  0x01d80000, 
  0x01dc0000, 
  0x01e00000, 
  0x01e40000, 
  0x01e80000, 
  0x01ec0000, 
  0x01f00000, 
  0x01f40000, 
  0x01f80000, 
  0x01fc0000, 
  0x02000000 /* guard sector */
};

static FlashAlgorithm intelFlashAlgorithm = {
   intelFlashReset,
   intelFlashProgramWord,
   intelFlashProgramBlock,
   intelFlashEraseChip,
   intelFlashEraseSector,
   intelFlashEraseRange,
   intelFlashProtectRange
};

static FlashDescriptor flashDescriptor_28F128J3A = {
   "28F128J3A", 
   bothbanks(0x89), bothbanks(0x18), 
   &intelFlashAlgorithm,
   sizeof(flashSectors_28F128J3A)/sizeof(dword) - 1, flashSectors_28F128J3A,
   { "bootldr",   0x00000000, 0x00040000, LFR_PATCH_BOOTLDR },
   { "root",    0x00040000, 0x0, LFR_EXPAND|LFR_JFFS2 },
};

//PUBLIC

void initFlash(void)
{
	volatile unsigned long lpvReg;
	volatile unsigned long lpv;
	volatile unsigned long lpvIReg;
	BOOL bRet;
	unsigned long value;
	volatile unsigned long lpv2;
	BOOL bRet2;

	// check out the memory organization
	lpvReg = (volatile unsigned long) VirtualAlloc(0,1, MEM_RESERVE,PAGE_READWRITE);
	if ((bRet = VirtualCopy((void *) lpvReg, (void *) (SA1100_DRAM_CONFIGURATION_BASE/256),
			    1, PAGE_READWRITE | PAGE_NOCACHE|PAGE_PHYSICAL)) != TRUE)
		DisplayError(TEXT(__FILE__), __LINE__);

	value = *((volatile unsigned long *) (lpvReg + SA1100_MSC0));
	if (value & MSC_RBW16)
		bt_flash_organization = BT_FLASH_ORGANIZATION_1x16;
	if ((bRet = VirtualFree((void *)lpvReg,0,MEM_RELEASE)) != TRUE)
		DisplayError(TEXT(__FILE__), __LINE__);

	// set up the flash base
	lpv = (unsigned long) VirtualAlloc(0,BOOTLDR_SIZE, MEM_RESERVE,PAGE_READWRITE);	

	if ((bRet = VirtualCopy((void *) lpv, (void *) (0x000000),
			   BOOTLDR_SIZE, PAGE_READWRITE | PAGE_NOCACHE|PAGE_PHYSICAL)) != TRUE)
		DisplayError(TEXT(__FILE__), __LINE__);
	flashword = (volatile unsigned long *)lpv;	

	// set up the interrupt register base
	lpvIReg = (unsigned long) VirtualAlloc(0,4*K_1, MEM_RESERVE,PAGE_READWRITE);	

	if ((bRet = VirtualCopy((void *) lpvIReg, (void *) (SA1100_ICIP/256),
			   4*K_1, PAGE_READWRITE | PAGE_NOCACHE|PAGE_PHYSICAL)) != TRUE)
		DisplayError(TEXT(__FILE__), __LINE__);
	
	interrupt_flash_reggie = (volatile unsigned long *)(lpvIReg + 
		(SA1100_ICMR & 0xffff));	
	if (bRet == TRUE)
		old_ICMR_value = (unsigned long) *interrupt_flash_reggie;

	//set up the vpp base
	lpv2 = (volatile unsigned long) VirtualAlloc(0,GPIO_SIZE, MEM_RESERVE,PAGE_READWRITE);
	if ((bRet2 = VirtualCopy((void *) lpv2, (void *) (0x49000000/256),
		GPIO_SIZE, PAGE_READWRITE | PAGE_NOCACHE|PAGE_PHYSICAL)) != TRUE){
		DisplayError(TEXT(__FILE__), __LINE__);
		return;
	}
	
	gpio_base= (volatile unsigned short *)lpv2;	

	mach_type = discover_machine_type((volatile unsigned short *) gpio_base);
	
	if (mach_type == MACH_TYPE_H3800)
		flashvpp= (volatile unsigned short *)(lpv2 + VPP_REG_3800);	
	else
		flashvpp= (volatile unsigned short *)lpv2;	


	// set up the other flash globals
	switch (bt_flash_organization) {
	case BT_FLASH_ORGANIZATION_2x16:
		nsectors = flashDescriptor_28F128J3A.nsectors;
		flashSectors = flashDescriptor_28F128J3A.sectors;
		bootloader_sector_end = 1; 
	    break;
	    
	case BT_FLASH_ORGANIZATION_1x16:
		nsectors = flashDescriptor_28F128J3A_1x16.nsectors;
		flashSectors = flashDescriptor_28F128J3A_1x16.sectors;
		bootloader_sector_end = 2;
	    break;
	    
	default:
	    putLabeledWord("flash_addr_shift(): "
			   "bad bt_flash_organization=",
			   bt_flash_organization);
	    break;
	}

	int dc = GetDeviceCode((unsigned long *) flashword);

	switch (dc){
	case DEVICE_CODE_32:
		flash_size = 4*M_1;
		break;
	case DEVICE_CODE_64:
		flash_size = 8*M_1;
		break;
	case DEVICE_CODE_128:
		flash_size = 16*M_1;
		break;
	}
	// if we are interleaved, we double it
	if (bt_flash_organization == BT_FLASH_ORGANIZATION_2x16)
		flash_size *=2;

	//flash_size = flashSectors[nsectors];
	flash_address_mask = flash_size-1;
}

void SaveFlash(HWND hWnd)
{
	TCHAR szFilename[128];
	TCHAR szText[128];

	if (MessageBox(hWnd, _T("This will save the contents of your bootloader segment to the file \\My Documents\\saved_bootldr.bin. Do you wish to proceed?"), 
	       _T("BootBlaster"), 
		   MB_YESNO) != IDYES)
		return ;

	wsprintf (szFilename, TEXT ("\\My Documents\\saved_bootldr.bin"));
	if (save_flash(hWnd,flashword,256*K_1,szFilename) == TRUE)
	{
		wsprintf (szText, TEXT ("Bootldr segment successfully saved to \\My Documents\\saved_bootldr.bin.  Please copy it to your desktop machine for safekeeping."));
		MessageBox (hWnd, szText, _T("BootBlaster"), MB_OK);
	}
	
}

void SaveFlashGZ(HWND hWnd)
{
	TCHAR szFilename[128];
	TCHAR szText[128];
	int error;
	


	if (MessageBox(hWnd, _T("This will save the contents of your bootloader segment to the file \\My Documents\\saved_bootldr.gz. Do you wish to proceed?"), 
	       _T("BootBlaster"), 
		   MB_YESNO) != IDYES)
		return ;

	wsprintf (szFilename, TEXT ("\\My Documents\\saved_bootldr.gz"));

	error = 0;
	if (save_flash_gz(hWnd,flashword,BOOTLDR_SIZE,szFilename) != TRUE)
		error = 1;
	
	if (!error){
		wsprintf (szText, TEXT ("Bootloader successfully saved to %s.  Please copy it to your desktop machine for safekeeping."),szFilename);
		MessageBox (hWnd, szText, _T("BootBlaster"), MB_OK);
	}
	else{
		wsprintf (szText, TEXT ("Bootloader failed to saveto %s."),szFilename);
		MessageBox (hWnd, szText, _T("BootBlaster") _T("Error"), MB_OK);
	}

}

void SaveFlashWince(HWND hWnd)
{
	TCHAR szFilename[128];
	TCHAR szAssetFilename[128];
	TCHAR szText[512];
	volatile unsigned long lpv;
	BOOL bRet;
	unsigned long count = 0;
	unsigned long flash_end = flash_size;
	int error;
	gzFile			file = 0x0;
	DWORD			dwCount = 0;
	DWORD			dwOpcode;
	DWORD			dwBytesWritten;
	char	ansiFName[128];
	int ret;
	HANDLE			hHexFile = 0;
	HWND hwndPB = NULL;
	HWND hwndSt= NULL;
	unsigned long step;
	double factor = 100.0;

	wsprintf (szFilename, TEXT ("\\My Documents\\wince_image.gz"));
	if (mach_type == MACH_TYPE_H3800){
		wsprintf (szAssetFilename, TEXT ("\\My Documents\\asset_image.gz"));
		wsprintf (szText, TEXT ("This will save the %d Mb wince segment to the file %s and the asset partition to the file %s. Do you wish to proceed?"),
			flash_size/M_1,szFilename,szAssetFilename);
		flash_end = flash_size - ASSET_PARTITION_SIZE; // last partition is assets

	}
	else {
	wsprintf (szText, TEXT ("This will save the %d Mb wince segment to the file %s. Do you wish to proceed?"),
			flash_size/M_1,szFilename);
	}

	if (MessageBox(hWnd,szText, 
	       _T("BootBlaster"), 
		   MB_YESNO) != IDYES)
		return ;
	
	hHexFile = CreateFile(szFilename, GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hHexFile != INVALID_HANDLE_VALUE){
		// the file exists
		CloseHandle(hHexFile);
		wsprintf (szText, TEXT ("The file %s already exists.  Do you want to overwrite it?"),szFilename);
		if (MessageBox (hWnd, szText, _T("BootBlaster"), MB_YESNO) != IDYES)
			return ;
	}
	UpdateWindow(hWnd);
	Sleep(100);
	ret = wcstombs(ansiFName,szFilename,lstrlen(szFilename)+1);
	file = gzopen(ansiFName,"wb1");	
	if (file == NULL) {
		wsprintf (szText, TEXT ("Failed to open file %s"),szFilename);
		MessageBox (hWnd, szText, _T("BootBlaster" _T("Error")), MB_OK);
		return ;
	}

	if (bt_flash_organization == BT_FLASH_ORGANIZATION_1x16)
		factor /=2.0;
#define SAVE_CHUNK_SIZE (128*K_1)
	hwndPB = MakeProgressBar(hWnd);
	//ret = SendMessage(hwndPB, PBM_SETRANGE , (WPARAM) 0, MAKELPARAM(0, 65000));
	Sleep(100);
	// wince starts at 0x40000
	count = 256*K_1/SAVE_CHUNK_SIZE;
	error = 0;

	while ((count * SAVE_CHUNK_SIZE < flash_end) && !error){
		lpv = (unsigned long) VirtualAlloc(0,SAVE_CHUNK_SIZE, MEM_RESERVE,PAGE_READONLY);	
		if ((bRet = VirtualCopy((void *) lpv, (void *) ((0x000000 + (count * SAVE_CHUNK_SIZE))/256),
			SAVE_CHUNK_SIZE, PAGE_READONLY | PAGE_PHYSICAL )) != TRUE){
			DisplayError(TEXT(__FILE__), __LINE__);
			error=1;
		}
		if (!error){
			unsigned long *p = (unsigned long *)lpv;
			for (dwCount = 0;(dwCount * sizeof(long)) < SAVE_CHUNK_SIZE ;dwCount++)
			{
				dwOpcode = p[dwCount];
				dwBytesWritten = gzwrite(file,(void *) &dwOpcode,sizeof(dwOpcode));
				if (dwBytesWritten != sizeof(dwOpcode))
				{
					wsprintf (szText, TEXT ("Bad write #%d to file %s  bytesWritten=%d"),
						dwCount,szFilename,dwBytesWritten);
					MessageBox (hWnd, szText, _T("BootBlaster" _T("Error")), MB_OK);
					error = 1;
				}

		}
	}

		bRet = VirtualFree((void *)lpv,0,MEM_RELEASE);
		count++;

		//step = (unsigned long)(((double)count*256*K_1/flash_size)*65000.0);
		step = (unsigned long)(((double)count*256*K_1/(double)flash_size)*factor);
		// increment the progress bar
		ret = SendMessage(hwndPB, PBM_SETPOS, (WPARAM) step, 0);

#if 0
		if (!(count%2))
			ret = SendMessage(hwndPB, PBM_SETPOS, (WPARAM) step, 0);
		else
			ret = SendMessage(hwndPB, PBM_SETPOS, (WPARAM) 65000, 0);
#endif
		Sleep(10);
	}


	gzclose(file);

	if (mach_type == MACH_TYPE_H3800){ // save the assets too
		lpv = (unsigned long) VirtualAlloc(0,ASSET_PARTITION_SIZE, MEM_RESERVE,PAGE_READONLY);	
		if ((bRet = VirtualCopy((void *) lpv, (void *) ((flash_end)/256),
			ASSET_PARTITION_SIZE, PAGE_READONLY | PAGE_PHYSICAL )) != TRUE){
			DisplayError(TEXT(__FILE__), __LINE__);
			error=1;
		}
		if (save_flash_gz(hWnd,(volatile unsigned long *)lpv,
			ASSET_PARTITION_SIZE,szAssetFilename) != TRUE){
			wsprintf (szText, TEXT ("Bootloader failed to save to %s."),szAssetFilename);
			MessageBox (hWnd, szText, _T("BootBlaster") _T("Error"), MB_OK);
		}
		bRet = VirtualFree((void *)lpv,0,MEM_RELEASE);
	}
	

	if (hwndPB)
		DestroyWindow(hwndPB);


	count--;
	if (!error){
		if (mach_type == MACH_TYPE_H3800)
			wsprintf (szText, 
			  TEXT ("Wince successfully saved to %s.  Assets saved to %s. Please copy them to your desktop machine for safekeeping."),
			  szFilename,szAssetFilename);
		else
			wsprintf (szText, 
			  TEXT ("Wince successfully saved to %s.  Please copy it to your desktop machine for safekeeping."),
			  szFilename);
		MessageBox (hWnd, szText, _T("BootBlaster"), MB_OK);
	}
	else{
		wsprintf (szText, TEXT ("Wince failed to save to %s.  Error on segment %d"),szFilename,count);
		MessageBox (hWnd, szText, _T("BootBlaster"), MB_OK);
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
		if (MessageBox (hWnd, szText, _T("BootBlaster"), MB_YESNO) != IDYES)
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

static BOOL save_flash_gz(HWND hWnd,volatile unsigned long *p,unsigned long size,TCHAR *fName)
{
	HANDLE			hHexFile = 0;
	gzFile			file = 0x0;
	DWORD			dwCount = 0;
	DWORD			dwOpcode;
	DWORD			dwBytesWritten;
	TCHAR szText[128];
	char	ansiFName[128];
	int ret;

	// only open the file once
	
	hHexFile = CreateFile(fName, GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hHexFile != INVALID_HANDLE_VALUE){
		// the file exists
		CloseHandle(hHexFile);
		wsprintf (szText, TEXT ("The file %s already exists.  Do you want to overwrite it?"),fName);
		if (MessageBox (hWnd, szText, _T("BootBlaster"), MB_YESNO) != IDYES)
			return FALSE;
	}
	
	
	ret = wcstombs(ansiFName,fName,lstrlen(fName)+1);
	file = gzopen(ansiFName,"wb");	
	if (file == NULL) {
		wsprintf (szText, TEXT ("Failed to open file %s"),fName);
		MessageBox (hWnd, szText, _T("BootBlaster" _T("Error")), MB_OK);
		return FALSE;
	}
	
	for (dwCount = 0;(dwCount * sizeof(long)) < size;dwCount++)
	{
		dwOpcode = p[dwCount];
		dwBytesWritten = gzwrite(file,(void *) &dwOpcode,sizeof(dwOpcode));
		if (dwBytesWritten != sizeof(dwOpcode))
		{
			wsprintf (szText, TEXT ("Bad write #%d to file %s  bytesWritten=%d"),
				dwCount,fName,dwBytesWritten);
			MessageBox (hWnd, szText, _T("BootBlaster" _T("Error")), MB_OK);
			gzclose(file);
			return FALSE;
		}
	}
	gzclose(file);
	return TRUE;
}

void ShowFlash(HWND hWnd)
{
	TCHAR szText[128];
	wsprintf (szText, TEXT ("Flash word 0 ->0x%x ") ,*((unsigned long *)flashword));
    MessageBox (hWnd, szText, _T("BootBlaster"), MB_OK);
}

void VerifyFlash(HWND hWnd)
{
	TCHAR szText[1024];
	unsigned long size;
	unsigned long version;
	size = guessSize();

	// test to show machine type	
	//wsprintf (szText, TEXT ("mach_type set to %d = 0x%x ."),mach_type,mach_type);
	//MessageBox (hWnd, szText, _T("BootBlaster"), MB_OK);
	

	if (isValidOHHImage((unsigned long)flashword,size)){
		version = *(flashword+9);
		wsprintf (szText, TEXT ("You appear to have a\nvalid OHH Bootloader %d.%d.%d in flash.  To access this Bootloader on the serial port, hold the joypad center and reset."),
			(version& 0xff0000)>>16,(version& 0x00ff00)>>8,(version& 0x0000ff));
		MessageBox (hWnd, szText, _T("BootBlaster"), MB_OK);
		return;
	}

	if (isValidParrotImage((unsigned long)flashword,0x40000)){
		wsprintf (szText, TEXT ("You appear to have a\nvalid Parrot Bootloader in flash."));
		MessageBox (hWnd, szText, _T("BootBlaster"), MB_OK);
		return;
	}

	if (isErased((unsigned long)flashword,0x40000)){
		wsprintf (szText, TEXT ("You appear to have an\nerased flash sector."));
		MessageBox (hWnd, szText, _T("BootBlaster"), MB_OK);
		return;
	}
	wsprintf (szText, TEXT ("Your flash is in an unrecognized state."));
	MessageBox (hWnd, szText, _T("BootBlaster"), MB_OK);
		

}

BOOL LoadImage(HWND hWnd)
{
	TCHAR szFileName[MAX_PATH];
	TCHAR szText[128];
#if 0
	if (hasProgrammed == TRUE){
		wsprintf (szText, TEXT ("Already Programmed once.\nTo reprogram exit and try again."));
		MessageBox (hWnd, szText, _T("BootBlaster"), MB_OK);
		return FALSE;
	}
#endif
	if (MessageBox(hWnd, _T("This will overwrite your bootloader partition.  The operation takes about 15 seconds.  YOU MUST NOT RESET YOUR IPAQ DURING THIS OPERATION!  BE PATIENT! Are you sure you wish to proceed?"), 
	       _T("BootBlaster") _T(" Warning"), 
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
	else if (!lstrcmp(suffix,_T(".gz"))){		
		if (!(pFile = LoadFileGZ(szFileName,&fileSize))){
			MessageBox(hWnd, _T("Could not open file \r\n"),
		       _T(APP_NAME_STR) _T(" Error"), MB_OK);
		
			return FALSE;
		}
	}
	else{
		wsprintf (szText, TEXT ("%s is an unsupported file type, try a .gz or .bin file, please!") ,szFileName);
		MessageBox (hWnd, szText, _T("BootBlaster") _T(" Error"), MB_OK);
		return FALSE;
	}

	if (!isValidBootloader(pFile,fileSize)){
	    TCHAR szText[128];
		wsprintf (szText, TEXT ("%s is not a valid bootloader image") ,szFileName);
		MessageBox (hWnd, szText, _T("BootBlaster") _T(" Error"), MB_OK);
	    return FALSE;
	}
	wsprintf (szText, TEXT ("You have chosen to flash the file: %s.  Is this correct?"),szFileName);
	if (MessageBox(hWnd,szText, 
	       _T("BootBlaster"), 
		   MB_YESNO) != IDYES)
		return FALSE ;
	
	return TRUE;
}

void TestFlash(HWND hWnd){
	int i;
	int num_tests = 100;
	volatile unsigned long val;
	HWND hwndPB = NULL;
	hwndPB = MakeProgressBar(hWnd);
	volatile unsigned long p = (volatile unsigned long)(flashword);
	TCHAR szText[128];

	for (i=0; i < num_tests; i++){
		interrupts_off();
		vpp_on();
		Delay(1000);
		// first clear the lock bits this is set for 3100
		*(unsigned long *)(p+0x20000) = 0x60;
		*(unsigned long *)(p+0x20000) = 0xd0;
		// wait for completion
		val = *(unsigned long *)(p+0x20000);
		while ((val & 0x80) == 0x0)
			val = *(unsigned long *)(p+0x20000);
		*(unsigned long *)(p+0x20000) = 0xff;
		Delay(1000);
		*(unsigned long *)(p+0x20000) = 0x20;
		*(unsigned long *)(p+0x20000) = 0xd0;
		// wait for completion
		val = *(unsigned long *)(p+0x20000);
		while ((val & 0x80) == 0x0)
			val = *(unsigned long *)(p+0x20000);
		*(unsigned long *)(p+0x20000) = 0xff;
		Delay(1000);

		interrupts_on();
		//EraseFlash(hWnd);  //This hangs
		SendMessage(hwndPB, PBM_SETPOS, (WPARAM) i, 0);
		if (val & 0x3f){
			clear_status();
			wsprintf (szText, TEXT ("val returns 0x%x") ,val);
			MessageBox (hWnd, szText, _T("BootBlaster") _T(" Test"), MB_OK);
		}
		Sleep(10);
	}


	if (hwndPB)
		DestroyWindow(hwndPB);
}


BOOL ProgramFlash(HWND hWnd){

	HWND hwndPB = NULL;
	int ret;
	BOOL bRes;
	TCHAR szText[128];	
	hwndPB = MakeProgressBar(hWnd);
	volatile unsigned long val;
	volatile unsigned long p = (volatile unsigned long)(flashword);



	interrupts_off();	
	vpp_on();
	ret = program_flash(pFile,fileSize,hwndPB);
	intelFlashReset();
	intelFlashReset();
	Delay(1000);
	// first clear the lock bits this clears ALL lock bits
	*(unsigned long *)(p) = bothbanks(0x60);
	*(unsigned long *)(p) = bothbanks(0xd0);
	// wait for completion
	val = *(unsigned long *)(p);
	while ((val & bothbanks(0x80)) != bothbanks(0x80))
		val = *(unsigned long *)(p);
	// reset the flash
	*(unsigned long *)(p) = bothbanks(0xff);
	Delay(1000);
	if (val & bothbanks(0x3f)){
		clear_status();
		interrupts_on();	
		wsprintf (szText, TEXT ("program_flash cleanup unlock status ->0x%x") ,val);
		MessageBox (hWnd, szText, _T("BootBlaster") _T(" Error"), MB_OK);
	}
    interrupts_on();

	
	// now reprotect the bootloader sector(s)
	ret =protect_flash_sector(hWnd,(unsigned long)(flashword));
	if (ret != 0){
		wsprintf (szText, TEXT ("Bad return protecting the bootloader sector: %d") ,ret);
		MessageBox (hWnd, szText, _T("BootBlaster") _T(" Error"), MB_OK);
	}
	if (mach_type == MACH_TYPE_H3100){
		ret = protect_flash_sector(hWnd,(unsigned long)flashword + 0x20000);
		if (ret != 0){
			wsprintf (szText, TEXT ("Bad return protecting the bootloader sector 2: %d") ,ret);
			MessageBox (hWnd, szText, _T("BootBlaster") _T(" Error"), MB_OK);
		}
	}
	bRes = VirtualFree((void *)pFile,256*K_1,MEM_RELEASE);
	if (hwndPB)
		DestroyWindow(hwndPB);
	
	if (ret){
		DisplayLLErrorText(hWnd,gErrorText,TEXT(__FILE__),__LINE__);
		return FALSE;
	}
	else
		return TRUE;
}

BOOL EraseFlash(HWND hWnd){
    int ret;

	ret = erase_flash(hWnd);
    if (ret)
		DisplayLLErrorText(hWnd,gErrorText,TEXT(__FILE__),__LINE__);
    
    return TRUE;
}

void ProtectWince(HWND hWnd){
    int ret;

	ret = protect_wince(hWnd);
    if (ret)
		DisplayLLErrorText(hWnd,gErrorText,TEXT(__FILE__),__LINE__);
    
    
}

//PRIVATE
static void vpp_on(void)
{
	set_vpp(1);
}
static void vpp_off(void)
{
	set_vpp(0);
}

static void set_vpp(int onOff)
{
	if (onOff)
		*flashvpp = VPP_ON;
	else
		*flashvpp = VPP_OFF;
		
}
static void set_interrupts(int on)
{
	DWORD lpInBuf;   
	DWORD  bytesReturned;
	BOOL bRet;

	if (on){
		//lpInBuf = TRUE ; // on
		*interrupt_flash_reggie = old_ICMR_value;

	}
	else{
		old_ICMR_value = (unsigned long) *interrupt_flash_reggie;
		*interrupt_flash_reggie = 0x0;
		//lpInBuf = FALSE ; // off
	}
//	if ((bRet = KernelIoControl (IOCTL_HAL_INTR_ON_OFF,(void *) &lpInBuf, sizeof(DWORD),
//				NULL,0,&bytesReturned )) != TRUE)
//		DisplayError(TEXT(__FILE__), __LINE__);

}

static void interrupts_on(void)
{
	set_interrupts(1);
}

static void interrupts_off(void)
{
	set_interrupts(0);
	Delay(100000);// in case there are any pending flash things.
	intelFlashReset();
	
}


static int erase_flash(HWND hWnd)
{
	volatile unsigned long val;
	volatile unsigned long p = (volatile unsigned long)(flashword);
	TCHAR szText[128];


	interrupts_off();
	vpp_on();
	Delay(1000);
	// first clear the lock bits this clears ALL lock bits
	*(unsigned long *)(p) = bothbanks(0x60);
	*(unsigned long *)(p) = bothbanks(0xd0);
	// wait for completion
	val = *(unsigned long *)(p);
	while ((val & bothbanks(0x80)) != bothbanks(0x80))
		val = *(unsigned long *)(p);
	// reset the flash
	*(unsigned long *)(p) = bothbanks(0xff);
	Delay(1000);
	if (val & bothbanks(0x3f)){
		clear_status();
		interrupts_on();	
		wsprintf (szText, TEXT ("erase_flash unlock status ->0x%x") ,val);
		MessageBox (hWnd, szText, _T("BootBlaster") _T(" Error"), MB_OK);
		return -1;
	}
	if (bt_flash_organization == BT_FLASH_ORGANIZATION_1x16){
		*(unsigned long *)(p+0x20000) = bothbanks(0x20);
		*(unsigned long *)(p+0x20000) = bothbanks(0xd0);
		// wait for completion
		val = *(unsigned long *)(p+0x20000);
		while ((val & bothbanks(0x80)) != bothbanks(0x80))
			val = *(unsigned long *)(p+0x20000);
		*(unsigned long *)(p+0x20000) = bothbanks(0xff);
		Delay(1000);
		if (val & bothbanks(0x3f)){
			clear_status();
			interrupts_on();	
			wsprintf (szText, TEXT ("erase_flash erase 0x20000 block status ->0x%x") ,val);
			MessageBox (hWnd, szText, _T("BootBlaster") _T(" Error"), MB_OK);
			return -1;
		}
	}
	// everybody must unlock block 0
	// we do 0 last in case of an error. i.e. you'll survive with 0 
	// programmed and 0x20000 erased but not the reverse
	*(unsigned long *)(p+0x0000) = bothbanks(0x20);
	*(unsigned long *)(p+0x0000) = bothbanks(0xd0);
	// wait for completion
	val = *(unsigned long *)(p+0x0000);
	while ((val & bothbanks(0x80)) != bothbanks(0x80))
		val = *(unsigned long *)(p+0x0000);
	*(unsigned long *)(p+0x0000) = bothbanks(0xff);
	Delay(1000);
	if (val & bothbanks(0x3f)){
		clear_status();
		interrupts_on();	
		wsprintf (szText, TEXT ("erase_flash erase 0x0000 block status ->0x%x") ,val);
		MessageBox (hWnd, szText, _T("BootBlaster") _T(" Error"), MB_OK);
		return -1;
	}

	interrupts_on();
	Sleep(10);
	return 0;
}

// this will set the lock bits for all of wince, erase flash
// already did the unptotect.
static int protect_wince(HWND hWnd)
{
	unsigned long p;
	TCHAR szText[128];
	int i;
	volatile unsigned long lpv;
	BOOL bRet;
	int ret;
	HWND hwndPB = MakeProgressBar(hWnd);

#if 0
	// to debug this
	interrupts_off();
	vpp_on();
	Delay(1000);
	unsigned long val;
	// first clear the lock bits this clears ALL lock bits
	*flashword = bothbanks(0x60);
	*flashword = bothbanks(0xd0);
	// wait for completion
	val = *flashword;
	while ((val & bothbanks(0x80)) != bothbanks(0x80))
		val = *flashword;
	// reset the flash
	*flashword = bothbanks(0xff);
	Delay(1000);
	if (val & bothbanks(0x3f)){
		clear_status();
		interrupts_on();	
		wsprintf (szText, TEXT ("erase_flash unlock status ->0x%x") ,val);
		MessageBox (hWnd, szText, _T("BootBlaster") _T(" Error"), MB_OK);
		return -1;
	}

	interrupts_on();
	Sleep(10);
#endif

	for (i = bootloader_sector_end; (p = flashSectors[i]) < flash_size; i++){
	//for (i = 1; i < 4; i++){
	//for (i = (nsectors-1); i < (nsectors-2); i--){
		// map this region.
		//p = flashSectors[i];
		lpv = (unsigned long) VirtualAlloc(0,flashSectors[1], MEM_RESERVE,PAGE_READWRITE);	
		if ((bRet = VirtualCopy((void *) lpv, (void *) ((0x000000 + p)/256),
			flashSectors[1],  PAGE_READWRITE | PAGE_NOCACHE|PAGE_PHYSICAL)) != TRUE){
			DisplayError(TEXT(__FILE__), __LINE__);
			return -1;
		}
		if (hwndPB)
			SendMessage(hwndPB, PBM_SETPOS, (WPARAM) (100.0 * ((double) p / flash_size)), 0);
		ret =protect_flash_sector(hWnd,lpv);
		//this works
		//ret =protect_flash_sector(hWnd,(unsigned long) flashword);
		bRet = VirtualFree((void *)lpv,0,MEM_RELEASE);
		if (bRet != TRUE){
			DisplayError(TEXT(__FILE__), __LINE__);
			return -1;
		}
		if (ret)
			return -1;

	}

	if (hwndPB)
		DestroyWindow(hwndPB);
	return 0;
}

static int protect_flash_sector(HWND hWnd,unsigned long p)
{
	volatile unsigned long val;
	TCHAR szText[128];
	unsigned long ctr = 500000;
	interrupts_off();
	vpp_on();
	Delay(1000);
	*(unsigned long *)(p) = bothbanks(0x60);
	*(unsigned long *)(p) = bothbanks(0x01);
	// wait for completion
	val = *(unsigned long *)(p);
	while (((val & bothbanks(0x80)) != bothbanks(0x80)) && (ctr > 0)){
		val = *(unsigned long *)(p);
		ctr--;
	}
	Delay(500);	
	// reset the flash
	*(unsigned long *)(p) = bothbanks(0xff);
	Delay(1000);
	if (val & bothbanks(0x3f)){
		clear_status();
		interrupts_on();	
		wsprintf (szText, TEXT ("protect_wince_region lock status ->0x%x") ,val);
		MessageBox (hWnd, szText, _T("BootBlaster") _T(" Error"), MB_OK);
		return -1;
	}

	interrupts_on();
	Sleep(10);
	return 0;
}

	

volatile unsigned long timeMe;
static int program_flash(unsigned long p,unsigned long size,HWND hwndPB)
{
	int i;
	unsigned long remaining_bytes;

	i = 0;
	remaining_bytes = size;
	//remaining_bytes =0;
	while(remaining_bytes > 0) {
	    int bytes_programmed = 0;
#if 1
	    // nice idea, but it freezes...
	    if ((i % K_4) == 0) {
		unsigned long step;
		intelFlashReset();
		interrupts_on();
		step = (unsigned long)(100 - ((double)remaining_bytes/size)*100.0);
		// increment the progress bar
		SendMessage(hwndPB, PBM_SETPOS, (WPARAM) step, 0);
		Sleep(100);
		interrupts_off();
		vpp_on();
		Delay(10000);
	    }
#endif
	    if (bt_flash_organization == BT_FLASH_ORGANIZATION_2x16){
		if (intelFlashProgramWord(i, *(unsigned long *)(p+i)))
		    return -1;
		bytes_programmed = 4;
	    }
	    else{
		if (intelFlashProgramWord_1x16(i, *(unsigned long *)(p+i))) 
		    return -1;
		bytes_programmed = 4;
	    }
	    i += bytes_programmed;
	    remaining_bytes -= bytes_programmed;
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

static unsigned long LoadFileGZ(const TCHAR *fname,unsigned long *ulSize)
{
    unsigned long lpv;
    DWORD			dwOpcode;    
    DWORD			dwBytesRead;
    unsigned long *p;
	char	ansiFName[128];
	int ret;
	gzFile			file;

   	ret = wcstombs(ansiFName,fname,lstrlen(fname)+1);
	file = gzopen(ansiFName,"rb");	
	if (file == NULL) {
        return 0;
    }
	*ulSize = 0;
    if (!(lpv = (unsigned long) VirtualAlloc(0,256*K_1, MEM_COMMIT,PAGE_READWRITE)))
		return lpv;
    p = (unsigned long *) lpv;    
	while ((dwBytesRead = gzread(file,(void *) &dwOpcode,sizeof(dwOpcode))) > 0){
		if (dwBytesRead != sizeof(dwOpcode)){
			VirtualFree((void *)lpv,256*K_1,MEM_RELEASE);
		    return 0;
		}
		*p++ = dwOpcode;
		(*ulSize)+=dwBytesRead;
    }
    if (dwBytesRead == -1){ // errored out
		VirtualFree((void *)lpv,256*K_1,MEM_RELEASE);
		return 0;
	}
	gzclose(file);
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


/* sectorAddress must be a valid start of sector address. sectors
   must be erased before they can be programmed! */
static int intelFlashEraseSector (unsigned long sectorAddress)
{
   int i;
   long timeout = FLASH_TIMEOUT;
   unsigned long flashWordOffset = (sectorAddress&flash_address_mask);
   unsigned long flashContents;
   unsigned long status;
   unsigned long done_status;

   flashWordOffset >>= flash_addr_shift();
   
   for (i = 0; i < nsectors; i++) {
      if (flashSectors[i] == sectorAddress)
         break;
   }
   if (i >= nsectors) {
      putLabeledWord("eraseFlashSector: sectorAddress must be start of a sector! address=", sectorAddress);
      putLabeledWord("nsectors=", nsectors);
      wsprintf (gErrorText, TEXT ("intelFlashEraseSector: 0x%x is not the start address of a sector, try again...")
		,sectorAddress);
      return -1;
   }

   /* send flash the erase sector command */
   flash_write_cmd(0x55, 0x20); /*  flashword[0x55] = bothbanks(0x20); */
   flash_write_cmd(flashWordOffset, 0xd0); /*  flashword[flashWordOffset] = bothbanks(0xD0); */

   /* address doesn't matter */
   status = intelFlashWaitforStatus(0, &timeout);
   
   intelFlashClearStatus();

   /* need to read from divided address (>>2 or >>1) */
   done_status = flash_make_val(0x7f);
#if 0
   putLabeledWord("done_status=0x", done_status);
#endif

   flashContents = flash_read_array(sectorAddress);
   if ((timeout <= 0) || (status & done_status)) {
      putstr("eraseSector error\r\n");
      putLabeledWord("  sectorAddress=", sectorAddress);
      putLabeledWord("  flashContents=", flashContents);
      putLabeledWord("  status=", status);
      putLabeledWord("  timeout=", timeout);
      wsprintf (gErrorText, TEXT ("intelFlashEraseSector: error addr->0x%x,flashContents->0x%x,status->0x%x, timeout->%ld")
		,sectorAddress,flashContents,status,timeout);
      return(-1);
   }   

   return 0;
}


static int flash_addr_shift(void)
{
    switch (bt_flash_organization) {
	case BT_FLASH_ORGANIZATION_2x16:
	    return (2);
	    break;
	    
	case BT_FLASH_ORGANIZATION_1x16:
	    return (1);
	    break;
	    
	default:
	    putLabeledWord("flash_addr_shift(): "
			   "bad bt_flash_organization=",
			   bt_flash_organization);
	    return (0);
	    break;
    }

    return (-1);
}
static void flash_write_val(unsigned addr,int val)
{
    char*   dst;
    
    switch (bt_flash_organization) {
	case BT_FLASH_ORGANIZATION_2x16:
	    dst = (char*)flashword + (addr & ~0x3);
	    *((unsigned long*)dst) = bothbanks(val);
	    break;

	case BT_FLASH_ORGANIZATION_1x16:
	    dst = (char*)flashword + (addr & ~0x1);
	    *((unsigned short*)dst) = val;
	    break;

	default:
	    putLabeledWord("flash_write_val(): bad bt_flash_organization=",
			   bt_flash_organization);
	    break;
    }
}

static void flash_write_cmd(unsigned cmd_addr,int cmd)
{
    switch (bt_flash_organization) {
	case BT_FLASH_ORGANIZATION_2x16:
	    flashword[cmd_addr] = bothbanks(cmd);
	    break;

	case BT_FLASH_ORGANIZATION_1x16:
#if 0
	    putLabeledWord("fwc(1x16), Addr=0x",
			   (unsigned long)&((short*)flashword)[cmd_addr]);
#endif
	    ((short*)flashword)[cmd_addr] = cmd;
	    break;

	default:
	    putLabeledWord("flash_write_cmd(): bad bt_flash_organization=",
			   bt_flash_organization);
	    break;
    }
}

static int intelFlashClearStatus()
{
   /* address does not matter */
   flash_write_cmd(0x55, 0x50); /*  flashword[0x55] = bothbanks(0x50); */
	return 0;
}

   /* now wait for it to be programmed */
static int intelFlashWaitforStatus(
    unsigned long addr,
    long*	  timeoutp)
{
   long	timeout = *timeoutp;
   unsigned long    done_status;
   unsigned long    status = 0x0;

   done_status = flash_make_val(0x80);
   while ((status & done_status) != done_status ) {
      status = flash_read_val(addr); 
      timeout--;
   }
   *timeoutp = timeout;
   return (status);
}

static unsigned long
flash_make_val(
    unsigned long   inval)
{
    switch (bt_flash_organization) {
	case BT_FLASH_ORGANIZATION_2x16:
	    return (bothbanks(inval));
	    break;
	    
	case BT_FLASH_ORGANIZATION_1x16:
	    return (inval);
	    break;
	    
	default:
	    putLabeledWord("flash_make_val(): "
			   "bad bt_flash_organization=",
			   bt_flash_organization);
	    return (0);
	    break;
    }

    return (-1);
}

static unsigned long
flash_read_array(
    unsigned	addr)
{
    char*   dst;

    /* force read array mode */
    flash_write_cmd(0x55, 0xff);
    
    switch (bt_flash_organization) {
	case BT_FLASH_ORGANIZATION_2x16:
	    dst = (char*)flashword + (addr & ~0x3);
	    return (*((unsigned long*)dst));

	case BT_FLASH_ORGANIZATION_1x16:
	    dst = (char*)flashword + (addr & ~0x1);
#if 0
	    putLabeledWord("fra(1x16), addr=0x", (unsigned long)dst);
#endif
	    return (*((unsigned short*)dst));
	    break;

	default:
	    putLabeledWord("flash_read_array(): bad bt_flash_organization=",
			   bt_flash_organization);
	    return 0x0;
		break;
    }
}

static unsigned long flash_read_val(unsigned addr)
{
    unsigned long retval = 0;
    
    switch (bt_flash_organization) {
	case BT_FLASH_ORGANIZATION_2x16:
	    retval = flashword[addr];
	    break;

	case BT_FLASH_ORGANIZATION_1x16:
	    retval = ((unsigned short*)flashword)[addr];
	    break;

	default:
		putLabeledWord("flash_read_val(): bad bt_flash_organization=",
			   bt_flash_organization);
	    break;
    }

    return (retval);
}

static unsigned long queryFlashID(unsigned long flashWordAddress)
{
   unsigned long result;
   unsigned long flashWordOffset = (flashWordAddress&flash_address_mask);
   int algorithm = queryFlash(0x13) & 0xFF;

   switch (algorithm) {
   case 1:
     /* reset flash -- Intel */
     flash_write_cmd(0x55, 0xff); /* flashword[0x55] = bothbanks(0xFF); */
     /* put flash in read identifier codes mode */
     flash_write_cmd(0x555, 0x90); /*  flashword[0x555] = bothbanks(0x90); */
     break;

   case 2:
     /* reset flash -- AMD */
     flash_write_cmd(0x55, 0xf0); /*  flashword[0x55] = bothbanks(0xF0); */
     /* put flash in query mode */
     flash_write_cmd(0x555, 0xaa); /*  flashword[0x555] = bothbanks(0xAA); */
     flash_write_cmd(0x2aa, 0x55); /*  flashword[0x2AA] = bothbanks(0x55); */
     flash_write_cmd(0x555, 0x90); /*  flashword[0x555] = bothbanks(0x90); */
     break;
   }

   /* read autoselect word */
   result = flash_read_val(flashWordOffset); /* flashword[flashWordOffset]; */

   switch (algorithm) {
   case 1:
     /* reset flash -- Intel */
     flash_write_cmd(0x55, 0xff); /*  flashword[0x55] = bothbanks(0xFF); */
     break;
   case 2:
     /* reset flash -- AMD */
     flash_write_cmd(0x55, 0xf0); /*  flashword[0x55] = bothbanks(0xF0); */
     break;
   }
   return result;
}




static unsigned long queryFlash(unsigned long flashWordAddress)
{
  /* there's a reason why we don't shift flashWordOffset by 2
   * here... we want the values we pass to queryFlash match the values on pages 15-17 of the spec.
   */
   unsigned long result;
   unsigned long flashWordOffset = flashWordAddress&flash_address_mask;

   /* put flash in query mode */
   /* davep flashword[0x55] = bothbanks(0x98); */
   flash_write_cmd(0x55, 0x98);
   
   /*  davep result = flashword[flashWordOffset]; */
   result = flash_read_val(flashWordOffset);
   
   /* reset flash */
   flash_write_cmd(0x55, 0xff); /* flashword[0x55] = bothbanks(0xFF);*/
   return result;
}

static int intelFlashEraseRange(unsigned long startAddress, unsigned long len)
{
  unsigned long lastSectorAddress = flashSectors[0];
  unsigned long limitAddress = startAddress + len;
  unsigned long sectorAddress, i;

  startAddress &= flash_address_mask;
  if (limitAddress >= flashSectors[nsectors])
    limitAddress = flashSectors[nsectors];

  for (i=0;(int)i<nsectors;i++) {
     sectorAddress = flashSectors[i+1]; /* actually nsectors entries in this array -- last is fictitious guard sector */
    if ((lastSectorAddress <= startAddress) && (sectorAddress > startAddress)) {
      putLabeledWord("Erasing sector ",lastSectorAddress);
      if (intelFlashEraseSector(lastSectorAddress))
	return -1;
      len -= (sectorAddress - startAddress);
      startAddress = sectorAddress;
      if (startAddress >= limitAddress)
	break;
    }
    lastSectorAddress = sectorAddress;
  }
  return 0;
}

/* each flash chip has 32B block, 64B block for chip array */
static int intelFlashProgramBlock(unsigned long flashAddress, unsigned long *values, int nbytes)
{
   unsigned long flashWordOffset = (flashAddress&flash_address_mask) >> 2;
   unsigned long blockOffset = (flashWordOffset & 0xFFFFFFC0);
   int nwords = nbytes >> 2;
   int result = 0;
   long timeout = FLASH_TIMEOUT;
   unsigned long status = 0;
   int i;

   if (0) putLabeledWord("intelFlashProgramFlashBlock\r\n", flashAddress);

   /* send the "write to buffer" command */
   flashword[flashWordOffset] = bothbanks(0xE8); 
   /* read the extended status register */
   do {
      status = flashword[flashWordOffset];
      if (0) putLabeledWord("  XSR=", status);
   } while ((status & 0x00800080) != 0x00800080);

   /* write word count at block start address */
   flashword[blockOffset] = bothbanks(nwords-1); /* length minus one */

   /* send the data */
   for (i = 0; i < nwords; i++) {
      flashword[flashWordOffset+i] = values[i];
      if (0) putLabeledWord(" sr=", flashword[blockOffset]);
   }

   /* send the confirmation to program */
   flashword[blockOffset] = bothbanks(0xD0);

   /* now wait for it to be programmed */
   timeout = FLASH_TIMEOUT;
   while (timeout > 0) {
      status = flashword[blockOffset];
      if (0) putLabeledWord("  status=", status);
      if ((status & bothbanks(0x80)) == bothbanks(0x80))
         break;
      timeout--;
   }
   status = intelFlashReadStatus();
   if (0) putLabeledWord("final status=", status);
   intelFlashClearStatus();
   
   if ((timeout <= 0) || (status & bothbanks(0x7f))) {
      putstr("programFlashBlock error\r\n");
      putLabeledWord("  flashAddress=", flashAddress);
      putLabeledWord("  status=", status);
      return(-1);
   }
   return 0;
}

static int intelFlashReadStatus()
{
   /* address does not matter */
   flash_write_cmd(0x55, 0x70); /*  flashword[0x55] = bothbanks(0x70); */
   return flash_read_val(0x55);    /* flashword[0x55]; */
}

static int intelFlashProtectRange(unsigned long startAddress, unsigned long len, int protect)
{

  /*
    startAddress - the start of the address range to protect
    len          - the len in bytes to protect
    protect      - 1 protect blocks within the address range startAddress + len
    protect      - 0 unprotect all blocks
  */

  unsigned long currentSectorAddress;
  unsigned long limitAddress = startAddress + len;
  int result = 0;
  int status = 0;

  if ( protect == 0) {
    unsigned long timeout = FLASH_TIMEOUT;
    /* unprotects whole chip */

    /*
     * any address will do, but since we also use this to get
     * status after the operation, a BA is required.
     * 0 is easy to type
     */
    currentSectorAddress = 0;
    
    flashword[currentSectorAddress >> 2] = bothbanks(0x60);
    flashword[currentSectorAddress >> 2] = bothbanks(0xd0);
   /* now wait for it to be complete */
    while (timeout > 0) {
      status = flashword[currentSectorAddress >> 2];
      if ((status & bothbanks(0x80)) == bothbanks(0x80))
         break;
      timeout--;
    }
    status = intelFlashReadStatus();
    if (status != (int) bothbanks(0x80))
      putLabeledWord("status :", status );         
    result = status;
    intelFlashClearStatus();
    /* put back in read mode */
    flashword[currentSectorAddress >> 2] = bothbanks(0xff);
    putLabeledWord("Protect=", queryFlash((currentSectorAddress>>2)+2)); 
  }
  return result;
}
static int intelFlashReset ()
{
   /* send flash the reset command */
   /* address does not matter -- only the data */
   flash_write_cmd(0x55, 0xff); /*  flashword[0x55] = bothbanks(0xFF); */
   return 0;
}

/*
 * Programs value at flashAddress
 * Sectors must be erased before they can be programmed.
 */
static int intelFlashProgramWord(unsigned long flashAddress, unsigned long value)
{
   unsigned long flashWordOffset = (flashAddress&flash_address_mask) >> 2;
   long timeout = FLASH_TIMEOUT;
   unsigned long flashContents = flashword[flashWordOffset];
   unsigned long status = 0;

   /* see if we can program the value without erasing */
   if ((flashContents & value) != value) {
      putstr("the flash sector needs to be erased first!\r\n");
      putLabeledWord("  flashAddress=", flashAddress);
      putLabeledWord("  flashWordOffset=", flashWordOffset);
      putLabeledWord("  &flashword[flashWordOffset]=", 
		     (dword)&flashword[flashWordOffset]);
      putLabeledWord("  flashContents=", flashContents);    
      wsprintf (gErrorText, TEXT ("intelFlashProgramWord: error addr->0x%x,offset->0x%x,flashContents->0x%x")
		,flashAddress,flashWordOffset,flashContents);
      return -1;
   }
   
   /* send flash the program word command */
   flashword[0x55] = bothbanks(0x40);
   flashword[flashWordOffset] = value;
   /* now wait for it to be programmed */
   while (timeout > 0) {
      status = flashword[flashWordOffset];
      if ((status & bothbanks(0x80)) == bothbanks(0x80))
         break;
      timeout--;
   }
   intelFlashClearStatus();
   
   if ((timeout <= 0) || (status & bothbanks(0x7f))) {
      putstr("programFlashWord error\r\n");
      putLabeledWord("  flashAddress=", flashAddress);
      putLabeledWord("  value=", value);
      putLabeledWord("  flashContents=", flashContents);
      putLabeledWord("  status=", status);
      wsprintf (gErrorText, TEXT ("intelFlashProgramShort: error addr->0x%x,value->0x%x,flashContents->0x%x,status->0x%x")
		,flashAddress,value,flashContents,status);

      return(-1);
   }
   return 0;
}

static int intelFlashEraseChip ()
{
  
   long timeout = FLASH_TIMEOUT;
   unsigned long flashWordOffset = 0;
   
   
   putstr("intelFlashEraseChip unimplemented\r\n");

   return 0;
}

/*  this goes through a series of checks to make sure that we are 
	loading a valid bootloader
*/
static BOOL isValidBootloader(unsigned long p,unsigned long size)
{
	BOOL ret = TRUE;
	if ((!isValidOHHImage(p,size)) && (!isValidParrotImage(p,size)))
		ret=FALSE;

	return ret;
}
static BOOL isValidOHHImage(unsigned long p,unsigned long size)
{
	BOOL ret = TRUE;
	unsigned int bsd_sum;
	unsigned long start_addr;
	unsigned long boot_caps;

	if (size < 0x2C) //exit early
		return FALSE;
	
	// right bootloader??
	if (*((unsigned long *)(p+0x20)) != BOOTLDR_MAGIC)
		ret = FALSE;
	
	// right arch??
	if (*((unsigned long *)(p+0x2C)) != ARCHITECTURE_MAGIC)
		ret = FALSE;

	// linked at 0x0??
	start_addr = (unsigned long)(*(unsigned long *)(p+0x28));
	if (start_addr != 0x0)
		ret = FALSE;

		// mach_type matches caps??
	boot_caps = (unsigned long)(*(unsigned long *)(p+0x30));
	if ((mach_type == MACH_TYPE_H3800) && !(boot_caps & BOOTCAP_3800_SUPPORT))
		ret = FALSE;


	//BSD Sum == 0??
	bsd_sum = bsd_sum_memory( p, size);
	if (bsd_sum != 0)
		ret = FALSE;

	return ret;
}
static BOOL isValidParrotImage(unsigned long p,unsigned long size)
{
	BOOL ret = TRUE;
	unsigned long tmp;
	
	if (size < 0x1000)
		return FALSE;
	
	// we'll just check a couple of magic numbers
	if (*((unsigned long *)(p+0x0)) != PARROT_MAGIC_0)
		ret = FALSE;
	
	
#if 1
	tmp = *((unsigned long *)(p+0xFFC));
	if ((tmp != PARROT_MAGIC_FFC) &&
		(tmp != PARROT_MAGIC_FFC_ALT))
		ret = FALSE;
#endif
	if (*((unsigned long *)(p+0x1000)) != PARROT_MAGIC_1000)
		ret = FALSE;

	return ret;
}


static unsigned int bsd_sum_memory(unsigned long img_src, size_t img_size) 
{
  unsigned long checksum = 0;   /* The checksum mod 2^16. */
  unsigned char *pch;		/* Each character read. */
  size_t i;

  pch = (unsigned char *)img_src;
  for (i=1; i <= img_size; i++)
   {
      /* Do a right rotate */
      if (checksum & 01)
         checksum = (checksum >>1) + 0x8000;
      else
         checksum >>= 1;
      checksum += *pch;      /* add the value to the checksum */
      checksum &= 0xffff;  /* Keep it within bounds. */
      pch++;
    }
  return(checksum & 0xffff);
}

static BOOL isErased(unsigned long p,unsigned long size)
{
	unsigned long i;

	for (i=0; i < size; i+=sizeof(unsigned long))
		if (*((unsigned long *)(p+i)) != 0xFFFFFFFF)
			return FALSE;
	return TRUE;
}

//start at flashword+256K, count backwards and look for first non ffffffff word.
static unsigned long guessSize(void)
{
	unsigned long p = (unsigned long)flashword + 0x40000 - sizeof(unsigned long);
	
	for (;p >(unsigned long)flashword ; p -= sizeof(unsigned long))
		if (*((unsigned long *)p) != 0xFFFFFFFF)
			return (p + sizeof(unsigned long) - (unsigned long) flashword);
	return 0x0;
}
static int intelFlashProtectRange_1x16(unsigned long startAddress, unsigned long len, int protect)
{

  /*
    startAddress - the start of the address range to protect
    len          - the len in bytes to protect
    protect      - 1 protect blocks within the address range startAddress + len
    protect      - 0 unprotect all blocks
  */

  unsigned long currentSectorAddress;
  int status = 0;
  int done_status;
  long timeout = FLASH_TIMEOUT;

  /* unprotects whole chip */

  /*
   * any address will do, but since we also use this to get
   * status after the operation, a BA is required.
   * 0 is easy to type
   */
  currentSectorAddress = 0;
  
  flash_write_cmd(currentSectorAddress >> 1, 0x60);	
  flash_write_cmd(currentSectorAddress >> 1, 0xd0);
  status = intelFlashWaitforStatus(currentSectorAddress,&timeout);
  intelFlashClearStatus();
  done_status = flash_make_val(0x7f);
  if ((timeout <= 0) || (status & done_status)) {
	wsprintf (gErrorText, TEXT ("intelFlashProtectRange_1x16: error status->0x%x, timeout->%ld")
		,status,timeout);
    return(-1);
  }
  intelFlashReset();
  return 0;
}

#define	FLASH_BUFFER_SIZE  (32)

static int intelFlashProgramBlock_1x16(unsigned long flashAddress, unsigned long *long_values, int resid)
{
   unsigned long flashWordOffset = (flashAddress&flash_address_mask) >> 1;
   unsigned long blockOffset = (flashWordOffset & 0xFFFFFFF0);
   int nwords;
   int result = 0;
   long timeout = FLASH_TIMEOUT;
   unsigned long status = 0;
   int i;
   unsigned short* flashshort = (unsigned short*)flashword;
   unsigned short* values = (unsigned short*)long_values;
   int	word_size_shift = flash_addr_shift();
   int	max_words;

#if 0
   putLabeledWord("intelFlashProgramFlashBlock(1x16)\r\n", flashAddress);
#endif   

   nwords = resid >> word_size_shift;
   max_words = FLASH_BUFFER_SIZE >> word_size_shift;
   
   while (nwords) {

       int  num_to_program;

       if (nwords < max_words)
	   num_to_program = nwords;
       else
	   num_to_program = max_words;

       /* send the "write to buffer" command */
       flash_write_cmd(flashWordOffset, 0xe8);
       /* read the extended status register */
       do {
	   status = flash_read_val(flashWordOffset);
#if 0
	   putLabeledWord("  XSR=", status);
#endif
       } while ((status & 0x0080) != 0x0080);
#if 0
       putLabeledWord("final XSR=", status);
#endif

       /* write word count at block start address */
       flash_write_cmd(blockOffset, num_to_program-1);

       /* send the data */
       for (i = 0; i < num_to_program; i++) {
	   flashshort[flashWordOffset+i] = values[i];
	   if (0) putLabeledWord(" sr=", flashshort[blockOffset]);
       }

       /* send the confirmation to program */
       flash_write_cmd(blockOffset, 0xD0);/*flashshort[blockOffset] = 0xD0; */

       /* now wait for it to be programmed */
       timeout = FLASH_TIMEOUT;
       while (timeout > 0) {
	   status = flashshort[blockOffset];
	   if (0) putLabeledWord("  status=", status);
	   if ((status & 0x80) == 0x80)
	       break;
	   timeout--;
       }
       status = intelFlashReadStatus();
#if 0
       putLabeledWord("final status=", status);
#endif       
       intelFlashClearStatus();
   
       if ((timeout <= 0) || (status & 0x7f)) {
	   putstr("programFlashBlock error\r\n");
	   putLabeledWord("  flashAddress=", flashAddress);
	   putLabeledWord("  status=", status);
	   return(-1);
       }

       /*
	* prepare for next iteration.
	*/
       nwords -= num_to_program;
       values += num_to_program;
       flashWordOffset += num_to_program;
   }
   
   return 0;
}


static int intelFlashProgramShort(unsigned long flashAddress, unsigned short value)
{
   long timeout = FLASH_TIMEOUT;
   unsigned short flashContents = (unsigned short) flash_read_array(flashAddress);
   unsigned long status = 0;

   /* see if we can program the value without erasing */
   if ((flashContents & value) != value) {
      putstr("the flash sector needs to be erased first!\r\n");
      putLabeledWord("  flashAddress=", flashAddress);
      putLabeledWord("  flashContents=", flashContents);
      return -1;
   }
   
   /* send flash the program word command */
   flash_write_cmd(0x55, 0x40); /*  flashword[0x55] = bothbanks(0x40); */
   flash_write_val(flashAddress, value); /*  flashword[flashWordOffset] = value; */
   /* now wait for it to be programmed */
   status = intelFlashWaitforStatus(0, &timeout);
   intelFlashClearStatus();
   
   if ((timeout <= 0) || (status & 0x7f)) {
      putstr("programFlashWord error\r\n");
      putLabeledWord("  flashAddress=", flashAddress);
      putLabeledWord("  value=", value);
      putLabeledWord("  flashContents=", flashContents);
      putLabeledWord("  status=", status);
      wsprintf (gErrorText, TEXT ("intelFlashProgramShort: error addr->0x%x,value->0x%x,flashContents->0x%x,status->0x%x")
		,flashAddress,value,flashContents,status);
      return(-1);
   }
   return 0;
}
static int intelFlashProgramWord_1x16(unsigned long flashAddress, unsigned long value)
{
    int	status;

    status = intelFlashProgramShort(flashAddress, (unsigned short)(value & 0xffff));
    if (status == 0)
	status = intelFlashProgramShort(flashAddress+2, (unsigned short) ((value>>16)&0xffff));

    return (status);
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
	if (bt_flash_organization == BT_FLASH_ORGANIZATION_1x16)
		return w;
	else
		return ((((w)&0xFFFF) << 16)|((w)&0xFFFF));

}

static int GetDeviceCode(unsigned long *p){
	unsigned long * pCode;
	

	if (bt_flash_organization == BT_FLASH_ORGANIZATION_1x16)
		pCode = (unsigned long *)(p + sizeof(short));
	else
		pCode = (unsigned long *)(p + sizeof(long));
	int ret = 0;
	interrupts_off();
	p[0] = 0x90;
	ret = flash_read_val(1) & 0xff;
	p[0] = 0xff;
	Delay(1000);
	interrupts_on();
	return ret;
}

static void DisplayLLErrorText(HWND hWnd,TCHAR *pM,TCHAR *pF,UINT line)
{	
	TCHAR szText[1200];

	gSystemErrorHasOccured = TRUE;

	wsprintf(szText,TEXT("error: %s from %s:%d"),
		pM,pF,line);

	MessageBox (hWnd, szText, _T("BootBlaster Low Level Error"), MB_OK);
}

static void clear_status(void)
{
	volatile unsigned long p = (volatile unsigned long)(flashword);
	*(unsigned long *)(p) = bothbanks(0x50);

}


static int discover_machine_type(volatile unsigned short *p)
{
    //volatile unsigned short *p = (unsigned short *) BITSY_EGPIO;
    volatile unsigned short *p2 = (unsigned short *) flashword;
    volatile unsigned short tst;
    //unsigned short def_val = EGPIO_BITSY_LCD_ON | EGPIO_BITSY_RS232_ON | EGPIO_BITSY_LCD_PCI |
	//		EGPIO_BITSY_LCD_5V_ON | EGPIO_BITSY_LVDD_ON;
    //unsigned short def_val = 0x4cf0; // exp derived.
    unsigned short init_val;
	unsigned short def_val = 0xcdf0; // exp derived.
    
 
    // first check to see if we are a 3800
    // on the 3800 this is the gpio dir register so it is safe to set it
    // we will be able to read it back if we are a 3800 and not else
    // we are setting the serial port on bit so it's a safe op
    // for the 3(1,6,7)xx models too
	init_val = *p;
    *p = def_val;
    tst = *p2;
    tst = *p;
    
	//return (int) tst;
    if ((tst == def_val)){
		*p = init_val; //set it back
		return MACH_TYPE_H3800;
	}
    else{
		if (bt_flash_organization == BT_FLASH_ORGANIZATION_1x16){
			return MACH_TYPE_H3100;
		}
		else{
			return MACH_TYPE_H3600;
		}

    }

}


//stubs
static void putLabeledWord(char *p,unsigned long v)
{}
static void putstr(char *p)
{}

