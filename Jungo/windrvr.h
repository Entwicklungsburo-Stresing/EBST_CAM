/* Jungo Connectivity Confidential. Copyright (c) 2016 Jungo Connectivity Ltd.  http://www.jungo.com */

/*
 * W i n D r i v e r
 * =================
 *
 * FOR DETAILS ON THE WinDriver FUNCTIONS, PLEASE SEE THE WinDriver MANUAL
 * OR INCLUDED HELP FILES.
 *
 * This file may not be distributed, it may only be used for development
 * or evaluation purposes. The only exception is distribution to Linux.
 * For details refer to \WinDriver\docs\license.txt.
 *
 * Web site: http://www.jungo.com
 * Email:    support@jungo.com
 */
#ifndef _WINDRVR_H_
#define _WINDRVR_H_

#if defined(__cplusplus)
    extern "C" {
#endif

#include "wd_ver.h"

#if defined(WIN32)
    #define DLLCALLCONV __stdcall
#else
    #define DLLCALLCONV
#endif

#if defined(WIN32) || defined(WINCE)
    #define WD_DRIVER_NAME_PREFIX "\\\\.\\"
#elif defined(LINUX)
    #define WD_DRIVER_NAME_PREFIX "/dev/"
#else
    #define WD_DRIVER_NAME_PREFIX ""
#endif

#if !defined(APPLE) && !defined(CONFIG_WD_LITE)
    /* This definition is used at do_setenv and should be before other
     * alternative definitions */
    /* If version suffix removed, it should be also removed from do_setenv
     * script */
    #define WD_DEFAULT_DRIVER_NAME_BASE "windrvr" WD_VER_ITOA
    #define WD_DEFAULT_DRIVER_NAME \
        WD_DRIVER_NAME_PREFIX WD_DEFAULT_DRIVER_NAME_BASE
#elif defined(CONFIG_WD_LITE)
    #define WD_DEFAULT_DRIVER_NAME_BASE "windrvr_lite"
    #define WD_DEFAULT_DRIVER_NAME \
        WD_DRIVER_NAME_PREFIX WD_DEFAULT_DRIVER_NAME_BASE
#else /* Apple */
    /* Driver name should be same as the name of the main class */
    #define WD_DEFAULT_DRIVER_NAME_BASE "WinDriver"
    #define WD_DEFAULT_DRIVER_NAME WD_DEFAULT_DRIVER_NAME_BASE
#endif

#define WD_MAX_DRIVER_NAME_LENGTH 128

#if defined(WD_DRIVER_NAME_CHANGE)
    const char* DLLCALLCONV WD_DriverName(const char *sName);
    /* Get driver name */
    #define WD_DRIVER_NAME WD_DriverName(NULL)
#else
    #define WD_DRIVER_NAME WD_DEFAULT_DRIVER_NAME
#endif

#if defined(CONFIG_WD_LITE)
    #define WD_PROD_NAME "WinDriver PCI Viewer"
#else
    #define WD_PROD_NAME "WinDriver"
#endif

#if !defined(SPARC) && (defined(__sparc__) || defined(__sparc) || \
        defined(sparc))
    #define SPARC
#endif

#if !defined(POWERPC) && !defined(PPC64) && !defined(IA64) && \
    !defined(x86) && ( defined(LINUX) || (defined(WIN32) && \
    !defined(WINCE) && !defined(_ALPHA_)) || \
    ((defined(APPLE) && ((defined(__i386__) || defined(__x86_64__))))))
    #define x86
#endif

#if defined(APPLE) && defined(__LP64__) && !defined(KERNEL_64BIT)
    #define KERNEL_64BIT
#endif

#if !defined(x86_64) && \
    (defined(x86) && (defined(KERNEL_64BIT) || defined(__x86_64__)))
    #define x86_64
#endif

#if defined(WINCE)
    #if defined(ARM)
        #define WD_CPU_SPEC " ARM"
    #elif defined(MIPS)
        #define WD_CPU_SPEC " MIPS"
    #elif defined(x86)
        #define WD_CPU_SPEC " x86"
    #else
        #define WD_CPU_SPEC ""
    #endif
#else
    #if defined(SPARC)
        #define WD_CPU_SPEC " Sparc"
    #elif defined(POWERPC)
        #define WD_CPU_SPEC " PowerPC"
    #elif defined(PPC64)
        #define WD_CPU_SPEC " ppc64"
    #elif defined(x86_64)
        #define WD_CPU_SPEC " x86_64"
    #elif defined(IA64)
        #define WD_CPU_SPEC " ia64"
    #else
        #define WD_CPU_SPEC " X86"
    #endif
#endif

#if defined(WINNT)
    #define WD_FILE_FORMAT " SYS"
#elif defined(WINCE)
    #define WD_FILE_FORMAT " DLL"
#else
    #define WD_FILE_FORMAT ""
#endif

#if defined(KERNEL_64BIT)
    #define WD_DATA_MODEL " 64bit"
#else
    #define WD_DATA_MODEL " 32bit"
#endif

#define WD_VER_STR  WD_PROD_NAME " v" WD_VERSION_STR \
    " Jungo Connectivity (c) 1997 - " COPYRIGHTS_YEAR_STR \
        " Build Date: " __DATE__ \
    WD_CPU_SPEC WD_DATA_MODEL WD_FILE_FORMAT

#if !defined(POSIX) && (defined(LINUX) || defined(APPLE))
    #define POSIX
#endif

#if !defined(UNIX) && defined(POSIX)
    #define UNIX
#endif

#if !defined(WIN32) && (defined(WINCE) || defined(WINNT))
    #define WIN32
#endif

#if defined(_WIN32_WCE) && !defined(WINCE)
    #define WINCE
#endif

#if !defined(WIN32) && !defined(WINCE) && !defined(UNIX) && !defined(APPLE)
    #define WIN32
#endif

#if defined(_KERNEL_MODE) && !defined(KERNEL)
    #define KERNEL
#endif

#if defined(KERNEL) && !defined(__KERNEL__)
    #define __KERNEL__
#endif

#if defined(_KERNEL) && !defined(__KERNEL__)
    #define __KERNEL__
#endif

#if defined( __KERNEL__) && !defined(_KERNEL)
    #define _KERNEL
#endif

#if !defined(POWERPC) && defined(APPLE) && !defined(x86)
    #define POWERPC
#endif
#if !defined(_BIGENDIAN) && (defined(SPARC) || defined(POWERPC) || \
    defined(PPC64))
    #define _BIGENDIAN
#endif

#if defined(LINUX) && defined(x86_64) && !defined(__KERNEL__)
    /* This fixes binary compatibility with older version of GLIBC
     * (64bit only) */
    __asm__(".symver memcpy,memcpy@GLIBC_2.2.5");
#endif

/* Structures on PPC64 are 8 byte aligned, even on 32 bit applications,
 * therefore packing is necessary. */
#if defined(LINUX) && defined(PPC64)
    #pragma pack(push, 1)
#endif

#if defined(APPLE)
    #if defined(__KERNEL__)
        #include <IOKit/IOTypes.h>
    #else
        #include <AvailabilityMacros.h>
        #include <IOKit/IOKitLib.h>
        #include <stdio.h>
    #endif
    enum {
        kWinDriverMethodSyncIoctl = 0,
        kWinDriverNumOfMethods,
    };

#endif

#if defined(APPLE)
    #include <libkern/OSAtomic.h>

    #if !defined(__P_TYPES__)
        #define __P_TYPES__
        typedef void VOID;
        typedef unsigned char UCHAR;
        typedef UInt16 USHORT;
        typedef UInt32 UINT;
        typedef UInt32 ULONG;
        typedef unsigned long BOOL;
        typedef void *PVOID;
        typedef UInt8 *PBYTE;
        typedef char CHAR;
        typedef char *PCHAR;
        typedef UInt16 *PWORD;
        typedef unsigned long DWORD, *PDWORD;
        typedef PVOID HANDLE;
        typedef UInt8 u8;
        typedef UInt16 u16;
        typedef UInt32 u32;
        typedef void *FILEHANDLE;
        typedef UInt32 PRCHANDLE;
    #if !defined(__KERNEL__)
        #include <string.h>
            #include <ctype.h>
        #include <stdlib.h>
    #endif
    #ifndef TRUE
        #define TRUE true
    #endif
    #ifndef FALSE
        #define FALSE false
    #endif
    #define __cdecl
    #define WINAPI
    #endif
    #ifdef __LP64__
        #define PTR2INT(value) ((UInt64)(value))
    #else
        #define PTR2INT(value) ((UInt32)(value))
    #endif
    #define MAC_UC_MAGIC_MASK 0xFF0000FF
    #define MAC_UC_MAGIC_NUM  0x7E000041
    #define MAC_UC_64BIT_APP  0x100
#endif

#if defined(UNIX)
    #if !defined(__P_TYPES__)
        #define __P_TYPES__
        typedef void VOID;
        typedef unsigned char UCHAR;
        typedef unsigned short USHORT;
        typedef unsigned int UINT;
        typedef unsigned long ULONG;
        typedef ULONG BOOL;
        typedef void *PVOID;
        typedef unsigned char *PBYTE;
        typedef char CHAR;
        typedef char *PCHAR;
        typedef unsigned short *PWORD;
        typedef unsigned long DWORD, *PDWORD;
        typedef PVOID HANDLE;
    #endif
    #if !defined(__KERNEL__)
        #include <string.h>
        #include <ctype.h>
        #include <stdlib.h>
    #endif
    #ifndef TRUE
        #define TRUE 1
    #endif
    #ifndef FALSE
        #define FALSE 0
    #endif
    #define __cdecl
    #define WINAPI

    #if defined(__KERNEL__)
        #if defined(LINUX)
            /* For _IO macros and for mapping Linux status codes
             * to WD status codes */
            #if defined(POWERPC) || defined(PPC64)
                #include <asm/ioctl.h>
                #include <linux/errno.h>
            #else
                #include <asm-generic/ioctl.h>
                #include <asm-generic/errno.h>
            #endif
        #endif
    #else
        #include <unistd.h>
        #if defined(LINUX)
            #include <sys/ioctl.h> /* for BSD ioctl() */
            #include <unistd.h>
        #else
            #include <unistd.h> /* for SVR4 ioctl()*/
        #endif
        #include <sys/types.h>
        #include <sys/stat.h>
        #include <fcntl.h>
    #endif
        typedef unsigned long long UINT64;
#elif defined(WINCE)
    #include <windows.h>
    #include <winioctl.h>
    typedef char CHAR;
    typedef unsigned __int64 UINT64;
    #define perror(str) fprintf(stderr, "%s: error no. %d\n", (str), \
    GetLastError())
    #if defined(PRIO_WIN32_TO_CE)
        #define WD_CE6_PRIORITY(prio) PRIO_WIN32_TO_CE(prio)
    #else
        /* Define CE6_256 priority level as defined in winbase.h
         * MAX_CE_PRIORITY_LEVELS = 256
         * MAX_WIN32_PRIORITY_LEVELS = 8
         * this definition exist only for CE 6.00 and up */
        #define WD_CE6_PRIORITY(prio) ((prio) + 256 - 8)
    #endif
#elif defined(WIN32)
    #if defined(__KERNEL__)
        int sprintf(char *buffer, const char *format, ...);
    #else
        #include <windows.h>
        #include <winioctl.h>
    #endif
    typedef unsigned __int64 UINT64;
#endif

#if !defined(__KERNEL__)
    #include <stdarg.h>
    #if !defined(va_copy) && !defined(__va_copy)
        #define va_copy(ap2,ap1) (ap2)=(ap1)
    #endif
    #if !defined(va_copy) && defined(__va_copy)
        #define va_copy __va_copy
    #endif
#endif

#ifndef WINAPI
    #define WINAPI
#endif

#if !defined(_WINDEF_)
    typedef unsigned char BYTE;
    typedef unsigned short int WORD;
#endif

#if !defined(_BASETSD_H_)
    typedef unsigned int UINT32;
#endif

// formatting for printing a 64bit variable
#if defined(UNIX) || defined(APPLE)
    #define PRI64       "ll"
#elif defined(WIN32)
    #define PRI64       "I64"
#endif

// formatting for printing a kernel pointer
#if defined(KERNEL_64BIT)
    #define KPRI PRI64
#else
    #define KPRI "l"
#endif

/*
 * The KPTR is guaranteed to be the same size as a kernel-mode pointer
 * The UPTR is guaranteed to be the same size as a user-mode pointer
 */
#if defined(KERNEL_64BIT)
    typedef UINT64 KPTR;
#else
    typedef DWORD KPTR;
#endif

#if defined(__KERNEL_)
    typedef KPTR UPTR;
#else
    #if defined(UNIX)
        typedef unsigned long UPTR;
    #else
        typedef size_t UPTR;
    #endif
#endif

typedef UINT64 DMA_ADDR;
typedef UINT64 PHYS_ADDR;

#include "windrvr_usb.h"

#if defined(WIN32)
    #if !defined(inline)
        #define inline __inline
    #endif
#endif

// IN WD_TRANSFER_CMD and WD_Transfer() DWORD stands for 32 bits and QWORD is 64
// bit.
typedef enum
{
    CMD_NONE = 0,       // No command
    CMD_END = 1,        // End command
    CMD_MASK = 2,       // Interrupt Mask

    RP_BYTE = 10,       // Read port byte
    RP_WORD = 11,       // Read port word
    RP_DWORD = 12,      // Read port dword
    WP_BYTE = 13,       // Write port byte
    WP_WORD = 14,       // Write port word
    WP_DWORD = 15,      // Write port dword
    RP_QWORD = 16,      // Read port qword
    WP_QWORD = 17,      // Write port qword

    RP_SBYTE = 20,      // Read port string byte
    RP_SWORD = 21,      // Read port string word
    RP_SDWORD = 22,     // Read port string dword
    WP_SBYTE = 23,      // Write port string byte
    WP_SWORD = 24,      // Write port string word
    WP_SDWORD = 25,     // Write port string dword
    RP_SQWORD = 26,     // Read port string qword
    WP_SQWORD = 27,     // Write port string qword

    RM_BYTE = 30,       // Read memory byte
    RM_WORD = 31,       // Read memory word
    RM_DWORD = 32,      // Read memory dword
    WM_BYTE = 33,       // Write memory byte
    WM_WORD = 34,       // Write memory word
    WM_DWORD = 35,      // Write memory dword
    RM_QWORD = 36,      // Read memory qword
    WM_QWORD = 37,      // Write memory qword

    RM_SBYTE = 40,      // Read memory string byte
    RM_SWORD = 41,      // Read memory string word
    RM_SDWORD = 42,     // Read memory string dword
    WM_SBYTE = 43,      // Write memory string byte
    WM_SWORD = 44,      // Write memory string word
    WM_SDWORD = 45,     // Write memory string dword
    RM_SQWORD = 46,     // Read memory string quad word
    WM_SQWORD = 47      // Write memory string quad word
} WD_TRANSFER_CMD;

enum { WD_DMA_PAGES = 256 };

typedef enum {
    DMA_KERNEL_BUFFER_ALLOC = 0x1, // The system allocates a contiguous buffer.
        // The user does not need to supply linear address.

    DMA_KBUF_BELOW_16M = 0x2, // If DMA_KERNEL_BUFFER_ALLOC is used,
        // this will make sure it is under 16M.

    DMA_LARGE_BUFFER = 0x4, // If DMA_LARGE_BUFFER is used,
        // the maximum number of pages are dwPages, and not
        // WD_DMA_PAGES. If you lock a user buffer (not a kernel
        // allocated buffer) that is larger than 1MB, then use this
        // option and allocate memory for pages.

    DMA_ALLOW_CACHE = 0x8,  // Allow caching of contiguous memory.

    DMA_KERNEL_ONLY_MAP = 0x10, // Only map to kernel, dont map to user-mode.
        // relevant with DMA_KERNEL_BUFFER_ALLOC flag only

    DMA_FROM_DEVICE = 0x20, // memory pages are locked to be written by device

    DMA_TO_DEVICE = 0x40, // memory pages are locked to be read by device

    DMA_TO_FROM_DEVICE = (DMA_FROM_DEVICE | DMA_TO_DEVICE), // memory pages are
        // locked for both read and write

    DMA_ALLOW_64BIT_ADDRESS = 0x80, // Use this value for devices that support
                                    // 64-bit DMA addressing.

    DMA_ALLOW_NO_HCARD = 0x100, // Allow memory lock without hCard

    DMA_GET_EXISTING_BUF = 0x200, // Get existing buffer by hDma handle
} WD_DMA_OPTIONS;

#define DMA_DIRECTION_MASK DMA_TO_FROM_DEVICE

/* Macros for backward compatibility */
#define DMA_READ_FROM_DEVICE DMA_FROM_DEVICE
#define DMA_WRITE_TO_DEVICE DMA_TO_DEVICE

enum {
    WD_MATCH_EXCLUDE = 0x1 /* Exclude if there is a match */
};

enum {
    WD_CPCI_PNP_SUPPORT = 0x1   /* Set this flag to use OS PnP support */
};

typedef struct
{
    DMA_ADDR pPhysicalAddr; // Physical address of page.
    DWORD dwBytes;          // Size of page.
} WD_DMA_PAGE, WD_DMA_PAGE_V80;

typedef struct
{
    DWORD hDma;             // Handle of DMA buffer
    PVOID pUserAddr;        // User address
    KPTR  pKernelAddr;      // Kernel address
    DWORD dwBytes;          // Size of buffer
    DWORD dwOptions;        // Refer to WD_DMA_OPTIONS
    DWORD dwPages;          // Number of pages in buffer.
    DWORD hCard;            // Handle of relevant card as received from
                            // WD_CardRegister()
    WD_DMA_PAGE Page[WD_DMA_PAGES];
} WD_DMA, WD_DMA_V80;

typedef enum {
    KER_BUF_ALLOC_NON_CONTIG = 0x0001,
    KER_BUF_ALLOC_CONTIG     = 0x0002,
    KER_BUF_ALLOC_CACHED     = 0x0004,
    KER_BUF_GET_EXISTING_BUF = 0x0008,
} WD_KER_BUF_OPTION;

typedef struct
{
    DWORD hKerBuf;    /* Handle of Kernel Buffer */
    DWORD dwOptions;  /* Refer to WD_KER_BUF_OPTION */
    UINT64 qwBytes;   /* Size of buffer */
    KPTR pKernelAddr; /* Kernel address */
    UPTR pUserAddr;   /* User address */
} WD_KERNEL_BUFFER, WD_KERNEL_BUFFER_V121;

typedef struct
{
    KPTR  pPort;       // I/O port for transfer or kernel memory address.
    DWORD cmdTrans;    // Transfer command WD_TRANSFER_CMD.

    // Parameters used for string transfers:
    DWORD dwBytes;     // For string transfer.
    DWORD fAutoinc;    // Transfer from one port/address
                       // or use incremental range of addresses.
    DWORD dwOptions;   // Must be 0.
    union
    {
        BYTE Byte;     // Use for 8 bit transfer.
        WORD Word;     // Use for 16 bit transfer.
        UINT32 Dword;  // Use for 32 bit transfer.
        UINT64 Qword;  // Use for 64 bit transfer.
        PVOID pBuffer; // Use for string transfer.
    } Data;
} WD_TRANSFER, WD_TRANSFER_V61;

enum {
    INTERRUPT_LATCHED = 0x00,
    INTERRUPT_LEVEL_SENSITIVE = 0x01,
    INTERRUPT_CMD_COPY = 0x02,
    INTERRUPT_CE_INT_ID = 0x04,
    INTERRUPT_CMD_RETURN_VALUE = 0x08,
    INTERRUPT_MESSAGE = 0x10,
    INTERRUPT_MESSAGE_X = 0x20
};

typedef struct
{
    DWORD hKernelPlugIn;
    DWORD dwMessage;
    PVOID pData;
    DWORD dwResult;
} WD_KERNEL_PLUGIN_CALL, WD_KERNEL_PLUGIN_CALL_V40;

typedef enum {
    INTERRUPT_RECEIVED = 0, /* Interrupt was received */
    INTERRUPT_STOPPED,      /* Interrupt was disabled during wait */
    INTERRUPT_INTERRUPTED   /* Wait was interrupted before an actual hardware
                             * interrupt was received */
} WD_INTERRUPT_WAIT_RESULT;

typedef struct
{
    DWORD hInterrupt;    // Handle of interrupt.
    DWORD dwOptions;     // Interrupt options: can be INTERRUPT_CMD_COPY

    WD_TRANSFER *Cmd;    // Commands to do on interrupt.
    DWORD dwCmds;        // Number of commands.

    // For WD_IntEnable():
    WD_KERNEL_PLUGIN_CALL kpCall; // Kernel PlugIn call.
    DWORD fEnableOk;     // TRUE if interrupt was enabled (WD_IntEnable()
                         // succeed).

    // For WD_IntWait() and WD_IntCount():
    DWORD dwCounter;     // Number of interrupts received.
    DWORD dwLost;        // Number of interrupts not yet dealt with.
    DWORD fStopped;      // Was interrupt disabled during wait.

    DWORD dwLastMessage; // Message data of the last received MSI (Windows)
    DWORD dwEnabledIntType; // Interrupt type that was actually enabled
} WD_INTERRUPT, WD_INTERRUPT_V91;

typedef struct
{
    DWORD dwVer;
    CHAR cVer[128];
} WD_VERSION, WD_VERSION_V30;

enum
{
    LICENSE_DEMO     = 0x00000001,
    LICENSE_WD       = 0x00000004,
    LICENSE_IO       = 0x00000008,
    LICENSE_MEM      = 0x00000010,
    LICENSE_INT      = 0x00000020,
    LICENSE_PCI      = 0x00000040,
    LICENSE_DMA      = 0x00000080,
    LICENSE_NT       = 0x00000100,
    LICENSE_ISAPNP   = 0x00000400,
    LICENSE_PCMCIA   = 0x00000800,
    LICENSE_PCI_DUMP = 0x00001000,
    LICENSE_MSG_GEN  = 0x00002000,
    LICENSE_MSG_EDU  = 0x00004000,
    LICENSE_MSG_INT  = 0x00008000,
    LICENSE_KER_PLUG = 0x00010000,
    LICENSE_LINUX    = 0x00020000,
    LICENSE_CE       = 0x00080000,
    LICENSE_CE2013   = 0x00800000,
    LICENSE_THIS_PC  = 0x00100000,
    LICENSE_WIZARD   = 0x00200000,
    LICENSE_KD       = 0x00400000,
    LICENSE_CPU0     = 0x00040000,
    LICENSE_CPU1     = 0x01000000,
    LICENSE_CPU2     = 0x02000000,
    LICENSE_CPU3     = 0x04000000,
    LICENSE_USB      = 0x08000000,
    LICENSE_IPC      = 0x10000000,
    LICENSE_APPLE    = 0x20000000,
    LICENSE_KER_BUF  = 0x40000000,
};

enum
{
    LICENSE2_EVENT        = 0x00000008,
    LICENSE2_WDLIB        = 0x00000010,
    LICENSE2_VISTA        = 0x00000080,
    LICENSE2_OS_UNLIMITED = 0x00000200
};

enum
{
    LICENSE_OS_WITH_WIZARD = LICENSE_NT | LICENSE_LINUX | LICENSE_APPLE,
    LICENSE_OS_WITHOUT_WIZARD = LICENSE_CE | LICENSE_CE2013
};

enum
{
    LICENSE_CPU_ALL = LICENSE_CPU3 | LICENSE_CPU2 | LICENSE_CPU1 | LICENSE_CPU0,
    LICENSE_X86 = LICENSE_CPU0,
    LICENSE_ALPHA = LICENSE_CPU1,
    LICENSE_SPARC = LICENSE_CPU1 | LICENSE_CPU0,
    LICENSE_PPC = LICENSE_CPU2,
    LICENSE_X86_64 = LICENSE_CPU2 | LICENSE_CPU0,
    LICENSE_IA64 = LICENSE_CPU1 | LICENSE_CPU2,
    LICENSE_PPC64 = LICENSE_CPU2 | LICENSE_CPU1 | LICENSE_CPU0
};

#define WD_LICENSE_LENGTH 1024
typedef struct
{
    CHAR cLicense[WD_LICENSE_LENGTH]; // Buffer with license string to put.
                      // If empty string then get current license setting
                      // into dwLicense.
    DWORD dwLicense;  // Returns license settings: LICENSE_DEMO, LICENSE_WD
                      // etc..., or 0 for invalid license.
    DWORD dwLicense2; // Returns additional license settings, if dwLicense
                      // could not hold all the information.
                      // Then dwLicense will return 0.
} WD_LICENSE, WD_LICENSE_V116;

enum
{
    WD_BUS_USB = (int)0xfffffffe,   /* USB */
    WD_BUS_UNKNOWN = 0,             /* Unknown bus type */
    WD_BUS_ISA = 1,                 /* ISA */
    WD_BUS_EISA = 2,                /* EISA, including ISA PnP */
    WD_BUS_PCI = 5,                 /* PCI */
    WD_BUS_PCMCIA = 8               /* PCMCIA */
};
typedef DWORD WD_BUS_TYPE;

typedef struct
{
    WD_BUS_TYPE dwBusType;  /* Bus Type: WD_BUS_PCI/PCMCIA/ISA/EISA */
    DWORD dwBusNum;         /* Bus number */
    DWORD dwSlotFunc;       /* Slot number on the bus */
} WD_BUS, WD_BUS_V30;

typedef enum
{
    ITEM_NONE         = 0,
    ITEM_INTERRUPT    = 1, /* Interrupt */
    ITEM_MEMORY       = 2, /* Memory */
    ITEM_IO           = 3, /* I/O */
    ITEM_BUS          = 5  /* Bus */
} ITEM_TYPE;

typedef enum
{
    WD_ITEM_MEM_DO_NOT_MAP_KERNEL = 1, /* Skip the mapping of physical memory to
                                        * the kernel address space */
    WD_ITEM_MEM_ALLOW_CACHE = 2,       /* Map physical memory as cached;
                                        * applicable only to host RAM, not to
                                        * local memory on the card */
    WD_ITEM_MEM_CE_MAP_VIRTUAL = 3     /* For Windows CE only -- perform kernel
                                        * mapping of physical memory to
                                        * non-static virtual system addresses,
                                        * as opposed to the default static
                                        * kernel address mapping.
                                        * NOTE: Do not set this flag for items
                                        * that need to be accessed in kernel
                                        * mode -- namely, items that will be
                                        * used to create interrupt transfer
                                        * commands or items that will be used
                                        * from a Kernel PlugIn driver.  */
} WD_ITEM_MEM_OPTIONS;

typedef struct
{
    DWORD item;         /* ITEM_TYPE */
    DWORD fNotSharable;
    union
    {
        /* ITEM_MEMORY */
        struct
        {
            PHYS_ADDR pPhysicalAddr; /* Physical address on card */
            UINT64 qwBytes;          /* Address range */
            KPTR pTransAddr;         /* Kernel-mode mapping of the physical base
                                        address, to be used for transfer
                                        commands; returned by WD_CardRegister()
                                      */
            UPTR pUserDirectAddr;    /* User-mode mapping of the physical base
                                        address, for direct user read/write
                                        transfers; returned by WD_CardRegister()
                                      */
            DWORD dwBar;             /* PCI Base Address Register number */
            DWORD dwOptions;         /* Bitmask of WD_ITEM_MEM_OPTIONS flags */
            KPTR pReserved;          /* Reserved for internal use */
        } Mem;

        /* ITEM_IO */
        struct
        {
            KPTR  pAddr;    /* Beginning of I/O address */
            DWORD dwBytes;  /* I/O range */
            DWORD dwBar;    /* PCI Base Address Register number */
        } IO;

        /* ITEM_INTERRUPT */
        struct
        {
            DWORD dwInterrupt;  /* Number of interrupt to install */
            DWORD dwOptions;    /* Interrupt options:
                        INTERRUPT_LATCHED -- latched
                        INTERRUPT_LEVEL_SENSITIVE -- level sensitive
                        INTERRUPT_MESSAGE -- Message-Signaled Interrupts (MSI)
                        INTERRUPT_MESSAGE_X -- Extended MSI (MSI-X) */
            DWORD hInterrupt;   /* Handle of the installed interrupt; returned
                                   by WD_CardRegister() */
            DWORD dwReserved1;  /* For internal use */
            KPTR  pReserved2;   /* For internal use */
        } Int;
        WD_BUS Bus; /* ITEM_BUS */
    } I;
} WD_ITEMS, WD_ITEMS_V118;

enum { WD_CARD_ITEMS = 20 };

typedef struct
{
    DWORD dwItems;
    WD_ITEMS Item[WD_CARD_ITEMS];
} WD_CARD, WD_CARD_V118;

typedef struct
{
    WD_CARD Card;           // Card to register.
    DWORD fCheckLockOnly;   // Only check if card is lockable, return hCard=1 if
                            // OK.
    DWORD hCard;            // Handle of card.
    DWORD dwOptions;        // Should be zero.
    CHAR cName[32];         // Name of card.
    CHAR cDescription[100]; // Description.
} WD_CARD_REGISTER, WD_CARD_REGISTER_V118;

#define WD_PROCESS_NAME_LENGTH 128
typedef struct
{
    CHAR  cProcessName[WD_PROCESS_NAME_LENGTH];
    DWORD dwSubGroupID; /* Identifier of the processes type */
    DWORD dwGroupID;  /* Unique identifier of the processes group for discarding
                       * unrelated process. WinDriver developers are encouraged
                       * to change their driver name before distribution to
                       * avoid this issue entirely. */
    DWORD hIpc;       /* Returned from WD_IpcRegister() */
} WD_IPC_PROCESS, WD_IPC_PROCESS_V121;

typedef struct
{
    WD_IPC_PROCESS procInfo;
    DWORD          dwOptions;  /* Reserved for future use; set to 0 */
} WD_IPC_REGISTER, WD_IPC_REGISTER_V121;

enum { WD_IPC_MAX_PROCS = 0x40 };

typedef struct
{
    DWORD          hIpc;       /* Returned from WD_IpcRegister() */

    /* Result processes */
    DWORD          dwNumProcs; /* Number of matching processes */
    WD_IPC_PROCESS procInfo[WD_IPC_MAX_PROCS];
} WD_IPC_SCAN_PROCS, WD_IPC_SCAN_PROCS_V121;

enum
{
    WD_IPC_UID_UNICAST = 0x1,
    WD_IPC_SUBGROUP_MULTICAST = 0x2,
    WD_IPC_MULTICAST = 0x4,
};

typedef struct
{
    DWORD hIpc;           /* Returned from WD_IpcRegister() */
    DWORD dwOptions;      /* WD_IPC_SUBGROUP_MULTICAST, WD_IPC_UID_UNICAST,
                           * WD_IPC_MULTICAST */

    DWORD  dwRecipientID; /* used only on WD_IPC_UNICAST */
    DWORD  dwMsgID;
    UINT64 qwMsgData;
} WD_IPC_SEND, WD_IPC_SEND_V121;

typedef struct
{
    DWORD hCard;            // Handle of card.
    WD_TRANSFER *Cmd;       // Buffer with WD_TRANSFER commands
    DWORD dwCmds;           // Number of commands.
    DWORD dwOptions;        // 0 (default) or WD_FORCE_CLEANUP
} WD_CARD_CLEANUP;

enum { WD_FORCE_CLEANUP = 0x1 };

enum { WD_PCI_CARDS = 100 }; // Slots max X Functions max

typedef struct
{
    DWORD dwBus;
    DWORD dwSlot;
    DWORD dwFunction;
} WD_PCI_SLOT;

typedef struct
{
    DWORD dwVendorId;
    DWORD dwDeviceId;
} WD_PCI_ID;

typedef struct
{
    /* Scan Parameters */
    WD_PCI_ID searchId;     /* PCI vendor and/or device IDs to search for;
                               dwVendorId==0 -- scan all PCI vendor IDs;
                               dwDeviceId==0 -- scan all PCI device IDs */
    DWORD dwCards;          /* Number of matching PCI cards */

    /* Scan Results */
    WD_PCI_ID cardId[WD_PCI_CARDS];     /* Array of matching card IDs */
    WD_PCI_SLOT cardSlot[WD_PCI_CARDS]; /* Array of matching PCI slots info */

    /* Scan Options */
    DWORD dwOptions;        /* Scan options -- WD_PCI_SCAN_OPTIONS */
} WD_PCI_SCAN_CARDS, WD_PCI_SCAN_CARDS_V80;

typedef enum {
    WD_PCI_SCAN_DEFAULT = 0x0,
    WD_PCI_SCAN_BY_TOPOLOGY = 0x1,
    WD_PCI_SCAN_REGISTERED = 0x2,
} WD_PCI_SCAN_OPTIONS;

enum { WD_PCI_MAX_CAPS = 50 };

enum { WD_PCI_CAP_ID_ALL = 0x0 };

typedef struct
{
    DWORD dwCapId;      /* PCI capability ID */
    DWORD dwCapOffset;  /* PCI capability register offset */
} WD_PCI_CAP;

typedef enum {
    WD_PCI_SCAN_CAPS_BASIC = 0x1,   /* Scan basic PCI capabilities */
    WD_PCI_SCAN_CAPS_EXTENDED = 0x2 /* Scan extended (PCIe) PCI capabilities */
} WD_PCI_SCAN_CAPS_OPTIONS;

typedef struct
{
    /* Scan Parameters */
    WD_PCI_SLOT pciSlot;   /* PCI slot information */
    DWORD       dwCapId;   /* PCI capability ID to search for, or
                              WD_PCI_CAP_ID_ALL to scan all PCI capabilities */
    DWORD       dwOptions; /* Scan options -- WD_PCI_SCAN_CAPS_OPTIONS;
                              default -- WD_PCI_SCAN_CAPS_BASIC */

    /* Scan Results */
    DWORD       dwNumCaps; /* Number of matching PCI capabilities */
    WD_PCI_CAP  pciCaps[WD_PCI_MAX_CAPS]; /* Array of matching PCI capabilities
                                           */
} WD_PCI_SCAN_CAPS, WD_PCI_SCAN_CAPS_V118;

typedef struct
{
    WD_PCI_SLOT pciSlot;    /* PCI slot information */
    WD_CARD Card;           /* Card information */
} WD_PCI_CARD_INFO, WD_PCI_CARD_INFO_V118;

typedef enum
{
    PCI_ACCESS_OK = 0,
    PCI_ACCESS_ERROR = 1,
    PCI_BAD_BUS = 2,
    PCI_BAD_SLOT = 3
} PCI_ACCESS_RESULT;

typedef enum
{
    PCMCIA_ACCESS_OK = 0,
    PCMCIA_BAD_SOCKET = 1,
    PCMCIA_BAD_OFFSET = 2,
    PCMCIA_ACCESS_ERROR = 3
} PCMCIA_ACCESS_RESULT;

typedef struct
{
    WD_PCI_SLOT pciSlot;    /* PCI slot information */
    PVOID       pBuffer;    /* Pointer to a read/write data buffer */
    DWORD       dwOffset;   /* PCI configuration space offset to read/write */
    DWORD       dwBytes;    /* Input -- number of bytes to read/write;
                               Output -- number of bytes read/written */
    DWORD       fIsRead;    /* 1 -- read data; 0 -- write data */
    DWORD       dwResult;   /* PCI_ACCESS_RESULT */
} WD_PCI_CONFIG_DUMP, WD_PCI_CONFIG_DUMP_V30;

enum { WD_ISAPNP_CARDS = 16 };
enum { WD_ISAPNP_COMPATIBLE_IDS = 10 };
enum { WD_ISAPNP_COMP_ID_LENGTH = 7 }; // ISA compressed ID is 7 chars long.
enum { WD_ISAPNP_ANSI_LENGTH = 32 }; // ISA ANSI ID is limited to 32 chars long.
typedef CHAR WD_ISAPNP_COMP_ID[WD_ISAPNP_COMP_ID_LENGTH + 1];
typedef CHAR WD_ISAPNP_ANSI[WD_ISAPNP_ANSI_LENGTH + 1 + 3]; // Add 3 bytes for
                                                            // DWORD alignment.
typedef struct
{
    WD_ISAPNP_COMP_ID cVendor; // Vendor ID.
    DWORD dwSerial; // Serial number of card.
} WD_ISAPNP_CARD_ID;

typedef struct
{
    WD_ISAPNP_CARD_ID cardId;  // VendorID & serial number of cards found.
    DWORD dwLogicalDevices;    // Logical devices on the card.
    BYTE bPnPVersionMajor;     // ISA PnP version Major.
    BYTE bPnPVersionMinor;     // ISA PnP version Minor.
    BYTE bVendorVersionMajor;  // Vendor version Major.
    BYTE bVendorVersionMinor;  // Vendor version Minor.
    WD_ISAPNP_ANSI cIdent;     // Device identifier.
} WD_ISAPNP_CARD, WD_ISAPNP_CARD_V40;

typedef struct
{
    WD_ISAPNP_CARD_ID searchId; // If searchId.cVendor[0]==0 - scan all vendor
                                // IDs. If searchId.dwSerial==0 - scan all
                                // serial numbers.
    DWORD dwCards;              // Number of cards found.
    WD_ISAPNP_CARD Card[WD_ISAPNP_CARDS]; // Cards found.
} WD_ISAPNP_SCAN_CARDS, WD_ISAPNP_SCAN_CARDS_V40;

typedef struct
{
    WD_ISAPNP_CARD_ID cardId;   // VendorID and serial number of card.
    DWORD dwLogicalDevice;      // Logical device in card.
    WD_ISAPNP_COMP_ID cLogicalDeviceId; // Logical device ID.
    DWORD dwCompatibleDevices;  // Number of compatible device IDs.
    WD_ISAPNP_COMP_ID CompatibleDevice[WD_ISAPNP_COMPATIBLE_IDS]; // Compatible
                                                                  // device IDs.
    WD_ISAPNP_ANSI cIdent;      // Device identifier.
    WD_CARD Card;               // Get card parameters for the ISA PnP card.
} WD_ISAPNP_CARD_INFO, WD_ISAPNP_CARD_INFO_V118;

typedef enum
{
    ISAPNP_ACCESS_OK = 0,
    ISAPNP_ACCESS_ERROR = 1,
    ISAPNP_BAD_ID = 2
} ISAPNP_ACCESS_RESULT;

typedef struct
{
    WD_ISAPNP_CARD_ID cardId; // VendorID and serial number of card.
    DWORD dwOffset;   // Offset in ISA PnP configuration space to read/write
                      // from.
    DWORD fIsRead;    // If 1 then read ISA PnP config, 0 write ISA PnP config.
    BYTE  bData;      // Result data of byte read/write.
    DWORD dwResult;   // ISAPNP_ACCESS_RESULT.
} WD_ISAPNP_CONFIG_DUMP, WD_ISAPNP_CONFIG_DUMP_V40;

// PCMCIA Card Services

// Extreme case - two PCMCIA slots and two multi-function (4 functions) cards
enum
{
    WD_PCMCIA_CARDS = 8,
    WD_PCMCIA_VERSION_LEN = 4,
    WD_PCMCIA_MANUFACTURER_LEN = 48,
    WD_PCMCIA_PRODUCTNAME_LEN = 48,
    WD_PCMCIA_MAX_SOCKET = 2,
    WD_PCMCIA_MAX_FUNCTION = 2
};

typedef struct
{
    BYTE uBus;      /* Bus number (first bus is 0) */
    BYTE uSocket;   /* Socket number (first socket is 0) */
    BYTE uFunction; /* Function number (first function is 0) */
    BYTE uPadding;  /* 1-byte padding so structure will be 4 bytes aligned */
} WD_PCMCIA_SLOT, WD_PCMCIA_SLOT_V622;

typedef struct
{
    WORD wManufacturerId; // card manufacturer
    WORD wCardId;         // card type and model
} WD_PCMCIA_ID;

typedef struct
{
    /* Scan Parameters */
    WD_PCMCIA_ID searchId;  /* PCMCIA vendor and/or device IDs to search for;
                               dwVendorId==0 -- scan all PCMCIA vendor IDs;
                               dwDeviceId==0 -- scan all PCMCIA device IDs */
    DWORD dwCards;          /* Number of matching PCI cards */

    /* Scan Results */
    WD_PCMCIA_ID cardId[WD_PCMCIA_CARDS];     /* Array of matching card IDs */
    WD_PCMCIA_SLOT cardSlot[WD_PCMCIA_CARDS]; /* Array of matching PCMCIA slots
                                                 information */

    /* Scan Options */
    DWORD dwOptions;    /* Reserved for future use; set to 0 */
} WD_PCMCIA_SCAN_CARDS, WD_PCMCIA_SCAN_CARDS_V622;

typedef struct
{
    WD_PCMCIA_SLOT pcmciaSlot; /* PCMCIA slot information */
    WD_CARD Card;              /* Card information */
    CHAR cVersion[WD_PCMCIA_VERSION_LEN];
    CHAR cManufacturer[WD_PCMCIA_MANUFACTURER_LEN];
    CHAR cProductName[WD_PCMCIA_PRODUCTNAME_LEN];
    WORD wManufacturerId;    // card manufacturer
    WORD wCardId;            // card type and model
    WORD wFuncId;            // card function code
    DWORD dwOptions;  // reserved for future use, set to 0
} WD_PCMCIA_CARD_INFO, WD_PCMCIA_CARD_INFO_V118;

typedef struct
{
    WD_PCMCIA_SLOT pcmciaSlot;  /* PCMCIA slot information */
    PVOID pBuffer;      /* Pointer to a read/write data buffer */
    DWORD dwOffset;     /* PCMCIA configuration space offset to read/write */
    DWORD dwBytes;      /* Input -- number of bytes to read/write;
                           Output -- number of bytes read/written */
    DWORD fIsRead;      /* 1 -- read data; 0 -- write data */
    DWORD dwResult;     /* PCMCIA_ACCESS_RESULT */
    DWORD dwOptions;    /* Reserved for future use; set to 0 */
} WD_PCMCIA_CONFIG_DUMP, WD_PCMCIA_CONFIG_DUMP_V622;

typedef enum {
    WD_PCMCIA_ACC_SPEED_DEFAULT, // use default access speed
    WD_PCMCIA_ACC_SPEED_250NS,
    WD_PCMCIA_ACC_SPEED_200NS,
    WD_PCMCIA_ACC_SPEED_150NS,
    WD_PCMCIA_ACC_SPEED_100NS
} WD_PCMCIA_ACC_SPEED;

typedef enum {
    WD_PCMCIA_ACC_WIDTH_DEFAULT, // use default bus width
    WD_PCMCIA_ACC_WIDTH_8BIT,
    WD_PCMCIA_ACC_WIDTH_16BIT
} WD_PCMCIA_ACC_WIDTH;

typedef enum {
    WD_PCMCIA_VPP_DEFAULT, // use default power level of the PCMCIA Vpp pin
    WD_PCMCIA_VPP_OFF,     // set voltage on the Vpp pin to zero (disable)
    WD_PCMCIA_VPP_ON,      // set voltage on the Vpp pin to 12V (enable)
    WD_PCMCIA_VPP_AS_VCC   // set voltage on the Vpp pin equal to Vcc
} WD_PCMCIA_VPP;

typedef struct
{
    DWORD dwOptions;            /* reserved for future use, set to 0 */
    WD_PCMCIA_SLOT pcmciaSlot;  /* PCMCIA slot information */
    BYTE uAccessSpeed;          /* One of WD_PCMCIA_ACC_SPEED_XXX values */
    BYTE uBusWidth;             /* One of WD_PCMCIA_ACC_WIDTH_XXX values */
    BYTE uVppLevel;             /* One of WD_PCMCIA_VPP_XXX values */
    BYTE uReserved;
    DWORD dwCardBase;           /* Offset in the PCMCIA device's memory where
                                   the memory mapping begins */
} WD_PCMCIA_CONTROL, WD_PCMCIA_CONTROL_V623;

enum { SLEEP_BUSY = 0, SLEEP_NON_BUSY = 1 };
typedef struct
{
    DWORD dwMicroSeconds; // Sleep time in Micro Seconds (1/1,000,000 Second)
    DWORD dwOptions;      // can be: SLEEP_NON_BUSY (10000 uSec +)
} WD_SLEEP, WD_SLEEP_V40;

typedef enum
{
    D_OFF       = 0,
    D_ERROR     = 1,
    D_WARN      = 2,
    D_INFO      = 3,
    D_TRACE     = 4
} DEBUG_LEVEL;

typedef enum
{
    S_ALL       = (int)0xffffffff,
    S_IO        = 0x00000008,
    S_MEM       = 0x00000010,
    S_INT       = 0x00000020,
    S_PCI       = 0x00000040,
    S_DMA       = 0x00000080,
    S_MISC      = 0x00000100,
    S_LICENSE   = 0x00000200,
    S_ISAPNP    = 0x00000400,
    S_PCMCIA    = 0x00000800,
    S_PNP       = 0x00001000,
    S_CARD_REG  = 0x00002000,
    S_KER_DRV   = 0x00004000,
    S_USB       = 0x00008000,
    S_KER_PLUG  = 0x00010000,
    S_EVENT     = 0x00020000,
    S_IPC       = 0x00040000,
    S_KER_BUF   = 0x00080000,
} DEBUG_SECTION;

typedef enum
{
    DEBUG_STATUS = 1,
    DEBUG_SET_FILTER = 2,
    DEBUG_SET_BUFFER = 3,
    DEBUG_CLEAR_BUFFER = 4,
    DEBUG_DUMP_SEC_ON = 5,
    DEBUG_DUMP_SEC_OFF = 6,
    KERNEL_DEBUGGER_ON = 7,
    KERNEL_DEBUGGER_OFF = 8,
    DEBUG_DUMP_CLOCK_ON = 9,
    DEBUG_DUMP_CLOCK_OFF = 10,
    DEBUG_CLOCK_RESET = 11
} DEBUG_COMMAND;

typedef struct
{
    DWORD dwCmd;     // DEBUG_COMMAND: DEBUG_STATUS, DEBUG_SET_FILTER,
                     // DEBUG_SET_BUFFER, DEBUG_CLEAR_BUFFER
    // used for DEBUG_SET_FILTER
    DWORD dwLevel;   // DEBUG_LEVEL: D_ERROR, D_WARN..., or D_OFF to turn
                     // debugging off
    DWORD dwSection; // DEBUG_SECTION: for all sections in driver: S_ALL
                     // for partial sections: S_IO, S_MEM...
    DWORD dwLevelMessageBox; // DEBUG_LEVEL to print in a message box
    // used for DEBUG_SET_BUFFER
    DWORD dwBufferSize; // size of buffer in kernel
} WD_DEBUG, WD_DEBUG_V40;

typedef struct
{
    PCHAR pcBuffer;  // buffer to receive debug messages
    DWORD dwSize;    // size of buffer in bytes
} WD_DEBUG_DUMP, WD_DEBUG_DUMP_V40;

typedef struct
{
    CHAR pcBuffer[256];
    DWORD dwLevel;
    DWORD dwSection;
} WD_DEBUG_ADD, WD_DEBUG_ADD_V503;

typedef struct
{
    DWORD hKernelPlugIn;
    PCHAR pcDriverName;
    PCHAR pcDriverPath; // Should be NULL (exists for backward compatibility).
                        // The driver will be searched in the operating
                        // system's drivers/modules directory.
    PVOID pOpenData;
} WD_KERNEL_PLUGIN, WD_KERNEL_PLUGIN_V40;

// IOCTL Structures
typedef enum
{
    WD_DEVICE_PCI = 0x1,
    WD_DEVICE_USB = 0x2
} WD_GET_DEVICE_PROPERTY_OPTION;

typedef struct
{
    union
    {
        HANDLE hDevice;
        DWORD dwUniqueID;
    } h;
    PVOID pBuf;
    DWORD dwBytes;
    DWORD dwProperty;
    DWORD dwOptions; /* WD_GET_DEVICE_PROPERTY_OPTION */
} WD_GET_DEVICE_PROPERTY;

typedef enum {
    WD_STATUS_SUCCESS = 0x0L,
    WD_STATUS_INVALID_WD_HANDLE = 0xffffffff,

    WD_WINDRIVER_STATUS_ERROR = 0x20000000L,

    WD_INVALID_HANDLE = 0x20000001L,
    WD_INVALID_PIPE_NUMBER = 0x20000002L,
    WD_READ_WRITE_CONFLICT = 0x20000003L, /* Request to read from an OUT (write)
                                           *  pipe or request to write to an IN
                                           *  (read) pipe */
    WD_ZERO_PACKET_SIZE = 0x20000004L, /* Maximum packet size is zero */
    WD_INSUFFICIENT_RESOURCES = 0x20000005L,
    WD_UNKNOWN_PIPE_TYPE = 0x20000006L,
    WD_SYSTEM_INTERNAL_ERROR = 0x20000007L,
    WD_DATA_MISMATCH = 0x20000008L,
    WD_NO_LICENSE = 0x20000009L,
    WD_NOT_IMPLEMENTED = 0x2000000aL,
    WD_KERPLUG_FAILURE = 0x2000000bL,
    WD_FAILED_ENABLING_INTERRUPT = 0x2000000cL,
    WD_INTERRUPT_NOT_ENABLED = 0x2000000dL,
    WD_RESOURCE_OVERLAP = 0x2000000eL,
    WD_DEVICE_NOT_FOUND = 0x2000000fL,
    WD_WRONG_UNIQUE_ID = 0x20000010L,
    WD_OPERATION_ALREADY_DONE = 0x20000011L,
    WD_USB_DESCRIPTOR_ERROR = 0x20000012L,
    WD_SET_CONFIGURATION_FAILED = 0x20000013L,
    WD_CANT_OBTAIN_PDO = 0x20000014L,
    WD_TIME_OUT_EXPIRED = 0x20000015L,
    WD_IRP_CANCELED = 0x20000016L,
    WD_FAILED_USER_MAPPING = 0x20000017L,
    WD_FAILED_KERNEL_MAPPING = 0x20000018L,
    WD_NO_RESOURCES_ON_DEVICE = 0x20000019L,
    WD_NO_EVENTS = 0x2000001aL,
    WD_INVALID_PARAMETER = 0x2000001bL,
    WD_INCORRECT_VERSION = 0x2000001cL,
    WD_TRY_AGAIN = 0x2000001dL,
    WD_WINDRIVER_NOT_FOUND = 0x2000001eL,
    WD_INVALID_IOCTL = 0x2000001fL,
    WD_OPERATION_FAILED = 0x20000020L,
    WD_INVALID_32BIT_APP = 0x20000021L,
    WD_TOO_MANY_HANDLES = 0x20000022L,
    WD_NO_DEVICE_OBJECT = 0x20000023L,
    // The following status codes are returned by USBD:
    // USBD status types:
    WD_USBD_STATUS_SUCCESS = 0x00000000L,
    WD_USBD_STATUS_PENDING = 0x40000000L,
    WD_USBD_STATUS_ERROR = (int)0x80000000L,
    WD_USBD_STATUS_HALTED = (int)0xC0000000L,

    // USBD status codes:
    // NOTE: The following status codes are comprised of one of the status
    // types above and an error code [i.e. 0xXYYYYYYYL - where: X = status type;
    // YYYYYYY = error code].
    // The same error codes may also appear with one of the other status types
    // as well.

    // HC (Host Controller) status codes.
    // [NOTE: These status codes use the WD_USBD_STATUS_HALTED status type]:
    WD_USBD_STATUS_CRC = (int)0xC0000001L,
    WD_USBD_STATUS_BTSTUFF = (int)0xC0000002L,
    WD_USBD_STATUS_DATA_TOGGLE_MISMATCH = (int)0xC0000003L,
    WD_USBD_STATUS_STALL_PID = (int)0xC0000004L,
    WD_USBD_STATUS_DEV_NOT_RESPONDING = (int)0xC0000005L,
    WD_USBD_STATUS_PID_CHECK_FAILURE = (int)0xC0000006L,
    WD_USBD_STATUS_UNEXPECTED_PID = (int)0xC0000007L,
    WD_USBD_STATUS_DATA_OVERRUN = (int)0xC0000008L,
    WD_USBD_STATUS_DATA_UNDERRUN = (int)0xC0000009L,
    WD_USBD_STATUS_RESERVED1 = (int)0xC000000AL,
    WD_USBD_STATUS_RESERVED2 = (int)0xC000000BL,
    WD_USBD_STATUS_BUFFER_OVERRUN = (int)0xC000000CL,
    WD_USBD_STATUS_BUFFER_UNDERRUN = (int)0xC000000DL,
#if defined(WINCE)
    WD_USBD_STATUS_NOT_ACCESSED_ALT = (int)0xC000000FL,  // HCD maps this to E
                                                         // when encountered
    WD_USBD_STATUS_NOT_ACCESSED = (int)0xC000000EL,
#else
    WD_USBD_STATUS_NOT_ACCESSED = (int)0xC000000FL,
#endif
    WD_USBD_STATUS_FIFO = (int)0xC0000010L,

#if defined(WIN32)
    WD_USBD_STATUS_XACT_ERROR = (int)0xC0000011L,
    WD_USBD_STATUS_BABBLE_DETECTED = (int)0xC0000012L,
    WD_USBD_STATUS_DATA_BUFFER_ERROR = (int)0xC0000013L,
#endif

#if defined(WINCE)
    WD_USBD_STATUS_ISOCH = 0xC0000100L,
    WD_USBD_STATUS_CANCELED = 0xC0000101L,
    WD_USBD_STATUS_NOT_COMPLETE = (int)0xC0000103L,
    WD_USBD_STATUS_CLIENT_BUFFER = (int)0xC0000104L,
#else
    WD_USBD_STATUS_CANCELED = (int)0xC0010000L,
#endif

    // Returned by HCD (Host Controller Driver) if a transfer is submitted to
    // an endpoint that is stalled:
    WD_USBD_STATUS_ENDPOINT_HALTED = (int)0xC0000030L,

    // Software status codes
    // [NOTE: The following status codes have only the error bit set]:
    WD_USBD_STATUS_NO_MEMORY = (int)0x80000100L,
    WD_USBD_STATUS_INVALID_URB_FUNCTION = (int)0x80000200L,
    WD_USBD_STATUS_INVALID_PARAMETER = (int)0x80000300L,

    // Returned if client driver attempts to close an endpoint/interface
    // or configuration with outstanding transfers:
    WD_USBD_STATUS_ERROR_BUSY = (int)0x80000400L,

    // Returned by USBD if it cannot complete a URB request. Typically this
    // will be returned in the URB status field when the Irp is completed
    // with a more specific error code. [The Irp status codes are indicated
    // in WinDriver's Debug Monitor tool (wddebug/wddebug_gui):
    WD_USBD_STATUS_REQUEST_FAILED = (int)0x80000500L,

    WD_USBD_STATUS_INVALID_PIPE_HANDLE = (int)0x80000600L,

    // Returned when there is not enough bandwidth available
    // to open a requested endpoint:
    WD_USBD_STATUS_NO_BANDWIDTH = (int)0x80000700L,

    // Generic HC (Host Controller) error:
    WD_USBD_STATUS_INTERNAL_HC_ERROR = (int)0x80000800L,

    // Returned when a short packet terminates the transfer
    // i.e. USBD_SHORT_TRANSFER_OK bit not set:
    WD_USBD_STATUS_ERROR_SHORT_TRANSFER = (int)0x80000900L,

    // Returned if the requested start frame is not within
    // USBD_ISO_START_FRAME_RANGE of the current USB frame,
    // NOTE: The stall bit is set:
    WD_USBD_STATUS_BAD_START_FRAME = (int)0xC0000A00L,

    // Returned by HCD (Host Controller Driver) if all packets in an
    // isochronous transfer complete with an error:
    WD_USBD_STATUS_ISOCH_REQUEST_FAILED = (int)0xC0000B00L,

    // Returned by USBD if the frame length control for a given
    // HC (Host Controller) is already taken by another driver:
    WD_USBD_STATUS_FRAME_CONTROL_OWNED = (int)0xC0000C00L,

    // Returned by USBD if the caller does not own frame length control and
    // attempts to release or modify the HC frame length:
    WD_USBD_STATUS_FRAME_CONTROL_NOT_OWNED = (int)0xC0000D00L

#if defined(WIN32)
    ,
    // Additional USB 2.0 software error codes added for USB 2.0:
    WD_USBD_STATUS_NOT_SUPPORTED = (int)0xC0000E00L, // Returned for APIS not
                                                     // supported/implemented
    WD_USBD_STATUS_INAVLID_CONFIGURATION_DESCRIPTOR = (int)0xC0000F00L,
    WD_USBD_STATUS_INSUFFICIENT_RESOURCES = (int)0xC0001000L,
    WD_USBD_STATUS_SET_CONFIG_FAILED = (int)0xC0002000L,
    WD_USBD_STATUS_BUFFER_TOO_SMALL = (int)0xC0003000L,
    WD_USBD_STATUS_INTERFACE_NOT_FOUND = (int)0xC0004000L,
    WD_USBD_STATUS_INAVLID_PIPE_FLAGS = (int)0xC0005000L,
    WD_USBD_STATUS_TIMEOUT = (int)0xC0006000L,
    WD_USBD_STATUS_DEVICE_GONE = (int)0xC0007000L,
    WD_USBD_STATUS_STATUS_NOT_MAPPED = (int)0xC0008000L,

    // Extended isochronous error codes returned by USBD.
    // These errors appear in the packet status field of an isochronous
    // transfer.

    // For some reason the controller did not access the TD associated with
    // this packet:
    WD_USBD_STATUS_ISO_NOT_ACCESSED_BY_HW = (int)0xC0020000L,
    // Controller reported an error in the TD.
    // Since TD errors are controller specific they are reported
    // generically with this error code:
    WD_USBD_STATUS_ISO_TD_ERROR = (int)0xC0030000L,
    // The packet was submitted in time by the client but
    // failed to reach the miniport in time:
    WD_USBD_STATUS_ISO_NA_LATE_USBPORT = (int)0xC0040000L,
    // The packet was not sent because the client submitted it too late
    // to transmit:
    WD_USBD_STATUS_ISO_NOT_ACCESSED_LATE = (int)0xC0050000L
#endif
} WD_ERROR_CODES;

typedef enum
{
    WD_INSERT                  = 0x1,
    WD_REMOVE                  = 0x2,
    WD_CPCI_REENUM             = 0x8,
    WD_POWER_CHANGED_D0        = 0x10, // Power states for the power management
    WD_POWER_CHANGED_D1        = 0x20,
    WD_POWER_CHANGED_D2        = 0x40,
    WD_POWER_CHANGED_D3        = 0x80,
    WD_POWER_SYSTEM_WORKING    = 0x100,
    WD_POWER_SYSTEM_SLEEPING1  = 0x200,
    WD_POWER_SYSTEM_SLEEPING2  = 0x400,
    WD_POWER_SYSTEM_SLEEPING3  = 0x800,
    WD_POWER_SYSTEM_HIBERNATE  = 0x1000,
    WD_POWER_SYSTEM_SHUTDOWN   = 0x2000,
    WD_IPC_UNICAST_MSG         = 0x4000,
    WD_IPC_MULTICAST_MSG       = 0x8000,
} WD_EVENT_ACTION;

#define WD_CPCI_INSERT (WD_INSERT | WD_CPCI_REENUM)
#define WD_CPCI_REMOVE (WD_REMOVE | WD_CPCI_REENUM)
#define WD_IPC_ALL_MSG (WD_IPC_UNICAST_MSG | WD_IPC_MULTICAST_MSG)

typedef enum
{
    WD_ACKNOWLEDGE              = 0x1,
    WD_ACCEPT_CONTROL           = 0x2  // used in WD_EVENT_SEND (acknowledge)
} WD_EVENT_OPTION;

#define WD_ACTIONS_POWER (WD_POWER_CHANGED_D0 | WD_POWER_CHANGED_D1 | \
    WD_POWER_CHANGED_D2 | WD_POWER_CHANGED_D3 | WD_POWER_SYSTEM_WORKING | \
    WD_POWER_SYSTEM_SLEEPING1 | WD_POWER_SYSTEM_SLEEPING3 | \
    WD_POWER_SYSTEM_HIBERNATE | WD_POWER_SYSTEM_SHUTDOWN)
#define WD_ACTIONS_ALL (WD_ACTIONS_POWER | WD_INSERT | WD_REMOVE)

typedef enum
{
    WD_EVENT_TYPE_UNKNOWN = 0,
    WD_EVENT_TYPE_PCI     = 1,
    WD_EVENT_TYPE_PCMCIA  = 2,
    WD_EVENT_TYPE_USB     = 3,
    WD_EVENT_TYPE_IPC     = 4,
} WD_EVENT_TYPE;

typedef struct
{
    DWORD hEvent;
    DWORD dwEventType;      /* WD_EVENT_TYPE */

    DWORD dwAction;         /* WD_EVENT_ACTION */
    DWORD dwEventId;
    DWORD hKernelPlugIn;
    DWORD dwOptions;        /* WD_EVENT_OPTION */
    union
    {
        struct
        {
            WD_PCI_ID cardId;
            WD_PCI_SLOT pciSlot;
        } Pci;
        struct
        {
            DWORD dwUniqueID;
        } Usb;
        struct
        {
            WD_PCMCIA_ID deviceId;
            WD_PCMCIA_SLOT slot;
        } Pcmcia;
        struct
        {
            DWORD hIpc;        /* Acts as a unique identifier */
            DWORD dwSubGroupID; /* Might be identical to same process running
                                 * twice (User implementation dependant) */
            DWORD dwGroupID;

            DWORD dwSenderUID;
            DWORD dwMsgID;
            UINT64 qwMsgData;
        } Ipc;
    } u;

    DWORD dwNumMatchTables;
    WDU_MATCH_TABLE matchTables[1];
} WD_EVENT, WD_EVENT_V121;

typedef struct
{
    DWORD applications_num;
    DWORD devices_num;
} WD_USAGE;

enum
{
    WD_USB_HARD_RESET = 1,
    WD_USB_CYCLE_PORT = 2
};

typedef struct
{
    WORD wVendorId;
    WORD wDeviceId;
    WORD wSubVendorId;
    WORD wSubDeviceId;
    DWORD dwOptions; // WD_MATCH_EXCLUDE
} WD_HS_MATCH_TABLE;

typedef struct {
    DWORD handle;
    DWORD dwOptions; // WD_CPCI_PNP_SUPPORT in case we want to use OS PnP
                     // support
    DWORD dwNumMatchTables;
    WD_HS_MATCH_TABLE matchTables[1];
} WD_HS_WATCH, WD_HS_WATCH_V622;

#ifndef BZERO
    #define BZERO(buf) memset(&(buf), 0, sizeof(buf))
#endif

#ifndef INVALID_HANDLE_VALUE
    #if defined(APPLE)
        #define INVALID_HANDLE_VALUE IO_OBJECT_NULL
    #else
        #define INVALID_HANDLE_VALUE ((HANDLE)(-1))
    #endif
#endif

#ifndef CTL_CODE
    #define CTL_CODE(DeviceType, Function, Method, Access) ( \
        ((DeviceType)<<16) | ((Access)<<14) | ((Function)<<2) | (Method) \
    )

    #define METHOD_BUFFERED   0
    #define METHOD_IN_DIRECT  1
    #define METHOD_OUT_DIRECT 2
    #define METHOD_NEITHER    3
    #define FILE_ANY_ACCESS   0
    #define FILE_READ_ACCESS  1    // file & pipe
    #define FILE_WRITE_ACCESS 2    // file & pipe
#endif

#if defined(LINUX) && defined(KERNEL_64BIT)
    #define WD_TYPE 0
    #define WD_CTL_CODE(wFuncNum) \
        _IOC(_IOC_READ|_IOC_WRITE, WD_TYPE, wFuncNum, 0)
    #define WD_CTL_DECODE_FUNC(IoControlCode) _IOC_NR(IoControlCode)
    #define WD_CTL_DECODE_TYPE(IoControlCode) _IOC_TYPE(IoControlCode)
#elif defined(UNIX) || defined(APPLE) /* XXX: Temporarily */
    #define WD_TYPE 0
    #define WD_CTL_CODE(wFuncNum) (wFuncNum)
    #define WD_CTL_DECODE_FUNC(IoControlCode) (IoControlCode)
    #define WD_CTL_DECODE_TYPE(IoControlCode) (WD_TYPE)
#else
    // Device type
    #define WD_TYPE 38200
    #if defined(KERNEL_64BIT)
        #define FUNC_MASK 0x400
    #else
        #define FUNC_MASK 0x0
    #endif
    #define WD_CTL_CODE(wFuncNum) CTL_CODE(WD_TYPE, (wFuncNum | FUNC_MASK), \
        METHOD_NEITHER, FILE_ANY_ACCESS)
    #define WD_CTL_DECODE_FUNC(IoControlCode) ((IoControlCode >> 2) & 0xfff)
    #define WD_CTL_DECODE_TYPE(IoControlCode) \
        DEVICE_TYPE_FROM_CTL_CODE(IoControlCode)
#endif

#if defined(LINUX)
    #define WD_CTL_IS_64BIT_AWARE(IoControlCode) \
        (_IOC_DIR(IoControlCode) & (_IOC_READ|_IOC_WRITE))
#elif defined(UNIX) || defined(APPLE) /* XXX: Temporarily */
    #define WD_CTL_IS_64BIT_AWARE(IoControlCode) TRUE
#else
    #define WD_CTL_IS_64BIT_AWARE(IoControlCode) \
        (WD_CTL_DECODE_FUNC(IoControlCode) & FUNC_MASK)
#endif

// WinDriver function IOCTL calls. For details on the WinDriver functions,
// see the WinDriver manual or included help files.

#define IOCTL_WD_KERNEL_BUF_LOCK          WD_CTL_CODE(0x9f3)
#define IOCTL_WD_KERNEL_BUF_UNLOCK        WD_CTL_CODE(0x9f4)
#define IOCTL_WD_DMA_LOCK                 WD_CTL_CODE(0x9be)
#define IOCTL_WD_DMA_UNLOCK               WD_CTL_CODE(0x902)
#define IOCTL_WD_TRANSFER                 WD_CTL_CODE(0x98c)
#define IOCTL_WD_MULTI_TRANSFER           WD_CTL_CODE(0x98d)
#define IOCTL_WD_PCI_SCAN_CARDS           WD_CTL_CODE(0x9a3)
#define IOCTL_WD_PCI_GET_CARD_INFO        WD_CTL_CODE(0x9e8)
#define IOCTL_WD_VERSION                  WD_CTL_CODE(0x910)
#define IOCTL_WD_PCI_CONFIG_DUMP          WD_CTL_CODE(0x91a)
#define IOCTL_WD_KERNEL_PLUGIN_OPEN       WD_CTL_CODE(0x91b)
#define IOCTL_WD_KERNEL_PLUGIN_CLOSE      WD_CTL_CODE(0x91c)
#define IOCTL_WD_KERNEL_PLUGIN_CALL       WD_CTL_CODE(0x91d)
#define IOCTL_WD_INT_ENABLE               WD_CTL_CODE(0x9b6)
#define IOCTL_WD_INT_DISABLE              WD_CTL_CODE(0x9bb)
#define IOCTL_WD_INT_COUNT                WD_CTL_CODE(0x9ba)
#define IOCTL_WD_ISAPNP_SCAN_CARDS        WD_CTL_CODE(0x924)
#define IOCTL_WD_ISAPNP_CONFIG_DUMP       WD_CTL_CODE(0x926)
#define IOCTL_WD_SLEEP                    WD_CTL_CODE(0x927)
#define IOCTL_WD_DEBUG                    WD_CTL_CODE(0x928)
#define IOCTL_WD_DEBUG_DUMP               WD_CTL_CODE(0x929)
#define IOCTL_WD_CARD_UNREGISTER          WD_CTL_CODE(0x9e7)
#define IOCTL_WD_ISAPNP_GET_CARD_INFO     WD_CTL_CODE(0x9e9)
#define IOCTL_WD_PCMCIA_SCAN_CARDS        WD_CTL_CODE(0x996)
#define IOCTL_WD_PCMCIA_GET_CARD_INFO     WD_CTL_CODE(0x9ea)
#define IOCTL_WD_PCMCIA_CONFIG_DUMP       WD_CTL_CODE(0x998)
#define IOCTL_WD_CARD_REGISTER            WD_CTL_CODE(0x9e6)
#define IOCTL_WD_INT_WAIT                 WD_CTL_CODE(0x9b9)
#define IOCTL_WD_LICENSE                  WD_CTL_CODE(0x9e4)
#define IOCTL_WD_EVENT_REGISTER           WD_CTL_CODE(0x9ef)
#define IOCTL_WD_EVENT_UNREGISTER         WD_CTL_CODE(0x9f0)
#define IOCTL_WD_EVENT_PULL               WD_CTL_CODE(0x9f1)
#define IOCTL_WD_EVENT_SEND               WD_CTL_CODE(0x9f2)
#define IOCTL_WD_DEBUG_ADD                WD_CTL_CODE(0x964)
#define IOCTL_WD_USAGE                    WD_CTL_CODE(0x976)
#define IOCTL_WDU_GET_DEVICE_DATA         WD_CTL_CODE(0x9a7)
#define IOCTL_WDU_SET_INTERFACE           WD_CTL_CODE(0x981)
#define IOCTL_WDU_RESET_PIPE              WD_CTL_CODE(0x982)
#define IOCTL_WDU_TRANSFER                WD_CTL_CODE(0x983)
#define IOCTL_WDU_HALT_TRANSFER           WD_CTL_CODE(0x985)
#define IOCTL_WD_WATCH_PCI_START          WD_CTL_CODE(0x9d6)
#define IOCTL_WD_WATCH_PCI_STOP           WD_CTL_CODE(0x99a)
#define IOCTL_WDU_WAKEUP                  WD_CTL_CODE(0x98a)
#define IOCTL_WDU_RESET_DEVICE            WD_CTL_CODE(0x98b)
#define IOCTL_WD_GET_DEVICE_PROPERTY      WD_CTL_CODE(0x990)
#define IOCTL_WD_CARD_CLEANUP_SETUP       WD_CTL_CODE(0x995)
#define IOCTL_WD_PCMCIA_CONTROL           WD_CTL_CODE(0x99b)
#define IOCTL_WD_DMA_SYNC_CPU             WD_CTL_CODE(0x99f)
#define IOCTL_WD_DMA_SYNC_IO              WD_CTL_CODE(0x9a0)
#define IOCTL_WDU_STREAM_OPEN             WD_CTL_CODE(0x9a8)
#define IOCTL_WDU_STREAM_CLOSE            WD_CTL_CODE(0x9a9)
#define IOCTL_WDU_STREAM_START            WD_CTL_CODE(0x9af)
#define IOCTL_WDU_STREAM_STOP             WD_CTL_CODE(0x9b0)
#define IOCTL_WDU_STREAM_FLUSH            WD_CTL_CODE(0x9aa)
#define IOCTL_WDU_STREAM_GET_STATUS       WD_CTL_CODE(0x9b5)
#define IOCTL_WDU_SELECTIVE_SUSPEND       WD_CTL_CODE(0x9ae)
#define IOCTL_WD_INTERRUPT_DONE_CE        WD_CTL_CODE(0x9bc)
#define IOCTL_WD_PCI_SCAN_CAPS            WD_CTL_CODE(0x9e5)
#define IOCTL_WD_IPC_REGISTER             WD_CTL_CODE(0x9eb)
#define IOCTL_WD_IPC_UNREGISTER           WD_CTL_CODE(0x9ec)
#define IOCTL_WD_IPC_SCAN_PROCS           WD_CTL_CODE(0x9ed)
#define IOCTL_WD_IPC_SEND                 WD_CTL_CODE(0x9ee)

#if defined(UNIX)
    typedef struct
    {
        DWORD dwHeader;
        PVOID pData;
        DWORD dwSize;
    } WD_IOCTL_HEADER;

    #define WD_IOCTL_HEADER_CODE 0xa410b413UL
#endif

#if defined(__KERNEL__)
    HANDLE __cdecl WD_Open(void);
    void __cdecl WD_Close(HANDLE hWD);
    DWORD __cdecl KP_DeviceIoControl(DWORD dwFuncNum, HANDLE h, PVOID pParam,
        DWORD dwSize);
    #define WD_FUNCTION(wFuncNum, h, pParam, dwSize, fWait) \
        KP_DeviceIoControl((DWORD)wFuncNum, h, (PVOID)pParam, (DWORD)dwSize)
#else
    typedef enum
    {
        OS_NOT_SET = 0,
        OS_CAN_NOT_DETECT,
        OS_WIN_NT_4,  /* Windows NT 4.0 */
        OS_WIN_NT_5,  /* Windows 2000, XP, Server 2003 */
        OS_WIN_NT_6,  /* Windows Vista, 7, 8, 8.1 */
        OS_WIN_NT_10, /* Windows 10 */
        OS_LINUX,
        OS_OS2,
        OS_WINCE_4,   /* Windows CE 4 and 5 */
        OS_WINCE_6,   /* Windows CE 6 and 7 */
        OS_WINCE_8,   /* Windows CE 8 */
        OS_APPLE,     /* MAC OS X */
    } OS_TYPE;

    OS_TYPE DLLCALLCONV get_os_type(void);

    #if defined(APPLE)
        DWORD WD_FUNCTION_LOCAL(int wFuncNum, HANDLE h, PVOID pParam,
            DWORD dwSize, BOOL fWait);

        HANDLE WD_OpenLocal(void);

        void WD_CloseLocal(HANDLE h);

        #define WD_OpenStreamLocal(read,sync) INVALID_HADLE_VALUE

        #define WD_UStreamRead(hFile, pBuffer, dwNumberOfBytesToRead, \
            dwNumberOfBytesRead)\
            WD_NOT_IMPLEMENTED

        #define WD_UStreamWrite(hFile, pBuffer, dwNumberOfBytesToWrite, \
            dwNumberOfBytesWritten)\
            WD_NOT_IMPLEMENTED

    #elif defined(UNIX)
        static inline ULONG WD_FUNCTION_LOCAL(int wFuncNum, HANDLE h,
            PVOID pParam, DWORD dwSize, BOOL fWait)
        {
            WD_IOCTL_HEADER ioctl_hdr;

            ioctl_hdr.dwHeader = WD_IOCTL_HEADER_CODE;
            ioctl_hdr.pData = pParam;
            ioctl_hdr.dwSize = dwSize;
            (void)fWait;
            #if defined(LINUX)
                return (ULONG)ioctl((int)(DWORD)h, wFuncNum, &ioctl_hdr);
            #endif
        }

            #define WD_OpenLocal()\
                ((HANDLE)(DWORD)open(WD_DRIVER_NAME, O_RDWR))
            #define WD_OpenStreamLocal(read,sync) \
                ((HANDLE)(DWORD)open(WD_DRIVER_NAME, \
                    ((read) ? O_RDONLY : O_WRONLY) | \
                    ((sync) ? O_SYNC : O_NONBLOCK)))

        #define WD_CloseLocal(h) close((int)(DWORD)(h))

        #define WD_UStreamRead(hFile, pBuffer, dwNumberOfBytesToRead, \
            dwNumberOfBytesRead)\
            WD_NOT_IMPLEMENTED

        #define WD_UStreamWrite(hFile, pBuffer, dwNumberOfBytesToWrite, \
            dwNumberOfBytesWritten)\
            WD_NOT_IMPLEMENTED

    #elif defined(WINCE) && defined(_WIN32_WCE_EMULATION)
        HANDLE WINAPI WCE_EMU_WD_Open();
        void WINAPI WCE_EMU_WD_Close(HANDLE hWD);
        BOOL WINAPI WCE_EMU_WD_FUNCTION(DWORD wFuncNum, HANDLE h, PVOID pParam,
            DWORD dwSize);
        #define WD_OpenLocal() WCE_EMU_WD_Open()
        #define WD_CloseLocal(h) WCE_EMU_WD_Close(h)
        #define WD_FUNCTION_LOCAL(wFuncNum, h, pParam, dwSize, fWait) \
            WCE_EMU_WD_FUNCTION(wFuncNum, h, pParam, dwSize)
    #elif defined(WIN32) || defined(WINCE)
        #define WD_CloseLocal(h) CloseHandle(h)

        #define WD_UStreamRead(hFile, pBuffer, dwNumberOfBytesToRead, \
            dwNumberOfBytesRead)\
            ReadFile(hFile, pBuffer, dwNumberOfBytesToRead, \
                dwNumberOfBytesRead, NULL) ? WD_STATUS_SUCCESS : \
                WD_OPERATION_FAILED

        #define WD_UStreamWrite(hFile, pBuffer, dwNumberOfBytesToWrite, \
            dwNumberOfBytesWritten)\
            WriteFile(hFile, pBuffer, dwNumberOfBytesToWrite, \
                dwNumberOfBytesWritten, NULL) ? WD_STATUS_SUCCESS : \
                WD_OPERATION_FAILED

        #if defined(WINCE)
            void DLLCALLCONV OsSetMaxProcPermissions(void);
            #if defined(WD_CE_ENHANCED_INTR)
                #define InterruptDisable CE_InterruptDisable
                #include <ceddk.h>
                #include <pkfuncs.h>
                #undef InterruptDisable
            #endif

            #define WD_OpenLocal()\
                (OsSetMaxProcPermissions(), CreateFile(\
                    TEXT("WDR1:"),\
                    GENERIC_READ,\
                    FILE_SHARE_READ | FILE_SHARE_WRITE,\
                    NULL, OPEN_EXISTING, 0, NULL))

            #define WD_OpenStreamLocal(read,sync)\
                (OsSetMaxProcPermissions(), CreateFile(\
                    TEXT("WDR1:"),\
                    (read) ? GENERIC_READ : GENERIC_WRITE,\
                    FILE_SHARE_READ | FILE_SHARE_WRITE,\
                    NULL, OPEN_EXISTING, (sync) ? 0 : FILE_FLAG_OVERLAPPED,\
                    NULL))

            #define WD_CE_PRIORITY(prio)\
                (get_os_type() >= OS_WINCE_6 ? WD_CE6_PRIORITY(prio) : prio)

            static DWORD WD_FUNCTION_LOCAL(int wFuncNum, HANDLE h, PVOID pParam,
                DWORD dwSize, BOOL fWait)
            {
                DWORD dwTmp;
                DWORD rc = WD_WINDRIVER_STATUS_ERROR;

                (DWORD)DeviceIoControl(h, (DWORD)wFuncNum, pParam, dwSize, &rc,
                    sizeof(DWORD), &dwTmp, NULL);

                return rc;
            }
        #elif defined(WIN32)
            #define WD_OpenLocal()\
                CreateFileA(\
                    WD_DRIVER_NAME,\
                    GENERIC_READ,\
                    FILE_SHARE_READ | FILE_SHARE_WRITE,\
                    NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL)

            #define WD_OpenStreamLocal(read,sync) \
                CreateFileA(\
                    WD_DRIVER_NAME,\
                    (read) ? GENERIC_READ : GENERIC_WRITE,\
                    FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, \
                    (sync) ? 0 : FILE_FLAG_OVERLAPPED, NULL)

            static DWORD WD_FUNCTION_LOCAL(int wFuncNum, HANDLE h, PVOID pParam,
                DWORD dwSize, BOOL fWait)
            {
                DWORD dwTmp;
                HANDLE hWD = fWait ? WD_OpenLocal() : h;
                DWORD rc = WD_WINDRIVER_STATUS_ERROR;

                if (hWD == INVALID_HANDLE_VALUE)
                    return (DWORD)WD_STATUS_INVALID_WD_HANDLE;

                DeviceIoControl(hWD, (DWORD)wFuncNum, pParam, dwSize, &rc,
                    sizeof(DWORD), &dwTmp, NULL);

                if (fWait)
                    WD_CloseLocal(hWD);

                return rc;
            }
        #endif
    #endif

    #define WD_FUNCTION WD_FUNCTION_LOCAL
    #define WD_Close WD_CloseLocal
    #define WD_Open WD_OpenLocal
    #define WD_StreamOpen WD_OpenStreamLocal
    #define WD_StreamClose WD_CloseLocal
#endif

#define SIZE_OF_WD_DMA(pDma) \
    ((DWORD)(sizeof(WD_DMA) + ((pDma)->dwPages <= WD_DMA_PAGES ? \
        0 : ((pDma)->dwPages - WD_DMA_PAGES) * sizeof(WD_DMA_PAGE))))
#define SIZE_OF_WD_EVENT(pEvent) \
    ((DWORD)(sizeof(WD_EVENT) + sizeof(WDU_MATCH_TABLE) * \
        ((pEvent)->dwNumMatchTables - 1)))
#define SIZE_OF_WD_HS_WATCH(pHsWatch) \
    ((DWORD)(sizeof(WD_HS_WATCH) + sizeof(WD_HS_MATCH_TABLE) * \
        ((pHsWatch)->dwNumMatchTables - 1)))

#define WD_Debug(h,pDebug)\
    WD_FUNCTION(IOCTL_WD_DEBUG, h, pDebug, sizeof(WD_DEBUG), FALSE)
#define WD_DebugDump(h,pDebugDump)\
    WD_FUNCTION(IOCTL_WD_DEBUG_DUMP, h, pDebugDump, sizeof(WD_DEBUG_DUMP), \
        FALSE)
#define WD_DebugAdd(h, pDebugAdd)\
    WD_FUNCTION(IOCTL_WD_DEBUG_ADD, h, pDebugAdd, sizeof(WD_DEBUG_ADD), FALSE)
#define WD_Transfer(h,pTransfer)\
    WD_FUNCTION(IOCTL_WD_TRANSFER, h, pTransfer, sizeof(WD_TRANSFER), FALSE)
#define WD_MultiTransfer(h, pTransferArray, dwNumTransfers) \
    WD_FUNCTION(IOCTL_WD_MULTI_TRANSFER, h, pTransferArray, \
        sizeof(WD_TRANSFER) * (dwNumTransfers), FALSE)
#define WD_KernelBufLock(h, pKerBuf)\
    WD_FUNCTION(IOCTL_WD_KERNEL_BUF_LOCK, h, pKerBuf, \
        sizeof(WD_KERNEL_BUFFER), FALSE)
#define WD_KernelBufUnlock(h, pKerBuf)\
    WD_FUNCTION(IOCTL_WD_KERNEL_BUF_UNLOCK, h, pKerBuf, \
        sizeof(WD_KERNEL_BUFFER), FALSE)
#define WD_DMALock(h,pDma)\
    WD_FUNCTION(IOCTL_WD_DMA_LOCK, h, pDma, SIZE_OF_WD_DMA(pDma), FALSE)
#define WD_DMAUnlock(h,pDma)\
    WD_FUNCTION(IOCTL_WD_DMA_UNLOCK, h, pDma, SIZE_OF_WD_DMA(pDma), FALSE)
#define WD_DMASyncCpu(h,pDma)\
    WD_FUNCTION(IOCTL_WD_DMA_SYNC_CPU, h, pDma, SIZE_OF_WD_DMA(pDma), FALSE)
#define WD_DMASyncIo(h,pDma)\
    WD_FUNCTION(IOCTL_WD_DMA_SYNC_IO, h, pDma, SIZE_OF_WD_DMA(pDma), FALSE)
#define WD_CardRegister(h,pCard)\
    WD_FUNCTION(IOCTL_WD_CARD_REGISTER, h, pCard, sizeof(WD_CARD_REGISTER),\
        FALSE)
#define WD_CardUnregister(h,pCard)\
    WD_FUNCTION(IOCTL_WD_CARD_UNREGISTER, h, pCard, sizeof(WD_CARD_REGISTER),\
        FALSE)
#define WD_IpcRegister(h, pIpcRegister) \
    WD_FUNCTION(IOCTL_WD_IPC_REGISTER, h, pIpcRegister, \
        sizeof(WD_IPC_REGISTER), FALSE)
#define WD_IpcUnRegister(h, pProcInfo) \
    WD_FUNCTION(IOCTL_WD_IPC_UNREGISTER, h, pProcInfo, sizeof(WD_IPC_PROCESS), \
        FALSE)
#define WD_IpcScanProcs(h, pIpcScanProcs) \
    WD_FUNCTION(IOCTL_WD_IPC_SCAN_PROCS, h, pIpcScanProcs, \
        sizeof(WD_IPC_SCAN_PROCS), FALSE)
#define WD_IpcSend(h, pIpcSend) \
    WD_FUNCTION(IOCTL_WD_IPC_SEND, h, pIpcSend, sizeof(WD_IPC_SEND), FALSE)
#define WD_CardCleanupSetup(h,pCardCleanup)\
    WD_FUNCTION(IOCTL_WD_CARD_CLEANUP_SETUP, h, pCardCleanup, \
        sizeof(WD_CARD_CLEANUP), FALSE)
#define WD_PciScanCards(h,pPciScan)\
    WD_FUNCTION(IOCTL_WD_PCI_SCAN_CARDS, h, pPciScan,\
        sizeof(WD_PCI_SCAN_CARDS), FALSE)
#define WD_PciScanCaps(h,pPciScanCaps) \
    WD_FUNCTION(IOCTL_WD_PCI_SCAN_CAPS, h, pPciScanCaps, \
        sizeof(WD_PCI_SCAN_CAPS), FALSE)
#define WD_PciGetCardInfo(h,pPciCard)\
    WD_FUNCTION(IOCTL_WD_PCI_GET_CARD_INFO, h, pPciCard, \
        sizeof(WD_PCI_CARD_INFO), FALSE)
#define WD_PciConfigDump(h,pPciConfigDump)\
    WD_FUNCTION(IOCTL_WD_PCI_CONFIG_DUMP, h, pPciConfigDump, \
        sizeof(WD_PCI_CONFIG_DUMP), FALSE)
#define WD_Version(h,pVerInfo)\
    WD_FUNCTION(IOCTL_WD_VERSION, h, pVerInfo, sizeof(WD_VERSION), FALSE)
#define WD_License(h,pLicense)\
    WD_FUNCTION(IOCTL_WD_LICENSE, h, pLicense, sizeof(WD_LICENSE), FALSE)
#define WD_KernelPlugInOpen(h,pKernelPlugIn)\
    WD_FUNCTION(IOCTL_WD_KERNEL_PLUGIN_OPEN, h, pKernelPlugIn, \
        sizeof(WD_KERNEL_PLUGIN), FALSE)
#define WD_KernelPlugInClose(h,pKernelPlugIn)\
    WD_FUNCTION(IOCTL_WD_KERNEL_PLUGIN_CLOSE, h, pKernelPlugIn, \
        sizeof(WD_KERNEL_PLUGIN), FALSE)
#define WD_KernelPlugInCall(h,pKernelPlugInCall)\
    WD_FUNCTION(IOCTL_WD_KERNEL_PLUGIN_CALL, h, pKernelPlugInCall, \
        sizeof(WD_KERNEL_PLUGIN_CALL), FALSE)
#define WD_IntEnable(h,pInterrupt)\
    WD_FUNCTION(IOCTL_WD_INT_ENABLE, h, pInterrupt, sizeof(WD_INTERRUPT), FALSE)
#define WD_IntDisable(h,pInterrupt)\
    WD_FUNCTION(IOCTL_WD_INT_DISABLE, h, pInterrupt, sizeof(WD_INTERRUPT), \
        FALSE)
#define WD_IntCount(h,pInterrupt)\
    WD_FUNCTION(IOCTL_WD_INT_COUNT, h, pInterrupt, sizeof(WD_INTERRUPT), FALSE)
#define WD_IntWait(h,pInterrupt)\
    WD_FUNCTION(IOCTL_WD_INT_WAIT, h, pInterrupt, sizeof(WD_INTERRUPT), TRUE)
#define WD_IsapnpScanCards(h,pIsapnpScan)\
    WD_FUNCTION(IOCTL_WD_ISAPNP_SCAN_CARDS, h, pIsapnpScan, \
        sizeof(WD_ISAPNP_SCAN_CARDS), FALSE)
#define WD_IsapnpGetCardInfo(h,pIsapnpCard)\
    WD_FUNCTION(IOCTL_WD_ISAPNP_GET_CARD_INFO, h, pIsapnpCard, \
        sizeof(WD_ISAPNP_CARD_INFO), FALSE)
#define WD_IsapnpConfigDump(h,pIsapnpConfigDump)\
    WD_FUNCTION(IOCTL_WD_ISAPNP_CONFIG_DUMP, h, pIsapnpConfigDump, \
        sizeof(WD_ISAPNP_CONFIG_DUMP), FALSE)
#define WD_PcmciaScanCards(h,pPcmciaScan)\
    WD_FUNCTION(IOCTL_WD_PCMCIA_SCAN_CARDS, h, pPcmciaScan, \
        sizeof(WD_PCMCIA_SCAN_CARDS), FALSE)
#define WD_PcmciaGetCardInfo(h,pPcmciaCard)\
    WD_FUNCTION(IOCTL_WD_PCMCIA_GET_CARD_INFO, h, pPcmciaCard, \
        sizeof(WD_PCMCIA_CARD_INFO), FALSE)
#define WD_PcmciaConfigDump(h,pPcmciaConfigDump)\
    WD_FUNCTION(IOCTL_WD_PCMCIA_CONFIG_DUMP, h, pPcmciaConfigDump, \
        sizeof(WD_PCMCIA_CONFIG_DUMP), FALSE)
#define WD_PcmciaControl(h,pPcmciaControl)\
    WD_FUNCTION(IOCTL_WD_PCMCIA_CONTROL, h, pPcmciaControl, \
        sizeof(WD_PCMCIA_CONTROL), FALSE)
#define WD_Sleep(h,pSleep)\
    WD_FUNCTION(IOCTL_WD_SLEEP, h, pSleep, sizeof(WD_SLEEP), FALSE)
#define WD_EventRegister(h, pEvent) \
    WD_FUNCTION(IOCTL_WD_EVENT_REGISTER, h, pEvent, SIZE_OF_WD_EVENT(pEvent), \
        FALSE)
#define WD_EventUnregister(h, pEvent) \
    WD_FUNCTION(IOCTL_WD_EVENT_UNREGISTER, h, pEvent, \
        SIZE_OF_WD_EVENT(pEvent), FALSE)
#define WD_EventPull(h,pEvent) \
    WD_FUNCTION(IOCTL_WD_EVENT_PULL, h, pEvent, SIZE_OF_WD_EVENT(pEvent), FALSE)
#define WD_EventSend(h,pEvent) \
    WD_FUNCTION(IOCTL_WD_EVENT_SEND, h, pEvent, SIZE_OF_WD_EVENT(pEvent), FALSE)
#define WD_Usage(h, pStop) \
    WD_FUNCTION(IOCTL_WD_USAGE, h, pStop, sizeof(WD_USAGE), FALSE)

#define WD_UGetDeviceData(h, pGetDevData) \
    WD_FUNCTION(IOCTL_WDU_GET_DEVICE_DATA, h, pGetDevData, \
        sizeof(WDU_GET_DEVICE_DATA), FALSE);
#define WD_GetDeviceProperty(h, pGetDevProperty) \
    WD_FUNCTION(IOCTL_WD_GET_DEVICE_PROPERTY, h, pGetDevProperty, \
        sizeof(WD_GET_DEVICE_PROPERTY), FALSE);
#define WD_USetInterface(h, pSetIfc) \
    WD_FUNCTION(IOCTL_WDU_SET_INTERFACE, h, pSetIfc, \
        sizeof(WDU_SET_INTERFACE), FALSE);
#define WD_UResetPipe(h, pResetPipe) \
    WD_FUNCTION(IOCTL_WDU_RESET_PIPE, h, pResetPipe, sizeof(WDU_RESET_PIPE), \
        FALSE);
#define WD_UTransfer(h, pTrans) \
    WD_FUNCTION(IOCTL_WDU_TRANSFER, h, pTrans, sizeof(WDU_TRANSFER), TRUE);
#define WD_UHaltTransfer(h, pHaltTrans) \
    WD_FUNCTION(IOCTL_WDU_HALT_TRANSFER, h, pHaltTrans, \
        sizeof(WDU_HALT_TRANSFER), FALSE);
#define WD_WatchPciStart(h, pHsWatch)\
    WD_FUNCTION(IOCTL_WD_WATCH_PCI_START, h, pHsWatch, \
        SIZE_OF_WD_HS_WATCH(pHsWatch), FALSE)
#define WD_WatchPciStop(h, pHsWatch)\
    WD_FUNCTION(IOCTL_WD_WATCH_PCI_STOP, h, pHsWatch, \
        SIZE_OF_WD_HS_WATCH(pHsWatch), FALSE)
#define WD_UWakeup(h, pWakeup) \
    WD_FUNCTION(IOCTL_WDU_WAKEUP, h, pWakeup, sizeof(WDU_WAKEUP), FALSE);
#define WD_USelectiveSuspend(h, pSelectiveSuspend) \
    WD_FUNCTION(IOCTL_WDU_SELECTIVE_SUSPEND, h, pSelectiveSuspend, \
        sizeof(WDU_SELECTIVE_SUSPEND), FALSE);
#define WD_UResetDevice(h, pResetDevice) \
    WD_FUNCTION(IOCTL_WDU_RESET_DEVICE, h, pResetDevice, \
        sizeof(WDU_RESET_DEVICE), FALSE);
#define WD_UStreamOpen(h, pStream) \
    WD_FUNCTION(IOCTL_WDU_STREAM_OPEN, h, pStream, sizeof(WDU_STREAM), FALSE);
#define WD_UStreamClose(h, pStream) \
    WD_FUNCTION(IOCTL_WDU_STREAM_CLOSE, h, pStream, sizeof(WDU_STREAM), FALSE);
#define WD_UStreamStart(h, pStream) \
    WD_FUNCTION(IOCTL_WDU_STREAM_START, h, pStream, sizeof(WDU_STREAM), FALSE);
#define WD_UStreamStop(h, pStream) \
    WD_FUNCTION(IOCTL_WDU_STREAM_STOP, h, pStream, sizeof(WDU_STREAM), FALSE);
#define WD_UStreamFlush(h, pStream) \
    WD_FUNCTION(IOCTL_WDU_STREAM_FLUSH, h, pStream, sizeof(WDU_STREAM), FALSE);
#define WD_UStreamGetStatus(h, pStreamStatus) \
    WD_FUNCTION(IOCTL_WDU_STREAM_GET_STATUS, h, pStreamStatus, \
        sizeof(WDU_STREAM_STATUS), FALSE);
#define WD_InterruptDoneCe(h, pInt) \
    WD_FUNCTION(IOCTL_WD_INTERRUPT_DONE_CE, h, pInt, sizeof(WD_INTERRUPT), \
        FALSE);

#define __ALIGN_DOWN(val,alignment) ( (val) & ~((alignment) - 1) )
#define __ALIGN_UP(val,alignment) \
    ( ((val) + (alignment) - 1) & ~((alignment) - 1) )

#ifdef WDLOG
    #include "wd_log.h"
#endif

#ifndef MIN
    #define MIN(a,b) ((a) > (b) ? (b) : (a))
#endif
#ifndef MAX
    #define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif
#define SAFE_STRING(s) ((s) ? (s) : "")

#if defined(LINUX) && defined(PPC64)
    #pragma pack(pop)
#endif

#ifdef __cplusplus
}
#endif

#endif /* _WINDRVR_H_ */

