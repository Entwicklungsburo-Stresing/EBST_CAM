#pragma once
// CCDCTL.h    defines the Interface between the application and the driver
// is used for LSCISA and LSCPCI version
//
// Define the IOCTL codes we will use.  The IOCTL code contains a command
// identifier, plus other information about the device, the type of access
// with which the file must have been opened, and the type of buffering.
//
// EB G. Stresing		10/2001

// copied from devioctl.h
// Macro definition for defining IOCTL and FSCTL function control codes.  Note
// that function codes 0-2047 are reserved for Microsoft Corporation, and
// 2048-4095 are reserved for customers.
//

#include <Windows.h>

#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)

//
// Define the method codes for how buffers are passed for I/O and FS controls
//

#define METHOD_BUFFERED                 0
#define METHOD_IN_DIRECT                1
#define METHOD_OUT_DIRECT               2
#define METHOD_NEITHER                  3

#define FILE_ANY_ACCESS                 0
#define FILE_READ_ACCESS          ( 0x0001 )    // file & pipe
#define FILE_WRITE_ACCESS         ( 0x0002 )    // file & pipe



// Device type           -- in the "User Defined" range."
#define GPD_TYPE 40000

// The IOCTL function codes from 0x800 to 0xFFF are for customer use.

	// get software version of driver
#define IOCTL_GetVersion \
    CTL_CODE( GPD_TYPE, 0x900, METHOD_BUFFERED, FILE_READ_ACCESS )
	// gets byte of PCI: space0 / ISA: Portoffset
#define IOCTL_ReadByteS0\
    CTL_CODE( GPD_TYPE, 0x901, METHOD_BUFFERED, FILE_READ_ACCESS )
	//read camera data to array
#define IOCTL_GetCCD \
    CTL_CODE( GPD_TYPE, 0x902, METHOD_IN_DIRECT, FILE_READ_ACCESS ) //!direct
	// read long in space0
#define IOCTL_ReadLongS0\
    CTL_CODE( GPD_TYPE, 0x903, METHOD_BUFFERED, FILE_READ_ACCESS )
	// gets long from Port of IORunRegs of PCI board
#define IOCTL_ReadLongIORunReg \
    CTL_CODE( GPD_TYPE, 0x904, METHOD_BUFFERED, FILE_READ_ACCESS )
	// read key input from port 0x60
#define IOCTL_ReadKey \
    CTL_CODE( GPD_TYPE, 0x905, METHOD_BUFFERED, FILE_READ_ACCESS )
	// initialize PCI board and get addresses, de-/ activate mouse
#define IOCTL_SetFct \
    CTL_CODE( GPD_TYPE, 0x906, METHOD_BUFFERED, FILE_READ_ACCESS )
#define IOCTL_ReadByteIORunReg \
    CTL_CODE( GPD_TYPE, 0x907, METHOD_BUFFERED, FILE_READ_ACCESS )

#define IOCTL_WriteByteS0\
    CTL_CODE( GPD_TYPE, 0x911, METHOD_BUFFERED, FILE_WRITE_ACCESS )
	// gets byte of PCI: space0 / ISA: Portoffset
#define IOCTL_WriteLongIORunReg \
    CTL_CODE(GPD_TYPE,  0x912, METHOD_BUFFERED, FILE_WRITE_ACCESS)
	// write long to Port of RunReg PCI board
#define IOCTL_WriteLongS0 \
    CTL_CODE(GPD_TYPE,  0x913, METHOD_BUFFERED, FILE_WRITE_ACCESS)
	// write long to space0 address
#define IOCTL_WriteByteIORunReg \
    CTL_CODE(GPD_TYPE,  0x914, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_WriteBytePort0 \
    CTL_CODE(GPD_TYPE,  0x915, METHOD_BUFFERED, FILE_WRITE_ACCESS)

typedef struct  _sDLDATA {
    ULONG   POff;     // write 2 longs to driver
    ULONG   Data;
}   sDLDATA;


typedef struct _sCCDFkts{  // struct for passing parameters to GETCCD

	ULONG NoOfLines; // lines of a blockread
	ULONG ADFlag;  // no function for PCI 
	ULONG Adrwaits; // Adress delay for FFTs -> 6mu
	ULONG Waits;	// PCI: VCLK waits for FFTs -> 3mu
	ULONG Vclks; 
	ULONG Fkt;
	ULONG Zadr;

} sCCDFkts, *PsCCDFkts;


//Errorcodes of PCI vxd driver
//are mostly not used in sys driver
#define NoError					0
#define Error_notinitiated		1 
#define Error_noregkey			2	//no registry key
#define Error_nosubregkey		3	//no registry sub key
#define Error_nobufspace		4
#define Error_nobios			5	//no PCI bios 
#define Error_noboard			6	//no Interfaceboard
#define Error_noIORegBase		7	//no PCI runtime address
#define Error_Physnotmapped		8
#define Error_Fktnotimplemented	9	//in SetBoard
#define Error_Timer				10	//PCI board Timer for vclks
#define Error_Mouse				11	//Mouse Act/Deact Error


/* Default PCI ven and dev ID */
#define Def_VenID       0x10b59056			// if not found in registry
#define Def_SubVenID	0x45537401          // my is 45537401 for first board
											// 45537402 for second and so on
#define Def_MaxDMABuf	32000				// max dma buffer length in bytes
