
#if !defined(AFX_FLASHUTILS_H__BDCBB719_CD62_11D5_A244_00508BA08C36__INCLUDED_)
#define AFX_FLASHUTILS_H__BDCBB719_CD62_11D5_A244_00508BA08C36__INCLUDED_

/* two 16bit parts in parallel (original iPAQ) */
#define	BT_FLASH_ORGANIZATION_2x16	(1)

/* one single 16bit part (B&W iPAQ) */
#define	BT_FLASH_ORGANIZATION_1x16	(2)

/*
 * bootldr capabilities
 * Stored at offset 0x30 into the boot flash.
 */

#define BOOTCAP_WAKEUP	(1<<0)
#define BOOTCAP_PARTITIONS (1<<1) /* partition table stored in params sector */
#define BOOTCAP_PARAMS_AFTER_BOOTLDR (1<<2) /* params sector right after bootldr sector(s), else in last sector */
#define BOOTCAP_3800_SUPPORT (1<<3) /* 3800 ipaqs require special support this bit is read by BootBlaster and eventually by load bootldr*/


#define BOOTLDR_MAGIC      0x646c7462        /* btld: marks a valid bootldr image */
#define ARM_PLATFORM   1
#define ARM_SA110_CPU   1
#define ARM_SA1100_CPU  2
#define ARM_SA1110_CPU  3
#define ARM_SA1111_CPU  4
#define MIPS_PLATFORM  2
#define SKIFF_MACH      1
#define ITSY_MACH       2 
#define BITSY_MACH      3
#define ASSABET_MACH	4
#define NEPONSET_MACH	5
#define JORNADA720_MACH	6
/* must match definition in linux/include/asm/mach-types.h */
#define MACH_TYPE_CATS 6
#define MACH_TYPE_PERSONAL_SERVER 17
#define MACH_TYPE_SA1100 16
#define MACH_TYPE_H3600                22
#define MACH_TYPE_H3100                136
#define MACH_TYPE_H3800                137
#define MACH_TYPE_H3600_ASCII          "22"
#define MACH_TYPE_H3100_ASCII          "136"
#define MACH_TYPE_H3800_ASCII          "137"
#define MACH_TYPE_ASSABET 25
#define MACH_TYPE_JORNADA720 48
#define MACH_TYPE_OMNIMETER 49



#define BITSY_ARCH ((ARM_PLATFORM << 24) | (ARM_SA1110_CPU << 16) | BITSY_MACH)
#define ARCHITECTURE_MAGIC BITSY_ARCH

#define PARROT_MAGIC_0 0xEA0003FE
#define PARROT_MAGIC_FFC 0x0
#define PARROT_MAGIC_FFC_ALT 0xFFFFFFFF
#define PARROT_MAGIC_1000 0xE321F0D3
#define VPP_REG_3800 0x1f00


typedef unsigned int   dword;
#define dim(x) (sizeof(x) / sizeof(x[0]))
#define FLASH_TIMEOUT 20000000
//#define FLASH_TIMEOUT 2
//#define bothbanks(w_) ((((w_)&0xFFFF) << 16)|((w_)&0xFFFF))

#define K_1 1024
#define K_4 (4 * K_1)
#define K_16 (16 * K_1)
#define M_1 (K_1 * K_1)
#define VPP_ON (0xf1e1)
#define VPP_OFF (0xf1e0)

#define DEVICE_CODE_32	0x16
#define DEVICE_CODE_64	0x17
#define DEVICE_CODE_128	0x18

#define BOOTLDR_SIZE (256*K_1)
#define ASSET_PARTITION_SIZE (256*K_1)
#define GPIO_SIZE (8*K_1)

static unsigned long bothbanks(unsigned long w);

// private protos
// Prototype of function that we need, but isn't in user level header files.
extern "C" BOOL VirtualCopy(LPVOID lpvDestMem, LPVOID lpvSrcMem, 
							DWORD dwSizeInBytes, DWORD dwProtectFlag);
extern "C" BOOL KernelIoControl(DWORD dwIoControlCode, LPVOID lpInBuf, DWORD nInBufSize, LPVOID lpOutBuf, DWORD nOutBufSize, LPDWORD lpBytesReturned);
static unsigned long LoadFile(const TCHAR *fname,unsigned long *ulSize);
static INT FindFile (HWND hWnd, LPTSTR szFileName, INT nMax);
static int intelFlashReset(void);
static int intelFlashProgramBlock(unsigned long flashAddress, unsigned long *values, int nbytes);
static int intelFlashProgramBlock_1x16(unsigned long flashAddress, unsigned long *values, int nbytes);
static int intelFlashProgramWord(unsigned long flashAddress, unsigned long value);
static int intelFlashProgramWord_1x16(unsigned long flashAddress, unsigned long value);
static int intelFlashEraseChip(void);
static int intelFlashEraseSector(unsigned long sectorAddress);
static int intelFlashEraseRange(unsigned long start, unsigned long len);
static int intelFlashProtectRange(unsigned long start, unsigned long len, int protect);
static int intelFlashProtectRange_1x16(unsigned long start, unsigned long len, int protect);
static void vpp_on(void);
static void vpp_off(void);
static void set_vpp(int onOff);
static void set_interrupts(int onOff);
static void interrupts_on(void);
static void interrupts_off(void);
static void flash_unprotect(void);
static int erase_flash(HWND hWnd);
static int protect_wince(HWND hWnd);
static int protect_flash_sector(HWND hWnd,unsigned long p);
static int intelFlashEraseSector (unsigned long sectorAddress);
static int flash_addr_shift(void);
static void flash_write_cmd(unsigned cmd_addr,int cmd);
static int intelFlashClearStatus();
static int intelFlashWaitforStatus(unsigned long addr,long*	timeoutp);
static unsigned long flash_make_val(unsigned long   inval);
static unsigned long flash_read_array(unsigned	addr);
static unsigned long flash_read_val(unsigned addr);
static unsigned long queryFlashID(unsigned long flashWordAddress);
static unsigned long queryFlash(unsigned long flashWordAddress);
static int intelFlashEraseRange(unsigned long startAddress, unsigned long len);
static int intelFlashProgramBlock(unsigned long flashAddress, unsigned long *values, int nbytes);
static int intelFlashReadStatus();
static int intelFlashProtectRange(unsigned long startAddress, unsigned long len, int protect);
static int intelFlashReset ();
static int intelFlashProgramWord(unsigned long flashAddress, unsigned long value);
static int intelFlashEraseChip ();
static int program_flash(unsigned long p,unsigned long size,HWND hwndPB);
static BOOL isValidBootloader(unsigned long p,unsigned long size);
static BOOL isValidOHHImage(unsigned long p,unsigned long size);
static BOOL isValidParrotImage(unsigned long p,unsigned long size);
static BOOL isErased(unsigned long p,unsigned long size);
static unsigned int bsd_sum_memory(unsigned long img_src, size_t img_size); 
static unsigned long guessSize(void);
static BOOL save_flash(HWND hWnd,volatile unsigned long *p,unsigned long size,TCHAR *fName);
static void flash_write_val(unsigned addr,int val);
static void Delay(unsigned long count);
static BOOL save_flash_gz(HWND hWnd,volatile unsigned long *p,unsigned long size,TCHAR *fName);
static unsigned long LoadFileGZ(const TCHAR *fname,unsigned long *ulSize);
static int GetDeviceCode(unsigned long *p);
static void DisplayLLErrorText(HWND hWnd,TCHAR *pM,TCHAR *pF,UINT line);
static void clear_status(void);
static int discover_machine_type(volatile unsigned short *p);
//stubs
static void putLabeledWord(char *p,unsigned long v);
static void putstr(char *p);

//crap
int ProgramFlashBav(HWND hWnd);
int EraseFlashBav(HWND hWnd);

#endif // !defined(AFX_FLASHUTILS_H__BDCBB719_CD62_11D5_A244_00508BA08C36__INCLUDED_)
