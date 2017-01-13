//   UNIT BOARD.C for all examples equal
//	PCIE version with DMA and INTR
//	V1.00  GS  6/2016



//#include "stdafx.h"		// use in C++ only
//#include "global.h"		// use in C++ only
#include "ccdctl.h" //"ccdctrl.h"
#include "board.h"
#include <limits.h>
#include <process.h>
#include "windrvr.h"
#include "wdc_lib.h"
#include "wdc_defs.h"
//#include "wd_kp.h"
//siehe beginn functions
//#include "lscpciej_lib.h" 
/*************************************************************
Internal definitions
*************************************************************/
/* WinDriver license registration string */

//Dont trust the debugger its CRAP

***REMOVED******REMOVED***#define LSCPCIEJ_DEFAULT_DRIVER_NAME WD_DEFAULT_DRIVER_NAME_BASE

/*************************************************************
General definitions
***************************************************readloop
**********/
/* Error messages display */
#define LSCPCIEJ_ERR printf

#define _CNT255 TRUE   //TRUE if FIFO version has 8bit Counter / TRUE is default

// use LSCPCI1 on PCI Boards
#define	DRIVERNAME	"\\\\.\\LSCPCIE"

// globals within board
// don't change values here - are set within functions SetBoardVars...

//DMA Addresses
enum{
	DmaAddr_DCSR = 0x000,
	DmaAddr_DDMACR = 0x004,
	DmaAddr_WDMATLPA = 0x008,
	DmaAddr_WDMATLPS = 0x00C,
	DmaAddr_WDMATLPC = 0x010,
	DmaAddr_WDMATLPP = 0x014,
	DmaAddr_RDMATLPP = 0x018,
	DmaAddr_RDMATLPA = 0x01C,
	DmaAddr_RDMATLPS = 0x020,
	DmaAddr_RDMATLPC = 0x024,
	DmaAddr_DmaBufSize = 0x04C,
	DmaAddr_ScansPerIntr = 0x050
};

//jungodriver specific variables
WD_PCI_CARD_INFO deviceInfo;
WDC_DEVICE_HANDLE hDev = NULL;
ULONG DMACounter = 0;//for debugging
//Buffer of WDC_DMAContigBufLock function = one DMA sub block - will be copied to the big pDMABigBuf later
USHORT *pDMASubBuf;
//PUSHORT *pDMASubBuf;  //!!GS
WD_DMA *pDMASubBufInfos = NULL; //there will be saved the neccesary parameters for the dma buffer
BOOL DMAAlreadyStarted = FALSE;
DWORD dwDMASubBufSize;
char ISRCounter = 0;//!!GS
SHORT DMAUserBufIndex = 0;
//BYTE DontReadUserBufIndex = 0;

// handle array for our drivers
HANDLE ahCCDDRV[5] = { INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE };

ULONG aWAITS[5] = { 0, 0, 0, 0, 0 };	// waitstates
ULONG aFLAG816[5] = { 1, 1, 1, 1, 1 };  //AD-Flag
ULONG aPIXEL[5] = { 0, 0, 0, 0, 0 };	// pixel
ULONG aXCKDelay[5] = { 1000, 1000, 1000, 1000, 1000 };	// sensor specific delay
BOOL aINIT[5] = { FALSE, FALSE, FALSE, FALSE, FALSE };


//globals for Ring buffer
pArrayT pRingBuf = NULL;
ULONG RingFifoDepth = 0;
volatile INT32 RingWRCnt = 0;
ULONG RingRDCnt = 0;
volatile BOOL RingThreadOn = FALSE;
volatile BOOL RingThreadOFF = FALSE;
ULONG Ringdrvno = 1;
volatile ULONG MaxLineCounter = 0;
volatile UINT64 MaxISRTime = 0;
volatile PUCHAR pUserBuf = NULL;
volatile BOOL UserBufValid = FALSE;
volatile INT32 RingCopyStop = 0;
volatile INT32 RingCopyRange = 0;
volatile ULONG RingFirstRun = 0;
volatile BOOL RingCopyAct = FALSE;
volatile BOOL RingFetchFutureAct = FALSE;
volatile INT32 RingFutureWrCnt = 0;
UCHAR RingCtrlReg = 0;
volatile ULONG RingCtrlRegOfs = 0;

ULONG DELAY = 0x100;
ULONG VFREQ = Vfreqini;
ULONG FFTLINES = 0;
BOOL HA_MODULE = FALSE;
BOOL HA_IRSingleCH = FALSE;
//defined as global for ReadRingThread no FIFO version
ULONG RRT_FftLines = 0;
BOOL RRT_ExtTrigFlag = FALSE;
ULONG ErrCnt = 0;
ULONG ErrVal = 0;

//global for ANDANTA ROI function in testfunction
#if (_TESTRUP)
ULONG Roilines = 256;
#endif
//priority globals
ULONG NEWPRICLASS = 0;
ULONG NEWPRILEVEL = 0;
ULONG READTHREADPriority = 15;
ULONG OLDTHREADLEVEL = 0;
ULONG OLDPRICLASS = 0;
HANDLE hPROCESS = 0;
HANDLE hTHREAD = 0;
__int16 RELEASETHREADms = 0;

//general switch to suppress ErrorMsg windows , global in BOARD
BOOL _SHOW_MSG = TRUE;


BOOL OneRingBufShot = FALSE;

// ***********     functions    **********************
//B! FIXME die jungo-library für fehler und co müsste wahrscheinlich in BOARD.C reingeschrieben werden 
#include "lscpciej_lib.c"
//#include "kp_lscpciej.c"

void ErrMsgBoxOn(void)
{//general switch to suppress error mesage box
	_SHOW_MSG = TRUE;
}

void ErrMsgBoxOff(void)
{//general switch to suppress error mesage box
	_SHOW_MSG = FALSE;
}

void ErrorMsg(char ErrMsg[40])
{
	if (_SHOW_MSG)
	{
		if (MessageBox(GetActiveWindow(), ErrMsg, "ERROR", MB_OK | MB_ICONEXCLAMATION) == IDOK) {};
	}
};

void ValMsg(long val)
{
	char AString[40];
	if (_SHOW_MSG)
	{
		sprintf_s(AString, 40, "%s%d", "val= ", val);
		if (MessageBox(GetActiveWindow(), AString, "ERROR", MB_OK | MB_ICONEXCLAMATION) == IDOK) {};
	}
};

//done
BOOL CCDDrvInit(UINT drvno)
{// returns true if driver was found
	ULONG MAXDMABUFLENGTH = 0x07fff; //val look in registry driver parameters
	//depends on os, how big a buffer can be
	BOOL fResult = FALSE;
	char AString[80] = "";
	HANDLE hccddrv = INVALID_HANDLE_VALUE;

	if ((drvno < 1) || (drvno>4)) return FALSE;

	if ((ULONG)_PIXEL > (ULONG)(MAXDMABUFLENGTH / 4))
	{
		ErrorMsg("DMA Buffer length > 0x7fff/4 -> need special driver!");
		return FALSE;
	}


	PWDC_DEVICE pDev = (PWDC_DEVICE)hDev;
	volatile DWORD dwStatus = 0;
	WDC_PCI_SCAN_RESULT scanResult;
	PLSCPCIEJ_DEV_CTX pDevCtx = NULL;

	LSCPCIEJ_DEV_ADDR_DESC devAddrDesc;

#if defined(WD_DRIVER_NAME_CHANGE)
	/* Set the driver name */

	if (!WD_DriverName(LSCPCIEJ_DEFAULT_DRIVER_NAME))
	{
		ErrLog("Failed to set the driver name for WDC library.\n");
		return WD_SYSTEM_INTERNAL_ERROR;
	}

#endif

	/* Initialize the LSCPCIEJ library
	dwStatus = LSCPCIEJ_LibInit();
	if (WD_STATUS_SUCCESS != dwStatus)
	{
	LSCPCIEJ_ERR("lscpciej_diag: Failed to initialize the LSCPCIEJ library: %s",
	LSCPCIEJ_GetLastErr());
	return dwStatus;
	}*/


	/* Set WDC library's debug options (default: level TRACE, output to Debug Monitor) */
	dwStatus = WDC_SetDebugOptions(WDC_DBG_DEFAULT, NULL);
	if (WD_STATUS_SUCCESS != dwStatus)
	{
		ErrLog("Failed to initialize debug options for WDC library.\n"
			"Error 0x%lx - %s\n", dwStatus, Stat2Str(dwStatus));
		WDC_Err("%s", LSCPCIEJ_GetLastErr());
		return FALSE;
	}

	/* Open a handle to the driver and initialize the WDC library */
***REMOVED***	if (WD_STATUS_SUCCESS != dwStatus)
	{
		ErrLog("Failed to initialize the WDC library. Error 0x%lx - %s\n",
			dwStatus, Stat2Str(dwStatus));
		//doesnt work at this moment before debugsetup
		//WDC_Err("%s", LSCPCIEJ_GetLastErr());
		return FALSE;
	}

	BZERO(scanResult);
	dwStatus = WDC_PciScanDevices(LSCPCIEJ_DEFAULT_VENDOR_ID, LSCPCIEJ_DEFAULT_DEVICE_ID, &scanResult); //VendorID, DeviceID
	if (WD_STATUS_SUCCESS != dwStatus)
	{

		ErrLog("DeviceFind: Failed scanning the PCI bus.\n"
			"Error: 0x%lx - %s\n", dwStatus, Stat2Str(dwStatus));
		WDC_Err("%s", LSCPCIEJ_GetLastErr());
		return FALSE;
	}



	//gives the information received from PciScanDevices to PciGetDeviceInfo
	BZERO(deviceInfo);
	memcpy(&deviceInfo.pciSlot, &scanResult.deviceSlot[drvno - 1], sizeof(deviceInfo.pciSlot));
	//deviceInfo.pciSlot = scanResult.deviceSlot[0];

	/* Retrieve the device's resources information */

	dwStatus = WDC_PciGetDeviceInfo(&deviceInfo);
	if (WD_STATUS_SUCCESS != dwStatus)
	{
		ErrLog("DeviceOpen: Failed retrieving the device's resources information.\n"
			"Error 0x%lx - %s\n", dwStatus, Stat2Str(dwStatus));
		WDC_Err("%s", LSCPCIEJ_GetLastErr());
		return FALSE;
	}

	hDev = LSCPCIEJ_DeviceOpen(&deviceInfo);
	if (!hDev)
	{
		LSCPCIEJ_ERR("DeviceOpen: Failed opening a handle to the device: %s",
			LSCPCIEJ_GetLastErr());
		return NULL;
	}

	pDev = ((PWDC_DEVICE)hDev);
	WDC_Err("DRVInit hDev id % x\n DRVInit hDev pci slot %x\n DRVInit hDev pci bus %x\n DRVInit hDev pci function %x\n DRVInit hDevNumAddrSp %x "
		, pDev->id, pDev->slot.pciSlot.dwSlot, pDev->slot.pciSlot.dwBus, pDev->slot.pciSlot.dwFunction, pDev->dwNumAddrSpaces);

	/*hccddrv = WDC_GetWDHandle();
	if (hccddrv == (HANDLE)INVALID_HANDLE_VALUE)
	{
	WDC_Err("createHandle failed");
	return FALSE;
	}*/	// false if LSCPCIn not there 
	//save handle in global array
	ahCCDDRV[drvno] = hDev;//hccddrv;


	//for testing
	KP_LSCPCIEJ_VERSION Data;
	KP_LSCPCIEJ_VERSION *pData;
	pData = &Data;
	DWORD dwResult;
	PDWORD	pdwResult = &dwResult;
	/*
	WDC_CallKerPlug(hDev, KP_LSCPCIEJ_MSG_VERSION, pData, pdwResult);
	if (*pdwResult != KP_LSCPCIEJ_STATUS_OK)
	{
	ErrLog("KP_CALL failed!!!.\n"
	"Error 0x%lx - %s\n", dwStatus, Stat2Str(dwStatus));
	WDC_Err("%s", LSCPCIEJ_GetLastErr());
	return FALSE;
	}
	WDC_Err("KPCALL: Version: %u \n", Data.dwVer);
	*/
	//SetupPCIE_DMA(drvno);
	//StartPCIE_DMAWrite(drvno);



	WDC_Err("finished DRVInit\n");
	return TRUE;	  // no Error, driver found

}; //CCDDrvInit


void CCDDrvExit(UINT drvno)
{
	WDC_DriverClose();
	WDC_PciDeviceClose(hDev);
	WDC_Err("Driver closed and PciDeviceClosed \n");
	//if (ahCCDDRV[drvno]!=INVALID_HANDLE_VALUE)
	//CloseHandle(ahCCDDRV[drvno]);	   // close driver
	ahCCDDRV[drvno] = INVALID_HANDLE_VALUE;
	aINIT[drvno] = FALSE;
};

//no needed after jungo???
BOOL InitBoard(UINT drvno)
{		// inits PCI Board and gets all needed addresses
	// and gets Errorcode if any
	ULONG Errorcode = Error_notinitiated;
	BOOL fResult = FALSE;
	DWORD   ReturnedLength;
	ULONG	ctrlcode = 1; // Init Board

	/*
	fResult = DeviceIoControl(ahCCDDRV[drvno], IOCTL_SetFct,  // read error code
		&ctrlcode,        // Buffer to driver.
		sizeof(ctrlcode),
		&Errorcode, sizeof(Errorcode), &ReturnedLength, NULL);
	if (!fResult)
	{
		ErrorMsg("InitBoard failed");
	};

	// these error messages are for inspection of the driver only
	// they can be omitted because here should be no error
	if (Errorcode == NoError)
	{
		return TRUE;
	}  // everything went fine

	else   switch (Errorcode)
	{
	case Error_notinitiated: ErrorMsg("CCD Board not initialized");
		break;
	case Error_noregkey: ErrorMsg(" No registry entry found ");
		break;
	case Error_nosubregkey: ErrorMsg(" No registry sub entry found ");
		break;
	case Error_nobufspace: ErrorMsg(" Can't init buffer space ");
		break;
	case Error_nobios: ErrorMsg(" No PCI bios found ");
		break;
	case Error_noboard: ErrorMsg(" Can't find CCD Board ");
		break;
	case Error_noIORegBase: ErrorMsg(" Can't find PCI space ");
		break;
	case Error_Physnotmapped: ErrorMsg(" Can't map PCI space ");
		break;
	case Error_Fktnotimplemented: ErrorMsg(" function not implemented");
		break;
	case Error_Timer: ErrorMsg(" PCI Timer Error ");
		break;

	}
	*/
	return TRUE;
	
};  // InitBoard

char CntBoards(void)
{//get how many PCI boards are there
	int i = 0;
	int foundBoards = 0;

	for (i = 1; i<5; i++) // check for max 4
		if (CCDDrvInit(i)) //call once per cam
		{
			foundBoards += 1;
		}
	return foundBoards;
}//CntBoards


//**************  new for PCIE   *******************************

BOOL SetDMAReg(ULONG Data, ULONG Bitmask, CHAR Address, UINT drvno){//the bitmask have "1" on the data dates like Bitmask: 1110 Data:1010 
	ULONG OldRegisterValues;
	ULONG NewRegisterValues;
	//read the old Register Values in the DMA Address Reg
	if (!ReadLongDMA(drvno, &OldRegisterValues, Address)){
		ErrLog("ReadLong DMA Failed in SetDMAReg \n");
		WDC_Err("%s", LSCPCIEJ_GetLastErr());
		return FALSE;
	}
	//save the bits, which shall not changed
	OldRegisterValues = OldRegisterValues & ~Bitmask; //delete the bits, which are "1" in the bitmask
	Data = Data & Bitmask; //to make sure that there are no bits, where the bitmask isnt set

	NewRegisterValues = Data | OldRegisterValues;
	//write the data to the DMA controller
	if (!WriteLongDMA(drvno, NewRegisterValues, Address)){
		ErrLog("WriteLong DMA Failed in SetDMAReg \n");
		WDC_Err("%s", LSCPCIEJ_GetLastErr());
		return FALSE;
	}

	if (!ReadLongDMA(drvno, &OldRegisterValues, Address)){
		ErrLog("ReadLong DMA Failed in SetDMAReg \n");
		WDC_Err("%s", LSCPCIEJ_GetLastErr());
		return FALSE;
	}
	return TRUE;
}
BOOL SetS0Reg(ULONG Data, ULONG Bitmask, CHAR Address, UINT drvno){
	ULONG OldRegisterValues;
	ULONG NewRegisterValues;
	//read the old Register Values in the S0 Address Reg
	if (!ReadLongS0(drvno, &OldRegisterValues, Address)){
		ErrLog("ReadLong S0 Failed in SetDMAReg \n");
		WDC_Err("%s", LSCPCIEJ_GetLastErr());
		return FALSE;
	}
	WDC_Err("S0 OldRegVal: %x\n", OldRegisterValues);
	//save the bits, which shall not changed
	OldRegisterValues = OldRegisterValues & ~Bitmask;
	NewRegisterValues = Data | OldRegisterValues;
	//write the data to the S0 controller
	if (!WriteLongS0(drvno, NewRegisterValues, Address)){
		ErrLog("WriteLong S0 Failed in SetDMAReg \n");
		WDC_Err("%s", LSCPCIEJ_GetLastErr());
		return FALSE;
	}
	return TRUE;
}

BOOL SetDMAAddrTlpRegs(UINT64 PhysAddrDMABuf64, ULONG tlpSize, UINT drvno){

	UINT64 addr64 = 0;
	ULONG PhysAddrDMABuf=0;
	ULONG RegisterValues=0;
	ULONG BitMask=0;
	//proof the physical address for the max. size of the DMA Controller
	/*this error never happens. I think there is no need for this check
	if (PhysAddrDMABuf64 > 0x3fffffffff){
	ErrLog("Physical DMA buffer address is more than 5bytes: 0x%016x \n",
	PhysAddrDMABuf64);
	WDC_Err("%s", LSCPCIEJ_GetLastErr());
	return FALSE;
	}*/
	//WDMATLPA (Reg name): write the lower part (bit 03:32) of the DMA adress to the DMA controller
	PhysAddrDMABuf = (ULONG)PhysAddrDMABuf64;
	BitMask = 0xFFFFFFFC;
	if (!SetDMAReg(PhysAddrDMABuf, BitMask, DmaAddr_WDMATLPA, drvno)){
		WDC_Err("Set the lower part of the DMA Address failed");
		return FALSE;
	}

	//WDMATLPS: write the upper part (bit 32:39) of the address	
	addr64 = PhysAddrDMABuf64 >> 32;
	PhysAddrDMABuf = (ULONG) addr64;	//shift to the upper part 
	addr64 = PhysAddrDMABuf;
	addr64 = addr64 << 24 ;
	PhysAddrDMABuf = (ULONG)addr64;		//shift to prepare for the Register
	BitMask = 0xFF001FFF;
	//CHECK proof 0x20 from old driver
	//ULONG TLPSize = 0x20;//(*ppDma)->Page->dwBytes / sizeof(WORD);//divide by 4 because of the conversion from bytes to WORD
	RegisterValues = PhysAddrDMABuf | tlpSize;
	if (!SetDMAReg(RegisterValues, BitMask, DmaAddr_WDMATLPS, drvno)){
		WDC_Err("Set the upper part of the DMA Address and the TLPsize failed");
		return FALSE;
	}

	//WDMATLPC: Set the number of DMA transfer count
	BitMask = 0xFFFF;
	if (!SetDMAReg(_NO_TLPS, BitMask, DmaAddr_WDMATLPC, drvno)){
		WDC_Err("Set the number of DMA transfer count failed");
		return FALSE;
	}
	return TRUE;
}//SetDMAAddrTlpRegs

BOOL SetDMAAddrTlp(UINT drvno){
	WD_DMA **ppDma = &pDMASubBufInfos;
	UINT64 PhysAddrDMABuf64;
	ULONG BitMask;

	//write Address
	PhysAddrDMABuf64 = (*ppDma)->Page[0].pPhysicalAddr;
	if (!SetDMAAddrTlpRegs(PhysAddrDMABuf64, TLPSize, drvno))
		return FALSE;
	return TRUE;
}
BOOL SetDMABufRegs(UINT drvno){
	//set DMA Buffer size in scans
	if (!SetS0Reg(DMABufSizeInScans, 0xffffffff, DmaAddr_DmaBufSize, drvno))//DMABufSizeInScans
		return FALSE;
	//ULONG reg;
	//ReadLongS0(DRV, &reg, DmaAddr_DmaBufSize);
	//WDC_Err("readreg DMABufSize: %x \n", reg);
	//set Scans per Interrupt
	if (!SetS0Reg(IntFreqInScans, 0xffffffff, DmaAddr_ScansPerIntr, drvno))
		return FALSE;
	//ReadLongS0(DRV, &reg, DmaAddr_ScansPerIntr);
	//WDC_Err("readreg SCANSPERINTR: %x \n", reg);
	return TRUE;
}
void SetDMAReset(void){
	ULONG BitMask;
	ULONG RegisterValues;
	BitMask = 0x1;
	RegisterValues = 0x1;
	if (!SetDMAReg(RegisterValues, BitMask, DmaAddr_DCSR, DRV)){
		WDC_Err("switch on the Initiator Reset for the DMA failed");
		return;
	}
	// DCSR: reset the Iniator Reset 
	RegisterValues = 0x0;
	if (!SetDMAReg(RegisterValues, BitMask, DmaAddr_DCSR, DRV)){
		WDC_Err("switch off the Initiator Reset for the DMA failed");
		return;
	}
}
void SetDMAStart(void){
	ULONG BitMask;
	ULONG RegisterValues;
	BitMask = 0x1;
	RegisterValues = 0x1;
	if (!SetDMAReg(RegisterValues, BitMask, DmaAddr_DDMACR, DRV)){
		WDC_Err("Set the Start Command for th DMA failed");
		return;
	}
}
BOOL SendDMAInfoToKP(void){

	DWORD hDma;
	DWORD dwOptions;
	PVOID pData;
	DWORD dwResult;
	PDWORD	pdwResult = &dwResult;


	WDC_Err("WDC:  hDma %u\n", pDMASubBufInfos->hDma);
	WDC_Err("WDC:  pUserAddr %u\n", pDMASubBufInfos->pUserAddr);
	WDC_Err("WDC:  pKernelAddr %u\n", pDMASubBufInfos->pKernelAddr);
	WDC_Err("WDC:  dwBytes %u\n", pDMASubBufInfos->dwBytes);
	WDC_Err("WDC:  dwOptions %u\n", pDMASubBufInfos->dwOptions);
	WDC_Err("WDC:  dwPages %u\n", pDMASubBufInfos->dwPages);
	WDC_Err("WDC:  hCard %u\n", pDMASubBufInfos->hCard);

	/*for WDK Flush
	pData = &dwDMASubBufSize;
	WDC_CallKerPlug(hDev, KP_LSCPCIEJ_MSG_BUFSIZE, pData, pdwResult);
	if (*pdwResult != KP_LSCPCIEJ_STATUS_OK)
	{
	WDC_Err("sendDMAInfoToKP dwDMASubBufSize send failed\n");
	return FALSE;
	}
	*/
	//for JUNGO Flush, complete WD_DMA struct
	/*
	pData = pDMASubBufInfos;
	WDC_CallKerPlug(hDev, KP_LSCPCIEJ_MSG_WDDMA, pData, pdwResult);
	if (*pdwResult != KP_LSCPCIEJ_STATUS_OK)
	{
	WDC_Err("sendDMAInfoToKP WD_DMA send failed\n");
	return FALSE;
	}
	*/
	//for Testfkt to write to the Regs
	/*
	pData = &hDev;// &deviceInfo.Card;//
	WDC_Err("deviceInfo.Card.Item[0].I.Mem.pTransAddr : 0x %x\n", deviceInfo.Card.Item[0].I.Mem.pTransAddr);
	WDC_CallKerPlug(hDev, KP_LSCPCIEJ_MSG_TESTSIGNALPREP, pData, pdwResult);
	if (*pdwResult != KP_LSCPCIEJ_STATUS_OK)
	{
	WDC_Err("sendDMAInfoToKP hDev send failed\n");
	return FALSE;
	}
	*/
	/*
	//for JUNGO Flush , fragmented WD_DMA struct
	dwOptions = pDMASubBufInfos->dwPages; //Im using dwPages instead of dwOptions

	pData = &dwOptions;
	WDC_CallKerPlug(hDev, KP_LSCPCIEJ_MSG_DWOPT, pData, pdwResult);
	if (*pdwResult != KP_LSCPCIEJ_STATUS_OK)
	{
	WDC_Err("sendDMAInfoToKP dwOptions send failed\n");
	return FALSE;
	}

	//send hDma of WD_DMA struct to KP
	hDma = pDMASubBufInfos->dwBytes;	 //Im using pUserAddr instead of hDma
	//because the structure is mixed. I think it is a bug.
	//Jungo says our dll or h files are mixed up
	pData = &hDma;
	WDC_CallKerPlug(hDev, KP_LSCPCIEJ_MSG_HDMA, pData, pdwResult);
	if (*pdwResult != KP_LSCPCIEJ_STATUS_OK)
	{
	WDC_Err("sendDMAInfoToKP hDma send failed\n");
	return FALSE;
	}
	*/

	return TRUE;
}

VOID DLLCALLCONV interrupt_handler(PVOID pData)
{	//here the DMASubBuf is copied to the DMABigBuf
	//DMA tranfer is always via DMASubBuf
	// the INTR occurs every 25 reads and copies 25 scans at once

	WDC_Err("entered interrupt handle\n");
	OutTrigHigh(DRV);
	PWDC_DEVICE pDev = (PWDC_DEVICE)pData;
	/*
	ULONG RegisterValues;
	ULONG BitMask = 0x100;
	UINT64 PhysAddrDMABuf64;
	volatile UINT64 ISRStartTime;
	volatile UINT64 ISREndTime = 0;
	volatile UINT64 CurrentISRTime = 0;
	*/
	//	ISRStartTime = ticksTimestamp();

	/*
	BYTE regdata;// = 0x08000000;
	ReadByteS0(DRV, &regdata, 0x4);
	regdata |= 0x8;
	WriteByteS0(DRV, regdata, 0x4);
	*/
	//SetS0Reg(regdata, regdata, 0x1, DRV);



	// copy pDMASubBuf to DMABigBuf 
	/*if (pDev->Int.dwCounter > 100){
	OutTrigLow(DRV);
	return;
	}*/
	WDC_Err("ISR 1\n");
	if (!(pDev->Int.dwCounter % (UserBufInScans / IntFreqInScans))) {//count until UserBuffer==UserBufinScansSize but divide because the ISR came with IntFrqInScans
		if (OneRingBufShot) { //stop
			StopFFTimer(DRV);
			SetIntFFTrig(DRV);//disable ext input
			OneRingBufShot = FALSE;
			OutTrigLow(DRV);
			WDC_Err("ISR 2\n");
			return;
		}
		//set Ring BigBuf back to first element
		pDMABigBuf = pDMABigBufBase;
		//DMAUserBufIndex = 0;
		WDC_Err("reset userringbuf to zero\n");
	}
	WDC_Err("ISR 3\n");
	//WDC_Err(" mod makes %d and USERBUFINSCANS/IntFreqInScans : %d and IntFreqInScans %d\n", pDev->Int.dwCounter % (USERBUFINSCANS / IntFreqInScans), (USERBUFINSCANS / IntFreqInScans), IntFreqInScans);
	PUSHORT pPartOfDMABuf;

/*	if (ISRCounter < 0) {
		ISRCounter++;
		WDC_Err("ISR 4\n");
		return;
	}
*/
	//copy a quarter of the small DMASubBuf to the Big DMABigBuf
	pPartOfDMABuf = pDMASubBuf + (ISRCounter *  IntFreqInScans);
	//if (pDev->Int.dwCounter % 2) { //odds
	WDC_Err("ISR 5\n");

	//!!GS
	//!!test write
	//memset(pDMASubBuf, (ISRCounter+1) * 10, IntFreqInScans * _PIXEL *sizeof(USHORT));

	//copy from block buffer pPartOfDMABuf to main buffer pDMABigBuf
	memcpy(pDMABigBuf, pPartOfDMABuf, IntFreqInScans * _PIXEL * sizeof(USHORT));



	WDC_Err("Partnumber of the DMA Buffer: %d\n", ISRCounter);
	ISRCounter++;
	if (ISRCounter >= 4)//number of ISR per dmaBuf - 1
		ISRCounter = 0;


	
	//count up the UserRingBuf
	pDMABigBuf += IntFreqInScans * _PIXEL; 
	//DMAUserBufIndex++;
	/*ISREndTime = ticksTimestamp();

	CurrentISRTime = ISREndTime - ISRStartTime;
	CurrentISRTime = Tickstous(CurrentISRTime);
	//if (CurrentISRTime > MaxISRTime)
	MaxISRTime = CurrentISRTime;
	*/

	WDC_Err("Got %d interrupts\n ", pDev->Int.dwCounter);
	OutTrigLow(DRV);
	//WDC_Err("DMACounter: %d \n", DMACounter);
}//DLLCALLCONV

BOOL SetupPCIE_DMA(UINT drvno)
{
	//***open the DMA
	//gets address of DMASubBuf from driver

	WDC_Err("entered SetupPCIE_DMA\n");
	DWORD dwOptions = DMA_FROM_DEVICE;// | DMA_ALLOW_64BIT_ADDRESS;// | DMA_ALLOW_CACHE;
	DWORD dwStatus;

	//do not read and write at the same time to the same range!
	//write 25 scans but read in INTR Handler only 25 always behind
	dwDMASubBufSize = (DMABufSizeInScans)  * _PIXEL * sizeof(USHORT);  //in bytes

	
	//only for test:
	//dwDMASubBufSize = 100;// *1000 * 1000;//100mb max

	// pDMASubBuf is the space which is allocated by this function = output - should be global
	dwStatus = WDC_DMAContigBufLock(hDev, &pDMASubBuf, dwOptions, dwDMASubBufSize, &pDMASubBufInfos); //size in Bytes
	//GS dwStatus = WDC_DMAContigBufLock(hDev, &pDMASubBuf, dwOptions, dwDMASubBufSize, &pDMASubBufInfos); //size in Bytes
	//dwStatus = WDC_DMASGBufLock(hDev, &DIODENRingBuf, dwOptions, dwDMASubBufSize, &pDMASubBufInfos); //size in Bytes


//GS	pDMASubBuf = &DIODENRingBuf;

	if (WD_STATUS_SUCCESS != dwStatus)
	{
		ErrLog("Failed locking a contiguous DMA buffer. Error 0x%lx - %s\n",
			dwStatus, Stat2Str(dwStatus));
		WDC_Err("%s", LSCPCIEJ_GetLastErr());
		return FALSE;
	}

	/*for KP
	if (HWINTR_EN)
	if(!SendDMAInfoToKP())
	WDC_Err("sendDMAInfoToKP failed");
	/*for KP
	if (HWINTR_EN)
	if(!SendDMAInfoToKP())
	WDC_Err("sendDMAInfoToKP failed");
	*/
	/*for KP
	if (HWINTR_EN)
	if(!SendDMAInfoToKP())
	WDC_Err("sendDMAInfoToKP failed");
	*/
	//

	//set Init Regs
	if (!SetDMAAddrTlp(drvno)){
		ErrLog("DMARegisterInit for TLP and Addr failed \n");
		WDC_Err("%s", LSCPCIEJ_GetLastErr());
		ErrorMsg("DMARegisterInit for TLP and Addr failed");
		return FALSE;
	}
	if (!SetDMABufRegs(drvno)){
		ErrLog("DMARegisterInit for Buffer failed \n");
		WDC_Err("%s", LSCPCIEJ_GetLastErr());
		ErrorMsg("DMARegisterInit for Buffer failed");
		return FALSE;
	}

	// Enable DMA interrupts (if not polling)
	dwStatus = LSCPCIEJ_IntEnable(hDev, interrupt_handler);
	if (WD_STATUS_SUCCESS != dwStatus)
	{
		ErrLog("Failed to enable the Interrupts. Error 0x%lx - %s\n",
			dwStatus, Stat2Str(dwStatus));
		WDC_Err("%s", LSCPCIEJ_GetLastErr());
		ErrorMsg("Failed to enable the Interrupts");
		return FALSE;
	}

	/*for testing
	ULONG CtrlData;
	//WriteLongS0(DRV, 0, 0x38); //set DREQ
	ReadLongS0(DRV, &CtrlData, 0x38);
	WDC_Err("S0Data Offs. 0x38: %x \n", CtrlData);
	*/

	ISRCounter = 0;
	//set user buf to firts element


	WDC_Err("finished SetupDMA\n");
	return TRUE;
}//SetupPCIE_DMA

void StartPCIE_DMAWrite(UINT drvno)
{	// starts transfer from PCIe board to PCs main RAM
	if (!HWINTR_EN){

		//WDC_Err("entered StartPCIE_DMAWrit\n");
		// DCSR: set the Iniator Reset 
		SetDMAReset();

		/* Flush the I/O caches (see documentation of WDC_DMASyncIo()) */
		//WDC_DMASyncIo(pDMASubBufInfos);
		/****DMA Transfer start***/
		/* Flush the CPU caches (see documentation of WDC_DMASyncCpu()) */
		//WDC_DMASyncCpu(pDMASubBufInfos);

		//SetDMADataPattern();
		/* DDMACR: Start DMA - write to the device to initiate the DMA transfer */
		SetDMAStart();
	}
}

void CleanupPCIE_DMA(UINT drvno)
{
	/* Disable DMA interrupts */
	WDC_IntDisable(hDev);
	/* Unlock and free the DMA buffer */
	WDC_DMABufUnlock(pDMASubBufInfos);

	/*
	BOOL fResult = FALSE;
	DWORD   ReturnedLength;
	ULONG	ctrlcode = 0;
	ULONG   data = 0;

	ctrlcode = 0x01; //is DCR2 as *4
	fResult = DeviceIoControl(ahCCDDRV[drvno], IOCTL_CLEANUP,  // read error code
	&ctrlcode,        // Buffer to driver.
	sizeof(ctrlcode),
	&data, sizeof(data), &ReturnedLength, NULL);
	if (!fResult)  { ErrorMsg("Cleanup failed"); };
	*/
}

BOOL ActMouse(UINT drvno)
{		// inits PCI Board and gets all needed addresses
	// and gets Errorcode if any
#ifndef _DEBUG
	ULONG Errorcode = Error_notinitiated;
	BOOL fResult = FALSE;
	DWORD   ReturnedLength;
	ULONG	ctrlcode = 3; // Activate mouse


	fResult = DeviceIoControl(ahCCDDRV[drvno], IOCTL_SetFct,  // read error code
		&ctrlcode,        // Buffer to driver.
		sizeof(ctrlcode),
		&Errorcode, sizeof(Errorcode), &ReturnedLength, NULL);
	if (!fResult)
		ErrorMsg("ActMouse failed");

	if (Errorcode != NoError)
	{
		ErrorMsg(" Mouse Error ");
		return FALSE;
	}

	ShowCursor(TRUE); //					
#endif
	return TRUE;
}//ActMouse

BOOL DeactMouse(UINT drvno)
{		// inits PCI Board and gets all needed addresses
	// and gets Errorcode if any
#ifndef _DEBUG
	ULONG Errorcode = Error_notinitiated;
	BOOL fResult = FALSE;
	DWORD   ReturnedLength;
	ULONG	ctrlcode = 2; // Deactivate mouse


	fResult = DeviceIoControl(ahCCDDRV[drvno], IOCTL_SetFct,  // read error code
		&ctrlcode,        // Buffer to driver.
		sizeof(ctrlcode),
		&Errorcode, sizeof(Errorcode), &ReturnedLength, NULL);
	if (!fResult)
		ErrorMsg("DeactMouse failed");

	if (Errorcode != NoError)
	{
		ErrorMsg(" Mouse Error ");
		return FALSE;
	}

	ShowCursor(FALSE); //		
#endif			
	return TRUE;

}//DeactMouse


int GetNumofProcessors()
{
	SYSTEM_INFO siSysInfo;
	GetSystemInfo(&siSysInfo);
	return siSysInfo.dwNumberOfProcessors;
}//GetNumofProcessors

void RSInterface(UINT drvno)
{
	ULONG reg = 0;
	ULONG i = 0;

	ReadLongIOPort(drvno, &reg, 0); //read LCR0 for check length 0xffffffco
	reg = ~reg; //length is inverted

	//set all to zero
	for (i = 0; i<reg / 4; i++) WriteLongS0(drvno, 0, i * 4);
}



BOOL SetBoardVars(UINT drvno, BOOL sym, BOOL burst, ULONG pixel, ULONG waits, ULONG flag816, ULONG pportadr,
	ULONG pclk, ULONG xckdelay)
{	//initiates board and DMA Registers
	//pportadr for compatibility - no function here
	// if FIFO: pclk set camera read frequency (write to FIFO), waits sets FIFO read frequency
	//	pclk -> freq=m_nWaitStates;//0=40MHz;1=20MHz;2=10MHz max 6=0 for FIFO mode
	// NO FIFO: pclk not used, waits sets read frequency
	//  xckdelay 0..7 set delay between XCK goes high and start of hor. clocks
	// xckdelay=3 default, =7 if Sony sensor
	BYTE data = 0;
	ULONG reg = 0;
	ULONG i = 0;
	USHORT buswidth = 0x840; //default 8 bit

	if (ahCCDDRV[drvno] == INVALID_HANDLE_VALUE)
	{
		WDC_Err("Handle is invalid of drvno: %i", drvno);
		return FALSE;
	}

	//set startval for CTRLA Reg  +slope, IFC=h, VON=1 

	WriteByteS0(drvno, 0x23, 0x04);  //write CTRLA reg in S0

	if (_COOLER) ActCooling(drvno, FALSE); //deactivate cooler


	waits = 0;
	//sym=FALSE; //use what is passed here
	//burst=TRUE;

	WriteByteS0(drvno, 0, 0x05);  //write CTRLB reg in S0
	
	WriteByteS0(drvno, 0, 0x06);  //write CTRLC reg in S0




	//set global vars if driver is there

	aWAITS[drvno] = waits;
	aFLAG816[drvno] = flag816;

	aPIXEL[drvno] = pixel;
	//	if (_HA_IR) 	aPIXEL[drvno] =  2 * pixel; // V2.45
	//	if (HA_IRSingleCH) 	aPIXEL[drvno] =  4 * pixel;
	aXCKDelay[drvno] = xckdelay;
	aINIT[drvno] = TRUE;


	//new for Fifo

	if (flag816 == 1) { reg = 2 * aPIXEL[drvno]; }//16 bit
	else { reg = aPIXEL[drvno]; };

	OpenShutter(drvno);//set to FFT
		

		//! GST Test: ND count soll > CAMACK sein damit keine ueberlappung
		reg = 1500;

		if (pclk>6) pclk = 6;
		reg |= pclk << 16;
		if (xckdelay>7) xckdelay = 7;
		if (xckdelay<1) xckdelay = 1;
		reg |= xckdelay << 19;
		WriteLongS0(drvno, reg, 0x10); //set pixelreg	


		/*

		if (reg>0x24) WriteLongS0(drvno,0, 0x24);	//reset ECReg
		if (reg>0x34) WriteLongS0(drvno,DELAY, 0x34);	//set WRFFDELAY
		*/
		return TRUE; //no error
};  // SetBoardVars




// ***************************  camera read stuff  **********
//*********************************************************

void Resort(UINT drvno, void* ptarget, void* psource)
//		example for resort array for 1 PCI board with 2 slots
//      array type long 8bit vals in +0 port1 and +1 in port2
//		resort for display -> port1 longs in DB1 and port2 longs in DB2 
{
	ULONG i = 0;
	ULONG barraylength = 0;
	ULONG larraylength = 0;

	BOOL gotit = FALSE;


	// 1----------------   resort for 1 PCI board and 2 or more slots
	//		example for resort array for 1 PCI board with 2 slots
	//      array type long 8bit vals in +0 port1 and +1 in port2
	//		resort for display -> port1 longs in DB1 and port2 longs in DB2 
	/*

	BYTE* pbtarray;
	BYTE* pbsarray ;

	pbsarray =(BYTE*) calloc(aPIXEL[drvno],sizeof(long));
	if (pbsarray==0)
	{ErrorMsg("alloc resort Buffer failed");
	return;
	}
	gotit=TRUE;
	larraylength = aPIXEL[drvno];
	barraylength = sizeof(ArrayT)*aPIXEL[drvno];

	pbtarray = (BYTE *)ptarget;


	for (i=0;i<barraylength;i++) //copy all relevant data to source
	pbsarray[i] =  pbtarray[i];


	for (i=0;i<larraylength-sizeof(ArrayT);i++)
	{
	pbtarray[i*4] = pbsarray[i*4+0];
	pbtarray[i*4+1] = 0; //pbsarray[i*2+1];
	pbtarray[i*4+2] = 0;//pbsarray[i*4+2];
	pbtarray[i*4+3] = 0;//pbsarray[i*4+3];
	//DB2 for display of 2nd camera
	pbtarray[barraylength+i*4] = pbsarray[i*4+1];
	pbtarray[barraylength+i*4+1] = 0; //pbsarray[i*2+1];
	pbtarray[barraylength+i*4+2] = 0;//pbsarray[i*4+2];
	pbtarray[barraylength+i*4+3] = 0;//pbsarray[i*4+3];
	}
	if (gotit) free(psarray);
	*/
	// ----------------- END  resort for 1 PCI board and 2 or more slots

	/*
	// 2---------------- resort for 2 channels IR cams parallel
	#if (_IR2) //if IR reads 2 lines at once
	//takes 3 micro sec on amd1000

	if (zadr==1) chl = 2; // CH2=1  CH1=2
	if (zadr==2) chl = 1;
	if (zadr==0) { chl =2; chh=1;}; //get both channels

	if (fkt==2)
	{ //add
	for (i=0;i<	aPIXEL[drvno];i++)
	* (pDiodenBase++) +=  * (LPLONG)(pCorArray + i*sizeof(ArrayT) + chl );

	if (zadr==0)
	for (i=0;i<	aPIXEL[drvno];i++) //get second array
	* (pDiodenBase2++) += * (LPLONG)(pCorArray + i*sizeof(ArrayT) + chh );
	}
	else
	{// fkt==1 standard read
	for (i=0;i<	aPIXEL[drvno];i++)
	* (pDiodenBase++) =  * (LPLONG)(pCorArray + i*sizeof(ArrayT) + chl );

	if (zadr==0)
	for (i=0;i<	aPIXEL[drvno];i++) //get second array
	* (pDiodenBase2++)= * (LPLONG)(pCorArray + i*sizeof(ArrayT) + chh );
	}

	// ----------------- END resort for 2 channels IR cams parallel
	*/

	// 3----------------- resort for standard IR camera
	//resort for IR camera - only pixel 3 and 4 are valid (overclocked)
	ArrayT* pltarray;
	ArrayT* plsarray;

	plsarray = (ArrayT *)psource;
	pltarray = (ArrayT *)ptarget;

	//	if (_IR2) pltarray += 4*_PIXEL * sizeof(ArrayT);


#if (HA_IRSingleCH) // one channel for HA G92xx with 256 pixel
	larraylength = _PIXEL;


	if (_FFPCI133) //16 bit 1 channel cam
	{
		pltarray[0] = plsarray[1] & 0x8080;//show shutterstate in first pixel 
		for (i = 1; i<larraylength; i++)
			pltarray[i] = plsarray[4 * i + 2];   // 0 in flanke, 1,2 signal, 3 null
		//pltarray[i] = plsarray[4*i+1] - plsarray[4*i+3] + 1000;  // 0 in flanke, 1,2 signal, 3 null
		//pltarray[i] = (plsarray[4*i+1] + plsarray[4*i+2])/2 - plsarray[4*i+3] + 1000;
	}
	else // 12bit IR cams 1 channel
	{
		for (i = 0; i<larraylength; i++)
			pltarray[i] = plsarray[4 * i + 3];   // CH od 
		if (_HWCH2) // 2cams parallel -> append 2nd array
			for (i = 0; i<larraylength; i++)
				pltarray[i + _PIXEL] = plsarray[_PIXEL * 4 + 4 * i + 3];
		//pltarray[i] = plsarray[4*i+2];  // CH ev
	}//else


#else	// two channel sensors with 512 pixel

	larraylength = _PIXEL / 2;

	for (i = 0; i<larraylength; i++)
	{ //test:600 fifo
		pltarray[2 * i + 1] = plsarray[4 * i + 1]; //was 2!!must change direction up/dn or modulation
		pltarray[2 * i] = plsarray[4 * i + 2]; //was 3

		if (_HWCH2) // 2cams parallel -> append 2nd array
			for (i = 0; i<larraylength; i++)
			{
				pltarray[2 * i + 1 + _PIXEL] = plsarray[_PIXEL * 4 + 4 * i + 2];
				pltarray[2 * i + _PIXEL] = plsarray[_PIXEL * 4 + 4 * i + 3];
			}
	}


#endif

	//  ----------------- END resort for standard IR camera


	// 4-------- resort hi/lo
	/*
	if (aFLAG816[drvno] ==1)
	{	// resort 12/16 bit array
	ULONG i=0;
	BYTE* ptarray;
	BYTE* psarray ;
	psarray = (BYTE*) calloc(_PIXEL*4, sizeof(BYTE));



	//		psarray = (UCHAR*) ptarget;  //(BYTE*) calloc(PIXEL*4, sizeof(BYTE));
	ptarray = (UCHAR*) ptarget;

	for (i=0;i<_PIXEL*4;i++)
	psarray[i]= ptarray[i];

	if (sizeof(ArrayT)==1) {return ;}; //8 bit not for 12bit

	//resort 12 bit array takes 4ns / pixel on a 3GHz PC
	if (sizeof(ArrayT)==4) //target 32 bit
	{
	for (i=0;i<_PIXEL-5;i++)
	{
	ptarray[i*4] = psarray[i*4+1];		// lo byte
	ptarray[i*4+1] = psarray[i*4];  // hi byte
	//	 ptarray[i*4+2] =  0;
	//	 ptarray[i*4+3] =  0;
	}
	}; //32 bit

	if (sizeof(ArrayT)==2) //target 16 bit
	{
	for (i=0;i<_PIXEL;i++)
	{
	ptarray[i*2] =  psarray[i*2+1];	// lo byte
	ptarray[i*2+1] =  psarray[i*2];	// hi byte
	}
	}; //16 bit
	free(psarray);
	}
	*/
	// ___________________________ END resort Hi/Lo

}	//Resort 







//replaced by StartReadWithDma
BOOL CallWRFile(UINT drvno, void* pdioden, ULONG arraylength, ULONG fkt)
{	//here  the standard read fkt=1 is implemented
	//wrap call to WriteFile to avaoid driver problem if target array>4k

	BOOL fResult = FALSE;
	DWORD   ReturnedLength = 0;
	sCCDFkts CCDfkt;
	BOOL b12alloc = FALSE;
	//DWORD arraylength = aPIXEL[drvno] * sizeof(ArrayT); //length in bytes
	PUSHORT pRArray = (USHORT*)pdioden;
	ULONG pixel = aPIXEL[drvno];
	BYTE a = 1;
	BYTE b = 0;
	ULONG firstFFTclks = 0;// fftlines*8
	PUSHORT pDioden;

	pDioden = &DIODEN;// (USHORT*)calloc(arraylength, sizeof(USHORT));


	//set function recs - here all have to be 0 - as we read the Fifo
	CCDfkt.NoOfLines = 0; // NoOfLines = block in FIFO DMA on demand
	CCDfkt.ADFlag = 0;	//dsdelay = delay before DMA start
	CCDfkt.Adrwaits = 0; // adrwaits -> 3mu
	CCDfkt.Waits = 0;  // Waits between vclks 6mu
	CCDfkt.Vclks = 0;
	CCDfkt.Fkt = fkt;  //for test purpose
	CCDfkt.Zadr = 0;

	//	arraylength = arraylength * 2; //2 * aPIXEL[drvno];
	/*
	arraylength = 2*aPIXEL[drvno]; //2bytes per word
	//alloc local array dioden, so we don't overwrite our DIODEN
	//2.200 pRArray = (BYTE*) calloc(aPIXEL[drvno],4*sizeof(BYTE));
	pRArray = (BYTE*) calloc(arraylength,sizeof(BYTE));
	if (pRArray==0)
	{ErrorMsg("alloc 12bit Buffer failed");
	return FALSE; }
	b12alloc=TRUE;
	*/
	//don't use dyn array, but static DIODEN instead

	//read camera data
	StartPCIE_DMAWrite(drvno);
	//fResult = WriteFile(ahCCDDRV[drvno], pDioden, arraylength, &ReturnedLength, NULL); //write to PC RAM, length in BYTES
	//if ((! fResult) || (ReturnedLength!=arraylength))
	//return FALSE;

	
	//	rtnlength = ReturnedLength;
	//	ValMsg(arraylength);
	//	ValMsg(ReturnedLength);

	//memcpy(pdioden, pDioden, arraylength);

	//if (b12alloc) free(pRArray);
	return TRUE;
}//CallWRFile




//replaced by StartReadWithDma
BOOL ReadPCIEFifo(UINT drvno, void* pdioden, long fkt)
{	//reads fifo data to buffer dioden
	//drvno: driver number 1..4; 1 for LSCPCI
	//dioden: pointer to destination array of type ArrayT
	//fkt=-1->read&don't store;fkt=0->clear array; fkt=1->read; fkt=2->add; 
	//returns true; false on error
	//same as GETCCD, but no parameters for fftlines .. zadr


	pArrayT pReadArray;
	pArrayT	pDiodenBase;
	pArrayT	pDiodenBase2;

	ULONG length = 0;
	ULONG i = 0;
	ULONG lwritten = 0;
	BOOL addalloc = FALSE;

	if (!aINIT[drvno]) return FALSE;	// return with error if no init

	pReadArray = (pArrayT)pdioden;
	//	pReadArray = pReadArray + (db-1) * pixel;
	pDiodenBase = pReadArray;

	if (fkt == 0) // set array to 0
	{
		for (i = 0; i< _PIXEL; i++)
			*(pReadArray++) = 0;
		return TRUE;
	}

	if (fkt == 5) // set array to i
	{
		for (i = 0; i< _PIXEL; i++)
			*(pReadArray++) = (ArrayT)i;
		return TRUE;
	}

	if (fkt>9)
		return FALSE;  // function not implemented


	if ((fkt == 2) || (fkt == -1)) //read in our local array ladioden - add and clrread
	{
		//alloc local array dioden, so we don't overwrite our DIODEN
		pReadArray = (pArrayT)calloc(aPIXEL[drvno], sizeof(ArrayT));
		if (pReadArray == 0)
		{
			ErrorMsg("alloc ADD/CLR Buffer failed");
			return FALSE;
		}
		addalloc = TRUE;
	}


	//call the read  -  only copy _NO_TLPS * 128 ! - target array may be greater
	//if (!WriteFile(ahCCDDRV[drvno], pReadArray, _NO_TLPS * 128, &lwritten, NULL)) //write to PC RAM, length in BYTES
	if (!CallWRFile(drvno, pReadArray, _NO_TLPS * 128, fkt))
	{
		ErrorMsg("Read DMA Buffer - FIFO failed");
		if (addalloc) free(pReadArray);
		return FALSE;
	}



	if (fkt == -1)
	{ // return , nothing else to do
		if (addalloc) free(pReadArray);
		return TRUE;
	}

	if (_RESORT) Resort(drvno, pReadArray, pReadArray);  //pixel resort




	//clrread and add fkt=-1 and 2 could not be implemented with dma
	//so we do it here after reading
	if (fkt == 2) // we must now add our data to DIODEN for online add
	{
		pDiodenBase2 = pReadArray;
		for (i = 0; i<_PIXEL; i++)
			* (pDiodenBase++) += *(pDiodenBase2++);
	}

	if (addalloc) free(pReadArray);

	return TRUE; // no Error, all went fine
};  // ReadPCIEFifo






// *********************** PCI board registers


BOOL CallIORead(UINT drvno, void* pdioden, ULONG fkt)
{	//here  the standard read fkt=1 is implemented
	//FIFO version -> = ReadFifo with resort & for double line cams

	BOOL fResult = FALSE;
	DWORD   ReturnedLength = 0;
	sCCDFkts CCDfkt;
	BOOL b12alloc = FALSE;
	DWORD arraylength = aPIXEL[drvno] * sizeof(ArrayT); //length in bytes
	PUCHAR pRArray = (UCHAR*)pdioden;
	ULONG pixel = aPIXEL[drvno];
	BYTE a = 1;
	BYTE b = 0;
	ULONG firstFFTclks = 0;// fftlines*8

	//set function recs - here all have to be 0 - as we read the Fifo
	CCDfkt.NoOfLines = 0; // NoOfLines = block in FIFO DMA on demand
	CCDfkt.ADFlag = 0;	//dsdelay = delay before DMA start
	CCDfkt.Adrwaits = 0; // adrwaits -> 3mu
	CCDfkt.Waits = 0;  // Waits between vclks 6mu
	CCDfkt.Vclks = 0;
	CCDfkt.Fkt = fkt;  //for test purpose
	CCDfkt.Zadr = 0;

	if (aFLAG816[drvno] == 2) // 8 bit allways byte array 
	{
		arraylength = aPIXEL[drvno];
		//alloc local array dioden, so we don't overwrite our DIODEN
		pRArray = (BYTE*)calloc(arraylength, sizeof(BYTE));
		if (pRArray == 0)
		{
			ErrorMsg("alloc 12bit Buffer failed");
			return FALSE;
		}
		b12alloc = TRUE;
	}

	//2.45
	//if (_HA_IR) arraylength*= 2;


	if (aFLAG816[drvno] == 1) // 12 bit allways byte array of words
	{
		if (HA_MODULE) // HA module has 4x values + if FFT lines*8 values
		{
			firstFFTclks = FFTLINES * 8; //FFT lines
			arraylength = 2 * aPIXEL[drvno]; //2bytes per word
			arraylength *= 4; //HA Module
			if (HA_IRSingleCH) arraylength *= 2;
			arraylength += firstFFTclks * 4; //FFT lines

			//if (_HWCH2) arraylength *= 2; //double if 2 cams 
			//alloc local array dioden, so we don't overwrite our DIODEN
			//2.200 pRArray = (BYTE*) calloc(aPIXEL[drvno],4*sizeof(BYTE)); 
			pRArray = (BYTE*)calloc(arraylength, sizeof(BYTE));
			if (pRArray == 0)
			{
				ErrorMsg("alloc HA Module Buffer failed");
				return FALSE;
			}
			b12alloc = TRUE;
		}
		else
		{
			arraylength = 2 * aPIXEL[drvno]; //2bytes per word

			if (_HWCH2) arraylength *= 2; //double if 2 cams 
			//alloc local array dioden, so we don't overwrite our DIODEN
			//2.200 pRArray = (BYTE*) calloc(aPIXEL[drvno],4*sizeof(BYTE)); 
			pRArray = (BYTE*)calloc(arraylength, sizeof(BYTE));
			if (pRArray == 0)
			{
				ErrorMsg("alloc 12bit Buffer failed");
				return FALSE;
			}
			b12alloc = TRUE;
		}
	}



	//read camera data
	/*old driver
	fResult = DeviceIoControl(ahCCDDRV[drvno], IOCTL_GetCCD,
		&CCDfkt, sizeof(CCDfkt),
		pRArray, arraylength,
		&ReturnedLength, NULL);
	if ((!fResult) || (ReturnedLength != arraylength))
		return FALSE;
	*/
	if (aFLAG816[drvno] == 2)
	{	// resort 8 bit array -> BYTE sort to data array 8,16 or 32
		ULONG i = 0;
		BYTE* ptarray;
		BYTE* psarray;
		psarray = (UCHAR*)pRArray;  //(BYTE*) calloc(PIXEL*4, sizeof(BYTE));
		ptarray = (UCHAR*)pdioden;

		if (sizeof(ArrayT) == 1) { return FALSE; }; //8 bit no sort

		if (sizeof(ArrayT) == 4) //target 32 bit
		{//BYTE resort
			for (i = 0; i<aPIXEL[drvno]; i++) //_PIXEL
			{//standard resort for FIFO 1 channel
				ptarray[i * 4] = psarray[i]; // target1 lo byte
				ptarray[i * 4 + 1] = 0;
				ptarray[i * 4 + 2] = 0;
				ptarray[i * 4 + 3] = 0;
			}
		}; //32bit

		if (sizeof(ArrayT) == 2) //target 16 bit
		{
			for (i = 0; i<aPIXEL[drvno]; i++)
			{
				ptarray[i * 2] = psarray[i];	// lo byte
				ptarray[i * 2 + 1] = 0;
			}
		}; //16 bit
	}// if AD=8bit


	if (aFLAG816[drvno] == 1)
	{	// resort 12/16 bit array -> BYTE sort of array data
		ULONG i = 0;
		ULONG pixel = aPIXEL[drvno];
		BYTE* ptarray;
		BYTE* psarray;
		psarray = (UCHAR*)pRArray;  //(BYTE*) calloc(PIXEL*4, sizeof(BYTE));
		ptarray = (UCHAR*)pdioden;

		if (sizeof(ArrayT) == 1) { return FALSE; }; //8 bit not for 12bit

		if (sizeof(ArrayT) == 4) //target 32 bit
		{//BYTE resort
			if (_HWCH2) //2 cams
			{
				for (i = 0; i<pixel; i++) //_PIXEL
				{
					//resort for 16bit 2 channel 2word->1long cha/chb  
					//2.227 hi/lo changed  
					if (_HILO) { a = 2; b = 0; }
					else { a = 0; b = 2; };
					ptarray[i * 4] = psarray[4 * i + b];   // target1 lo 
					ptarray[i * 4 + 1] = psarray[4 * i + a]; // target1 hi  

					ptarray[i * 4 + 2] = psarray[4 * i + b + 1]; // target2 lo	
					ptarray[i * 4 + 3] = psarray[4 * i + a + 1]; //	target2 hi  

					//			ptarray[i*8] =   //psarray[8*i+4]; // war +4 target1 lo byte
					//			ptarray[i*8+1] = 0;//psarray[8*i+2]; //psarray[8*i-2];   // war +2 target2 hi byte
					//			ptarray[i*8-4] = 0;//psarray[8*i];//psarray[8*i+4]; // target1 lo byte war -4
					//			ptarray[i*8-3] = 0;//psarray[8*i-2];//psarray[8*i+2];   // t1 target1 hi byte				
				}
			}
			else //not HWCH2
				if (HA_MODULE)
				{
					if (_HILO) { a = 0; b = 1; };
					for (i = 0; i<pixel; i++) //_PIXEL
					{// resort for 1 channel word->long
						// for HA Module C8061 and C7041
						//if FFT first clks for vclk are not valid
						/* take every value
						ptarray[i*4] =  psarray[2*i+a+firstFFTclks*4]; // target1 lo byte
						ptarray[i*4+1] = psarray[2*i+b+firstFFTclks*4];    // target1 hi byte
						*/
						if (HA_IRSingleCH)
						{
							if (_HILO) { a = 8; b = 9; }
							else { a = 9; b = 8; }
							//IR 256 resort
							ptarray[i * 4] = psarray[16 * i + a]; // target1 lo byte
							ptarray[i * 4 + 1] = psarray[16 * i + b];    // target1 hi byte
							ptarray[i * 4 + 2] = 0;
							ptarray[i * 4 + 3] = 0;
						}
						else
						{
							//FFT resort
							if (_HILO) { a = 3; b = 2; }
							else { a = 3; b = 2; }
							ptarray[i * 4] = psarray[8 * i + a + firstFFTclks * 4]; // target1 lo byte
							ptarray[i * 4 + 1] = psarray[8 * i + b + firstFFTclks * 4];    // target1 hi byte
							ptarray[i * 4 + 2] = 0;
							ptarray[i * 4 + 3] = 0;
						}
					}//for
				}//HA_MODULE
				else //not HA_MODULE
				{
	
					for (i = 0; i<pixel; i++) //_PIXEL
					{//standard resort for FIFO 1 channel word->long
						//2.227 hi/lo changed -> for FFT and PDA
						if (_HILO) { a = 0; b = 1; };
						ptarray[i * 4] = psarray[2 * i + a]; // target1 lo byte
						ptarray[i * 4 + 1] = psarray[2 * i + b];    // target1 hi byte
						ptarray[i * 4 + 2] = 0;
						ptarray[i * 4 + 3] = 0;
					}
				}
		}; //32 bit

		//resort 12 bit array takes 4ns / pixel on a 3GHz PC
		if (sizeof(ArrayT) == 2) //target 16 bit array
		{
			if (_HWCH2) //2 cams
			{
				for (i = 0; i<pixel - 1; i++)
				{//here the 2cam 12bit is appended
					//append 2nd cam first
					if (_HILO) { a = 2; b = 0; }
					else { a = 0; b = 2; };
					ptarray[2 * pixel + i * 2 + 1] = psarray[i * 4 + a + 1];	// hi byte cam2 
					ptarray[2 * pixel + i * 2] = psarray[i * 4 + b + 1];	// lo byte cam2

					ptarray[i * 2 + 1] = psarray[i * 4 + a];	// hi byte cam1
					ptarray[i * 2] = psarray[i * 4 + b];	// lo byte cam1

				} //for
			}
			else // only one camera
			{
				if (HA_MODULE)
				{//resort for HA Module
					if (HA_IRSingleCH)
					{
						if (_HILO) { a = 8; b = 9; }
						else { a = 9; b = 8; }
						for (i = 0; i<pixel; i++)
						{//change lo-hi
							ptarray[i * 2] = psarray[i * 16 + a];	// lo byte
							ptarray[i * 2 + 1] = psarray[i * 16 + b];	// hi byte
						}
					}
					else//FFT
					{
						if (_HILO) { a = 3; b = 2; }
						else { a = 3; b = 2; };
						for (i = 0; i<pixel; i++)
						{//change lo-hi
							ptarray[i * 2] = psarray[i * 8 + a + firstFFTclks * 4];	// lo byte
							ptarray[i * 2 + 1] = psarray[i * 8 + b + firstFFTclks * 4];	// hi byte
						}
					}

				}
				else
				{//standard read
					if (_HILO) { a = 0; b = 1; }

					for (i = 0; i<pixel; i++)
					{//change lo-hi
						ptarray[i * 2] = psarray[i * 2 + a];	// lo byte
						ptarray[i * 2 + 1] = psarray[i * 2 + b];	// hi byte
					}
				}
			}
		}; //16 bit
	}

	if (b12alloc) free(pRArray);
	return TRUE;
}//CallIORead



//  call of the read function - FIFO version
BOOL ReadFifo(UINT drvno, void* pdioden, long fkt)
{	//reads fifo data to buffer dioden
	//drvno: driver number 1..4; 1 for LSCPCI
	//dioden: pointer to destination array of type ArrayT
	//fkt=-1->read&don't store;fkt=0->clear array; fkt=1->read; fkt=2->add; 
	//returns true; false on error
	//same as GETCCD, but no parameters for fftlines .. zadr


	pArrayT pReadArray;
	pArrayT	pDiodenBase;
	pArrayT	pDiodenBase2;

	ULONG length = 0;
	ULONG i = 0;
	BOOL addalloc = FALSE;

	if (!aINIT[drvno]) return FALSE;	// return with error if no init

	pReadArray = (pArrayT)pdioden;
	//	pReadArray = pReadArray + (db-1) * pixel;
	pDiodenBase = pReadArray;

	if (fkt == 0) // set array to 0
	{
		for (i = 0; i< _PIXEL; i++)
			*(pReadArray++) = 0;
		return TRUE;
	}
	/*
	if (fkt==5) // set array to i
	{
	for (i=0;i<	_PIXEL;i++)
	*(pReadArray++) =  (ArrayT) i;
	return TRUE;
	}
	*/
	if (fkt>9)
		return FALSE;  // function not implemented

	//if ((_IR) && (!addalloc))

		if ((fkt == 2) || (fkt == -1)) //read in our local array ladioden - add and clrread
		{
			//alloc local array dioden, so we don't overwrite our DIODEN
			pReadArray = (pArrayT)calloc(aPIXEL[drvno], sizeof(ArrayT));
			if (pReadArray == 0)
			{
				ErrorMsg("alloc ADD/CLR Buffer failed");
				return FALSE;
			}
			addalloc = TRUE;
		}


	//call the read
	if (!CallIORead(drvno, pReadArray, fkt))
	{
		ErrorMsg("Read DMA Buffer - FIFO failed");
		if (addalloc) free(pReadArray);
		return FALSE;
	}


	if (fkt == -1)
	{ // return , nothing else to do
		if (addalloc) free(pReadArray);
		return TRUE;
	}

	if (_RESORT) Resort(drvno, pReadArray, pReadArray);  //pixel resort


	//clrread and add fkt=-1 and 2 could not be implemented with dma
	//so we do it here after reading
	if (fkt == 2) // we must now add our data to DIODEN for online add
	{
		pDiodenBase2 = pReadArray;
		for (i = 0; i<_PIXEL; i++)
			* (pDiodenBase++) += *(pDiodenBase2++);
	}

	if (addalloc) free(pReadArray);

	return TRUE; // no Error, all went fine
};  // ReadFifo


//done
BOOL ReadLongIOPort(UINT drvno, ULONG *DWData, ULONG PortOff)
//this function reads the memory mapped data , not the I/O Data
{// reads long of PCIruntime register LCR
	// PortOff: Reg Offset from BaseAdress - in bytes
	// on return -> TRUE if success
	volatile DWORD dwStatus = 0;
	DWORD   ReturnedLength;
	ULONG	PortOffset;

	PortOffset = PortOff;
	dwStatus = WDC_PciReadCfg(hDev, PortOff, DWData, sizeof(ULONG));
	if (WD_STATUS_SUCCESS != dwStatus)
	{
		ErrLog("Failed to read the PCI Cfg Space\n"
			"Error 0x%lx - %s\n", dwStatus, Stat2Str(dwStatus));
		WDC_Err("%s", LSCPCIEJ_GetLastErr());
		//old message...i kept it because i dont know what it does
		ErrorMsg("Read IORunReg failed");
		return FALSE;
	}

	/* old driver function
	DeviceIoControl(ahCCDDRV[drvno],IOCTL_ReadLongIORunReg,
	&PortOffset,
	sizeof(PortOffset),
	DWData,sizeof(ULONG),&ReturnedLength,NULL);
	*/

	return TRUE;
};  // ReadLongIOPort

//done
BOOL ReadLongS0(UINT drvno, ULONG *DWData, ULONG PortOff)
{// reads long on space0 area
	// PortOff: Offset from BaseAdress - in Bytes !
	// return -> TRUE if success
	volatile DWORD dwStatus = 0;
	ULONG	PortOffset;

	//space0 starts at S0-Offset=0x80 in BAR0
	PortOffset = PortOff + 0x80;

	dwStatus = WDC_ReadAddrBlock(hDev, 0, PortOffset, sizeof(ULONG), DWData, WDC_MODE_8, WDC_ADDR_RW_DEFAULT);
	//the second parameter gives the memory space 0:mem mapped cfg/S0-space 1:I/O cfg/S0-space 2:DMA-space
	if (WD_STATUS_SUCCESS != dwStatus)
	{
		ErrLog("Failed to read the AddrBlock as an long in space0\n"
			"Error 0x%lx - %s\n", dwStatus, Stat2Str(dwStatus));
		WDC_Err("%s", LSCPCIEJ_GetLastErr());
		//old message...i kept it because i dont know what it does
		ErrorMsg("Read long in space0 failed");
		return FALSE;
	}

	/*fResult = DeviceIoControl(ahCCDDRV[drvno],IOCTL_ReadLongS0,  // read one byte
	&PortOffset,        // Buffer to driver.
	sizeof(PortOffset),
	DWData,sizeof(ULONG),&ReturnedLength,NULL);
	if (! fResult)
	{ErrorMsg("Read long in space0 failed");  return FALSE;};
	*/
	return TRUE;
};  // ReadLongS0

//done
BOOL ReadLongDMA(UINT drvno, ULONG *DWData, ULONG PortOff)
{// reads long on DMA area
	// PortOff: Offset from BaseAdress - in Bytes !
	// return -> TRUE if success
	volatile DWORD dwStatus = 0;
	DWORD   ReturnedLength;
	ULONG	PortOffset;

	PortOffset = PortOff;
	dwStatus = WDC_ReadAddrBlock(hDev, 0, PortOff, sizeof(ULONG), DWData, WDC_MODE_8, WDC_ADDR_RW_DEFAULT);
	//the second parameter gives the memory space 0:mem mapped cfg/S0-space 1:I/O cfg/S0-space 2:DMA-space
	if (WD_STATUS_SUCCESS != dwStatus)
	{
		ErrLog("Failed to read the AddrBlock of DMA\n"
			"Error 0x%lx - %s\n", dwStatus, Stat2Str(dwStatus));
		WDC_Err("%s", LSCPCIEJ_GetLastErr());
		//old message...i kept it because i dont know what it does
		ErrorMsg("Read long in DMA failed");
		return FALSE;
	}
	/*
	fResult = DeviceIoControl(ahCCDDRV[drvno], IOCTL_ReadLongDMAReg,  // read one byte
	&PortOffset,        // offset in bytes
	sizeof(PortOffset),
	DWData, sizeof(ULONG), &ReturnedLength, NULL);
	if (!fResult)
	{
	ErrorMsg("Read long in DMA failed");  return FALSE;
	};
	*/
	return TRUE;
};  // ReadLongDMA
//done
BOOL ReadByteS0(UINT drvno, BYTE *data, ULONG PortOff)
{// reads byte in space0 area except r10-r1f
	// PortOff: Offset from BaseAdress - in Bytes !
	// returns TRUE if success
	volatile DWORD dwStatus = 0;
	DWORD   ReturnedLength;
	ULONG	PortOffset;

	PortOffset = PortOff + 0x80;
	dwStatus = WDC_ReadAddrBlock(hDev, 0, PortOffset, 1/*sizeof(BYTE)*/, data, WDC_MODE_8, WDC_ADDR_RW_DEFAULT);
	//the second parameter gives the memory space 0:mem mapped cfg/S0-space 1:I/O cfg/S0-space 2:DMA-space
	if (WD_STATUS_SUCCESS != dwStatus)
	{
		ErrLog("Failed to read the AddrBlock as an byte in spaceS0\n"
			"Error 0x%lx - %s\n", dwStatus, Stat2Str(dwStatus));
		WDC_Err("%s", LSCPCIEJ_GetLastErr());
		//old message...i kept it because i dont know what it does
		ErrorMsg("Read byte in space0 failed");
		return FALSE;
	}
	/*
	fResult = DeviceIoControl(ahCCDDRV[drvno],IOCTL_ReadByteS0,  // read one byte
	&PortOffset,        // Buffer to driver.
	sizeof(PortOffset),
	data,sizeof(UCHAR),&ReturnedLength,NULL);
	if (! fResult)
	{ ErrorMsg("Read byte in space0 failed");
	return FALSE;};
	*/
	return TRUE;
};  // ReadByteS0

//done
BOOL WriteLongIOPort(UINT drvno, ULONG DWData, ULONG PortOff)
{	// writes long to PCIruntime register
	// PortOff: Reg Offset from BaseAdress - in bytes
	// returns TRUE if success
	BOOL fResult = FALSE;
	sDLDATA WriteData;
	ULONG	DataLength;
	DWORD   ReturnedLength;
	volatile DWORD dwStatus = 0;
	PULONG data = &DWData;

	//WriteData.POff	= PortOff;
	//WriteData.Data	= DWData;
	//DataLength		= 8; 

	dwStatus = WDC_PciWriteCfg(hDev, PortOff, data, sizeof(ULONG));
	if (WD_STATUS_SUCCESS != dwStatus)
	{
		ErrLog("Failed to write to the PCI Cfg space\n"
			"Error 0x%lx - %s\n", dwStatus, Stat2Str(dwStatus));
		WDC_Err("%s", LSCPCIEJ_GetLastErr());
		//old message...i kept it because i dont know what it does
		ErrorMsg("WriteLongIOPort failed");
		return FALSE;
	}
	/*
	fResult = DeviceIoControl(ahCCDDRV[drvno],IOCTL_WriteLongIORunReg,
	&WriteData,
	DataLength,
	NULL,0,&ReturnedLength,NULL);
	if (! fResult)
	{ErrorMsg("WriteLongIOPort failed");
	return FALSE;}
	*/
	return TRUE;
};  // WriteLongIOPort

//done
BOOL WriteLongS0(UINT drvno, ULONG DWData, ULONG PortOff)
{	// writes long to space0 register
	// PortOff: Reg Offset from BaseAdress - in bytes
	// returns TRUE if success


	volatile DWORD dwStatus = 0;
	ULONG	PortOffset;
	PULONG data = &DWData;

	PortOffset = PortOff + 0x80;

	dwStatus = WDC_WriteAddrBlock(hDev, 0, PortOffset, sizeof(ULONG), data, WDC_MODE_8, WDC_ADDR_RW_DEFAULT);
	//the second parameter gives the memory space 0:mem mapped cfg/S0-space 1:I/O cfg/S0-space 2:DMA-space
	if (WD_STATUS_SUCCESS != dwStatus)
	{
		ErrLog("Failed to write the AddrBlock as an long in space0\n"
			"Error 0x%lx - %s\n", dwStatus, Stat2Str(dwStatus));
		WDC_Err("%s", LSCPCIEJ_GetLastErr());
		//old message...i kept it because i dont know what it does
		ErrorMsg("WriteLongS0 failed");
		return FALSE;
	}
	/*
	ULONG checkdata;
	ReadLongS0(DRV, &checkdata, PortOff);
	WDC_Err("XXXWriteLongS0: %x\n", *data);
	if (*data != checkdata){
	WDC_Err("\nWriteLong:\ndata to write: %x\n", *data);
	WDC_Err("data read: %x\n", checkdata);
	WDC_Err("Address in S0: %x\n", PortOff);
	}*/

	/*
	//space0 starts at S0-Offset in BAR0
	WriteData.POff = PortOff; // offset in bytes
	WriteData.Data = DWData;
	DataLength = 8;

	fResult = DeviceIoControl(ahCCDDRV[drvno], IOCTL_WriteLongS0,  // port offset in longs
	&WriteData,
	DataLength,
	NULL, 0, &ReturnedLength, NULL);
	if (!fResult)
	{
	ErrorMsg("WriteLongS0 failed");
	return FALSE;
	}
	*/
	return TRUE;
};  // WriteLongS0
//done
BOOL WriteLongDMA(UINT drvno, ULONG DWData, ULONG PortOff)
{	// writes long to space0 register
	// PortOff: Reg Offset from BaseAdress - in bytes
	// returns TRUE if success
	BOOL fResult = FALSE;
	sDLDATA WriteData;
	DWORD dwStatus = 0;
	PULONG data = &DWData;

	dwStatus = WDC_WriteAddrBlock(hDev, 0, PortOff, sizeof(ULONG), data, WDC_MODE_8, WDC_ADDR_RW_DEFAULT);
	//the second parameter gives the memory space 0:mem mapped cfg/S0-space 1:I/O cfg/S0-space 2:DMA-space
	if (WD_STATUS_SUCCESS != dwStatus)
	{
		ErrLog("Failed to write the AddrBlock of DMA\n"
			"Error 0x%lx - %s\n", dwStatus, Stat2Str(dwStatus));
		WDC_Err("%s", LSCPCIEJ_GetLastErr());
		//old message...i kept it because i dont know what it does
		ErrorMsg("WriteLongDMA failed");
		return FALSE;
	}


	/*
	WriteData.POff = PortOff; // offset in bytes
	WriteData.Data = DWData;
	DataLength = 8;
	fResult = DeviceIoControl(ahCCDDRV[drvno], IOCTL_WriteLongDMAReg,  // port offset in longs
	&WriteData,
	DataLength,
	NULL, 0, &ReturnedLength, NULL);
	if (!fResult)
	{
	ErrorMsg("WriteLongDMA failed");
	return FALSE;
	}
	*/
	return TRUE;
};  // WriteLongDMA
//done
/*
BOOL WriteByteDMA(UINT drvno, USHORT DWData, ULONG PortOff)
{	// writes long to space0 register
// PortOff: Reg Offset from BaseAdress - in bytes
// returns TRUE if success
BOOL fResult = FALSE;
sDLDATA WriteData;
ULONG	DataLength;
DWORD   ReturnedLength;
volatile DWORD dwStatus = 0;
PUSHORT data = &DWData;

dwStatus = WDC_WriteAddrBlock(hDev, 0, PortOff, sizeof(USHORT), data, WDC_MODE_8, WDC_ADDR_RW_DEFAULT);
//the second parameter gives the memory space 0:mem mapped cfg/S0-space 1:I/O cfg/S0-space 2:DMA-space
if (WD_STATUS_SUCCESS != dwStatus)
{
ErrLog("Failed to write the AddrBlock of DMA\n"
"Error 0x%lx - %s\n", dwStatus, Stat2Str(dwStatus));
WDC_Err("%s", LSCPCIEJ_GetLastErr());
//old message...i kept it because i dont know what it does
ErrorMsg("WriteLongDMA failed");
return FALSE;
}
return TRUE;
};
*/
//done
BOOL WriteByteS0(UINT drvno, BYTE DWData, ULONG PortOff)
{	// writes byte to space0 register except r10-r1f
	// PortOff: Reg Offset from BaseAdress - in bytes
	// returns TRUE if success
	BOOL fResult = FALSE;
	sDLDATA WriteData;
	ULONG	DataLength;
	DWORD   ReturnedLength;
	volatile DWORD dwStatus = 0;
	PBYTE data = &DWData;
	ULONG	PortOffset;


	PortOffset = PortOff + 0x80;

	dwStatus = WDC_WriteAddrBlock(hDev, 0, PortOffset, 1/*sizeof(BYTE)*/, data, WDC_MODE_8, WDC_ADDR_RW_DEFAULT);
	//the second parameter gives the memory space 0:mem mapped cfg/S0-space 1:I/O cfg/S0-space 2:DMA-space
	if (WD_STATUS_SUCCESS != dwStatus)
	{
		ErrLog("Failed to write the AddrBlock as an byte in spaceS0\n"
			"Error 0x%lx - %s\n", dwStatus, Stat2Str(dwStatus));
		WDC_Err("%s", LSCPCIEJ_GetLastErr());
		//old message...i kept it because i dont know what it does
		ErrorMsg("WriteByteS0 failed");
		return FALSE;
	}

	//no comparison possible because some Read-Only-Register are changing when we are writing in the same register
	BYTE checkdata;
	ReadByteS0(DRV, &checkdata, PortOff);
	if (*data != checkdata){
		;// WDC_Err("\nWriteByteError:\ndata to write: %x\n", *data);
		//WDC_Err("data read: %x\n", checkdata);
	}


	/*
	//space0 starts at S0-Offset in BAR0
	WriteData.POff = PortOff;
	WriteData.Data = DWData;
	DataLength = sizeof(WriteData);
	fResult = DeviceIoControl(ahCCDDRV[drvno], IOCTL_WriteByteS0,  //
	&WriteData,
	DataLength,
	NULL, 0, &ReturnedLength, NULL);
	if (!fResult)
	{
	ErrorMsg("WriteByteS0 failed");
	return FALSE;
	}
	*/
	return TRUE;
};  // WriteByteS0
/*seriell port programming -°not nedded anymore
BOOL WriteBytePort0(UINT drvno, BYTE DWData, ULONG PortOff)
{	// writes byte to subd with nd clks if VON=hi / adr if VON=lo data
// PortOff: adr data
// returns TRUE if success
BOOL fResult = FALSE;
sDLDATA WriteData;
ULONG	DataLength;
DWORD   ReturnedLength;

WriteData.POff = PortOff & 0xff;
WriteData.Data = DWData;
DataLength = sizeof(WriteData);
fResult = DeviceIoControl(ahCCDDRV[drvno], IOCTL_WriteBytePort0,  //
&WriteData,
DataLength,
NULL, 0, &ReturnedLength, NULL);
if (!fResult)
{
ErrorMsg("WriteBytePort0 failed");
return FALSE;
}
return TRUE;
};  // WriteBytePort0
*/


void AboutDrv(UINT drvno)
{
	USHORT version = 0;
	ULONG S0Data = 0;
	UCHAR udata1, udata2, udata3, udata4 = 0;
	BOOL fResult = FALSE;
	ULONG PortNumber = 0;		// must be 0
	DWORD   ReturnedLength = 0;  // Number of bytes returned
	char pstring[80] = "";
	char wstring[16] = "";
	char astring[3] = "";
	HWND hWnd = GetActiveWindow();
	HDC aDC = GetDC(hWnd);

	/*
	fResult = DeviceIoControl(ahCCDDRV[drvno], IOCTL_GetVersion,
		&PortNumber,        // Buffer to driver.
		4, &version, sizeof(version), &ReturnedLength, NULL);
	if (fResult)
	{ // read driver version via DevIoCtl
		sprintf_s(wstring, 17, "Driver LSCPCI%d", drvno);
		sprintf_s(pstring, 81, " version: 0x%x", version);
		if (MessageBox(hWnd, pstring, wstring, MB_OK | MB_ICONEXCLAMATION) == IDOK) {};
	}
	else
	{
		ErrorMsg("About DeviceIo failed");
	};
	*/

	// read ISA Id from S0Base+7
	ReadLongS0(drvno, &S0Data, 4); // Board ID =5053
	S0Data = S0Data >> 16;

	//or
	//S0Data = (UCHAR)ReadByteS0(8); // ID=53
	sprintf_s(pstring, 80, "     ID = 0x%x", S0Data);
	if (MessageBox(hWnd, pstring, " Board ID=53 ", MB_OK | MB_ICONEXCLAMATION) == IDOK) {};

	//ReadLongIOPort(drvno, &S0Data, 0); //read LCR0 for check length 0xffffffco
	//S0Data = ~S0Data; //length is inverted
	//GS
	S0Data = 0x07FF;

	if (S0Data == 0) { ErrorMsg(" no Space0!"); return; }

	sprintf_s(pstring, 80, "     length = 0x%x", S0Data);
	if (MessageBox(hWnd, pstring, " PCI space0 length=", MB_OK | MB_ICONEXCLAMATION) == IDOK) {};

	if (S0Data >= 0x1F)
	{//if WE -> has space 0x20
		ReadByteS0(drvno, &udata1, 0x1C);
		ReadByteS0(drvno, &udata2, 0x1D);
		ReadByteS0(drvno, &udata3, 0x1E);
		ReadByteS0(drvno, &udata4, 0x1F);
		sprintf_s(pstring, 80, " ven ID = %c%c%c%c", udata1, udata2, udata3, udata4);
		if (MessageBox(hWnd, pstring, " Board vendor=EBST ", MB_OK | MB_ICONEXCLAMATION) == IDOK) {};
	}

	if (S0Data >= 0x3F)
	{//if 9056 -> has space 0x40
		ReadLongS0(drvno, &S0Data, 0x3C);
		sprintf_s(pstring, 80, " board version = 0x%x", S0Data);
		if (MessageBox(hWnd, pstring, " Board version ", MB_OK | MB_ICONEXCLAMATION) == IDOK) {};
	}

	ReleaseDC(hWnd, aDC);
};


/* functions for managing controlbits in CtrlA register

the bits of CtrlA register have these functions:

DB5		DB4		DB3			DB2		DB1		DB0
Slope	-	    TrigOut		XCK		IFC		V_ON
1: pos	1: on   1: high		1: high	1: high	1: high	V=1
0: neg	0: off  0: low		0: low	0: low	0: low	V=VFAK

D7, D8 have no function

+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
void LowSlope(UINT drvno)
{// clear bit D5
	BYTE CtrlA;

	NotBothSlope(drvno);
	ReadByteS0(drvno, &CtrlA, 4);
	CtrlA &= 0x0df;
	WriteByteS0(drvno, CtrlA, 4);
}; //LowSlope

void HighSlope(UINT drvno)
{// set bit D5
	BYTE CtrlA;

	NotBothSlope(drvno);
	ReadByteS0(drvno, &CtrlA, 4);
	CtrlA |= 0x20;
	WriteByteS0(drvno, CtrlA, 4);
}; //HighSlope

void BothSlope(UINT drvno)
{// set bit D4
	BYTE CtrlA;

	HighSlope(drvno);
	ReadByteS0(drvno, &CtrlA, 4);
	CtrlA |= 0x10;
	WriteByteS0(drvno, CtrlA, 4);
}; //BothSlope

void NotBothSlope(UINT drvno)
{// set bit D4
	BYTE CtrlA;
	ReadByteS0(drvno, &CtrlA, 4);
	CtrlA &= 0xEF;
	WriteByteS0(drvno, CtrlA, 4);
}; //NotBothSlope

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Ausgabe eines High-Signals an Pin 17                                      */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void OutTrigLow(UINT drvno)
{
	BYTE CtrlA;
	ReadByteS0(drvno, &CtrlA, 4);
	CtrlA &= 0xf7;
	WriteByteS0(drvno, CtrlA, 4);
};						//OutTrigLow

/*---------------------------------------------------------------------------*/


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Ausgabe eines Low-Signals an Pin 17                                       */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void OutTrigHigh(UINT drvno)
{
	BYTE CtrlA;
	ReadByteS0(drvno, &CtrlA, 4);
	CtrlA |= 0x08;
	WriteByteS0(drvno, CtrlA, 4);
}; //OutTrigHigh


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Ausgabe eines PulseWidth breiten Rechteckpulses an Pin 17                 */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void OutTrigPulse(UINT drvno, ULONG PulseWidth)
{
	OutTrigHigh(drvno);
	Sleep(PulseWidth);
	OutTrigLow(drvno);
};

/*---------------------------------------------------------------------------*/

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Wait for raising edge of Pin #17 SubD = D6 in CtrlA register
ReturnKey is 0 if trigger, else keycode (except space )
if keycode is space, the loop is not canceled

D6 depends on Slope (D5)
HighSlope = TRUE  : pos. edge
HighSlope = FALSE : neg. edge

if ExtTrigFlag=FALSE this function is used to get the keyboard input

+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

void WaitTrigger(UINT drvno, BOOL ExtTrigFlag, BOOL *SpaceKey, BOOL *AbrKey)
// returns if Trigger or Key
{
	BOOL FirstLo = FALSE;
	BOOL HiEdge = FALSE;
	BOOL Abbr = FALSE;
	BOOL Space = FALSE;
	UCHAR ReturnKey = 0;
	BYTE ReadTrigPin = 0;

	do
	{
		if (ExtTrigFlag)
		{
			ReadByteS0(drvno, &ReadTrigPin, 4);
			ReadTrigPin &= 0x040;
			if (ReadTrigPin == 0) FirstLo = TRUE; //first look for lo
			if (FirstLo) { if (ReadTrigPin > 0) HiEdge = TRUE; }; // then look for hi
		}
		else HiEdge = TRUE;

#if _PS2KEYBOARD  //with PS2 keyboard
		ReturnKey = ReadKeyPort(drvno);
		if (ReturnKey == _ScanCode_Cancel) Abbr = TRUE;
		if (ReturnKey == _ScanCode_End) Space = TRUE;
#else	//other keyboard -> do not use highest priority thread
		// or use Sleep to serve the interrupt
		if (GetAsyncKeyState(VK_ESCAPE))  Abbr = TRUE;
		if (GetAsyncKeyState(VK_SPACE))  Space = TRUE;
#endif
	} while ((!HiEdge) && (!Abbr));
	if (Abbr) *AbrKey = TRUE;	//stops immediately
	if (Space) *SpaceKey = TRUE;	//stops after next trigger
};// WaitTrigger


//******************** the triginput has an optional FF to detect short pulses
// the FF is edge triggered and must be reset via RSTrigShort after each pulse to arm it again
// it is enabled once by EnTrigShort()
//********************

void WaitTriggerShort(UINT drvno, BOOL ExtTrigFlag, BOOL *SpaceKey, BOOL *AbrKey)
// returns if Trigger or Key
{
	BOOL FirstLo = FALSE;
	BOOL HiEdge = FALSE;
	BOOL Abbr = FALSE;
	BOOL Space = FALSE;
	UCHAR ReturnKey = 0;
	BYTE ReadTrigPin = 0;

	do
	{
		if (ExtTrigFlag)
		{
			ReadByteS0(drvno, &ReadTrigPin, 4);
			ReadTrigPin &= 0x040;
			if (ReadTrigPin > 0) HiEdge = TRUE;
		}
		else HiEdge = TRUE;

#if _PS2KEYBOARD  //with PS2 keyboard
		ReturnKey = ReadKeyPort(drvno);
		if (ReturnKey == _ScanCode_Cancel) Abbr = TRUE;
		if (ReturnKey == _ScanCode_End) Space = TRUE;
#else	//other keyboard -> do not use highest priority thread
		// or use Sleep to serve the interrupt
		if (GetAsyncKeyState(VK_ESCAPE))  Abbr = TRUE;
		if (GetAsyncKeyState(VK_SPACE))  Space = TRUE;
#endif
	} while ((!HiEdge) && (!Abbr));
	if (Abbr) *AbrKey = TRUE;	//stops immediately
	if (Space) *SpaceKey = TRUE;	//stops after next trigger
	RSTrigShort(drvno);//!
};// WaitTrigger^Short


void EnTrigShort(UINT drvno)
{//use the short trig pulse FF for ext TrigIn
	UCHAR CtrlA;
	ReadByteS0(drvno, &CtrlA, 4);
	CtrlA |= 0x080;	// set trigger path to FF
	CtrlA |= 0x010; // enable FF
	WriteByteS0(drvno, CtrlA, 4);
}; //EnTrigShort

void RSTrigShort(UINT drvno)
{//reset the short trig pulse FF 
	UCHAR CtrlA;
	ReadByteS0(drvno, &CtrlA, 4);
	CtrlA &= 0x0EF; // write CLR to FF
	WriteByteS0(drvno, CtrlA, 4);
	CtrlA |= 0x010; // arm FF again
	WriteByteS0(drvno, CtrlA, 4);
}; //RSTrigShort

void DisTrigShort(UINT drvno)
{//use the direct input for ext TrigIn
	UCHAR CtrlA;
	ReadByteS0(drvno, &CtrlA, 4);
	CtrlA &= 0x07F;	// set trigger path to FF
	CtrlA &= 0x0EF; // clr  FF
	WriteByteS0(drvno, CtrlA, 4);
}; //SetTrigShort


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void CloseShutter(UINT drvno)   // IFC = low
{
	UCHAR CtrlA;
	ReadByteS0(drvno, &CtrlA, 4);
	CtrlA &= 0x0fd;	/* $FD = 1111 1101 */
	WriteByteS0(drvno, CtrlA, 4);
}; //CloseShutter

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void OpenShutter(UINT drvno)   // IFC = high
{
	UCHAR CtrlA;
	ReadByteS0(drvno, &CtrlA, 4);
	CtrlA |= 0x02;
	WriteByteS0(drvno, CtrlA, 4);
}; //OpenShutter


BOOL GetShutterState(UINT drvno)
{
	UCHAR CtrlA;
	ReadByteS0(drvno, &CtrlA, 4);
	CtrlA &= 0x02;
	if (CtrlA == 0) return FALSE;
	return TRUE;
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* set V_ON low (V = V_Fak)                                               */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void V_On(UINT drvno)
{
	UCHAR CtrlA;
	ReadByteS0(drvno, &CtrlA, 4);
	CtrlA |= 0x01;
	WriteByteS0(drvno, CtrlA, 4);
}; //V_On

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* set V_ON high (V = 1)                                                  */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void V_Off(UINT drvno)
{
	UCHAR CtrlA;
	ReadByteS0(drvno, &CtrlA, 4);
	CtrlA &= 0xfe;	// $FE = 1111 1110 
	WriteByteS0(drvno, CtrlA, 4);
}; //V_Off


// optional Opto Couplers
void SetOpto(UINT drvno, BYTE ch)
{//sets signal=low
	BYTE ctrlc;
	ReadByteS0(drvno, &ctrlc, 6);
	if (ch == 2) { ctrlc |= 0x04; }
	else ctrlc |= 0x02;
	WriteByteS0(drvno, ctrlc, 6);
}; //SetOpto


void RsetOpto(UINT drvno, BYTE ch)
{ //sets signal=high
	BYTE ctrlc;
	ReadByteS0(drvno, &ctrlc, 6);
	if (ch == 2) { ctrlc &= 0xfb; }
	else ctrlc &= 0xfd;
	WriteByteS0(drvno, ctrlc, 6);
}; //RsetOpto


BOOL GetOpto(UINT drvno, BYTE ch)
{//no input or low -> high / high input -> low 
	BYTE ctrlc;
	ReadByteS0(drvno, &ctrlc, 6);
	if (ch == 2) { ctrlc &= 0x04; }
	else ctrlc &= 0x02;
	if (ctrlc>0) return TRUE;
	return FALSE;
}; //GetOpto

void SetDAT(UINT drvno, ULONG datin100ns)
{//delay after trigger HW register
	datin100ns |= 0x80000000; // enable delay
	WriteLongS0(drvno, datin100ns, 0x20);
}; //SetDAT

void RSDAT(UINT drvno)
{//delay after trigger HW register
	WriteLongS0(drvno, 0, 0x20);
}; //RSDAT


void SetEC(UINT drvno, ULONG ecin100ns)
{//delay after trigger HW register
	ULONG data = 0;
	ReadLongS0(drvno, &data, 0x24);
	ecin100ns |= data;
	ecin100ns |= 0x80000000; // enable delay
	WriteLongS0(drvno, ecin100ns, 0x24);
}; //SetDAT



void SetTORReg(UINT drvno, BYTE fkt)
{
	BYTE val = 0; //defaut XCK= high during read
	BYTE val2 = 0;
	if (fkt == 1) val = 0x80; // set to REG -> OutTrig
	if (fkt == 2) val = 0x10; // set to FFRead
	if (fkt == 3) val = 0x20; // set to TIN
	if (fkt == 4) val = 0x40; //set to EC
	if (fkt == 5) val = 0x08; //set to DAT

	ReadByteS0(drvno, &val2, 0x2B);
	val2 &= 0x03; //dont disturb lower bits
	val |= val2;
	WriteByteS0(drvno, val, 0x2B);
}//SetTORReg

void SetISPDA(UINT32 drvno, BOOL set)
{//set bit if PDA sensor - used for EC and IFC
	BYTE val = 0;
	ReadByteS0(drvno, &val, 0x2B);
	if (set != 0) { val |= 0x02; }
	else val &= 0xfd;
	WriteByteS0(drvno, val, 0x2B);
	OpenShutter(drvno); //enable output
}//SetISPDA

void SetISFFT(UINT32 drvno, BOOL set)
{//set bit if FFT sensor - used for vclks and IFC
	// also OpenShutter must be set!
	BYTE val = 0;
	ReadByteS0(drvno, &val, 0x2B);
	if (set != 0) { val |= 0x01; }
	else val &= 0xfe;
	WriteByteS0(drvno, val, 0x2B);
}//SetISFFT

void RsTOREG(UINT32 drvno)
{// reset TOREG
	WriteByteS0(drvno, 0, 0x2B);
}



//**************************  new setup of fiber link camera
// send setup	d0:d15 = data for AD-Reg  ADS5294
//				d16:d23 = ADR of  AD-Reg
//				d24 = ADDR0		AD=1
//				d25 = ADDR1		AD=0
//				d26 makes load pulse
//				all written to DB0 in Space0 = Long0
//				for AD set maddr=01
void SendFLCAM(UINT32 drvno, BYTE maddr, BYTE adaddr, USHORT data)
{
	ULONG ldata = 0;

	ldata = maddr;
	ldata = ldata << 8;
	ldata |= adaddr;
	ldata = ldata << 16;
	ldata |= data;
	WriteLongS0(drvno, ldata, 0x0);
	ldata |= 0x4000000;		//load val
	WriteLongS0(drvno, ldata, 0x0);
	ldata = 0;		//rs load
	WriteLongS0(drvno, ldata, 0x0);

}//SendFLCAM



void ClrRead(ULONG drvno, ULONG fftlines, ULONG zadr, ULONG ccdclrcount)
//normal clear for Kamera is a complete read out
//most cams needs up to 10 complete reads for resetting the sensor
//depends how much it was overexposured
{
	UINT i;
	SetIntFFTrig(drvno);
	StartFFTimer(drvno, 1000);
	//	RSFifo(drvno);
	for (i = 0; i<ccdclrcount; i++)
	{
		SWTrig(drvno);
		while (FlagXCKI(drvno))
		{
		}//wait until its ready
	}
	StopFFTimer(drvno);
	RSFifo(drvno);
}; //ClrRead



void ClrShCam(UINT drvno, UINT zadr) //clear for Shutter cameras
{
	pArrayT dummy = NULL;
	CloseShutter(drvno);              //IFC=low
	Sleep(5);
	//GETCCD(drvno, dummy, 0, -1, zadr);
	Sleep(5);
	OpenShutter(drvno);               //IFC=High
	Sleep(5);
}; //ClrShCam


UCHAR ReadKeyPort(UINT drvno)
{		//Reads PS2 Key directly -> very low jitter
	// !!! works with PS2 keyboard only !!!!!!!!
	// on WINNT, getasynckeystate does not work with highest priority

	UCHAR Data = 0;
	BOOL fResult = FALSE;
	DWORD   ReturnedLength;
	ULONG	PortOffset = 0; //has no function

	fResult = DeviceIoControl(ahCCDDRV[drvno], IOCTL_ReadKey,  // read one byte
		&PortOffset,        // Buffer to driver.
		sizeof(PortOffset),
		&Data, sizeof(Data), &ReturnedLength, NULL);
	if (!fResult)
	{
		ErrorMsg("Read Key Ioctl failed");
		exit(0);
	};
	return Data;
};  // ReadKeyPort




/*
void SendCommand(UINT drvno, BYTE adr, BYTE data)
//for programming of seriell port for AD98xx
{//before calling IFC has to be set to low
BYTE regorg=0;
BYTE reg=0;
//	Sleep(1);
WriteByteS0(drvno,adr,0); // write address to bus

ReadByteS0(drvno,&regorg,5);// get reg data
reg = regorg | 0x20;
WriteByteS0(drvno,reg,5); // // ND lo pulse
WriteByteS0(drvno,regorg,5); // ND hi pulse

WriteByteS0(drvno,data,0);	// write data to bus

WriteByteS0(drvno, reg ,5); // ND lo pulse
//	Sleep(1);
WriteByteS0(drvno,regorg,5);	// ND hi pulse
//	Sleep(1);
}//SendCommand
*/

void RSEC(UINT drvno)
//reset EC register to zero (enables programming IFC via register)
{
	/*
	ULONG reg=0;
	ReadLongIOPort(drvno,&reg,0); //read LCR0 for check length 0xffffffco
	reg	=  ~reg; //length is inverted
	if (reg>0x34)
	{
	WriteByteS0(drvno,0,0x24);	// EC=0
	WriteByteS0(drvno,0,0x25);	// EC=0
	} */

	WriteLongS0(drvno, 0, 0x24);
}


void SendCommand(UINT drvno, BYTE adr, BYTE data)
//for programming of seriell port for AD98xx
{//before calling IFC has to be set to low
	//and EC must be set 0 with 	RSEC(drvno);
	BYTE regorg = 0;
	BYTE regh = 0;
	BYTE regl = 0;
	BYTE regno = 0;
	//	Sleep(1);
	WriteByteS0(drvno, adr, 0); // write address to bus


	//clk with ND
		regno = 5;
		ReadByteS0(drvno, &regorg, regno);// get reg data
		regh = regorg & 0xDF;			//ND_DIS clk is inverted
		regl = regorg | 0x20;
	

	WriteByteS0(drvno, regh, regno); // // ND/VON hi  setup

	WriteByteS0(drvno, regl, regno); // // ND/VON lo 
	WriteByteS0(drvno, regh, regno); // // ND/VON hi

	WriteByteS0(drvno, data, 0);	// write data to bus

	WriteByteS0(drvno, regl, regno); // // ND/VON lo 
	WriteByteS0(drvno, regh, regno); // // ND/VON hi


	WriteByteS0(drvno, regorg, regno); // ND/VON restore
	WriteByteS0(drvno, 0, 0);	// write 0 to bus

}//SendCommand 




void SetHiamp(UINT drvno, BOOL hiamp)
{

	//	if (_HA_IR) CloseShutter(drvno);// IR uses #11 or #14
	if (hiamp) { V_On(drvno); }	//standard use #11 VON
	else { V_Off(drvno); }
}//SetHiamp


void SetAD(UINT drvno, BYTE adadr, BYTE addata)
//for AD98xx
{
	BYTE reg = 0;
	CloseShutter(drvno);// IFC=lo
	RSEC(drvno);
	Sleep(1);
	reg = 0;

#define _OLDSEND FALSE // TRUE for old send mechanism PAL 94.1

#if (!_OLDSEND)
	reg = adadr << 4;
	if ((adadr & 0x80) == 0x80) //neg
		reg |= 0x01; //D8
#else
	reg = adadr;
	if ((adadr & 0x80) == 0x80) //neg
		reg |= 0x80; //D8
#endif

	SendCommand(drvno, 0xA1, reg); //send adr
	SendCommand(drvno, 0xB2, addata); //send data
	SendCommand(drvno, 0xC3, 1); //load SH
	SendCommand(drvno, 0xC3, 2); // SH send to AD
	Sleep(1);
	OpenShutter(drvno);		// IFC=hi
	Sleep(1);
}//SetAD



void SetADOff(UINT drvno, BYTE ofs, BOOL pos)
//for AD98xx
{ //0=1x .. 64=6x
	BYTE sign_adr = 5;
	if (pos) {}
	else sign_adr |= 0x80; //add sign bit in highest bit
	SetAD(drvno, sign_adr, ofs);
}//SetADOff


void SetADAmpRed(UINT drvno, BYTE amp)
//for AD98xx
{ //0=1x .. 64=6x
	if (amp >= 64) amp = 0x3F;
	SetAD(drvno, 2, amp);
}//SetADAmpRed


void SetAD16Default(UINT drvno, UINT cds)
{// for AD98xx
	UCHAR db;
	// SHA mode
	//db = 0xc8 ;  //1100 1000; 

	// cds mode Vin=4 0xD8
	// cds Vin=2 0x58
	// cds 
	if (cds == 2)
		db = 0xC8;  // sha mode, 16bit
	else
		db = 0xD8;  // cds mode, 16bit


	//	if (res==8 )	db |= 01;  //set to 8 bit mode

	SetAD(drvno, 0, db);
	SetADAmpRed(drvno, 0x0);
	SetADOff(drvno, 0, TRUE);
	SetDA(drvno, 0, 2);
}//SetAD16Default

void SetDA(UINT drvno, BYTE gain, BYTE ch)
//AD98xx setup
{ // ch=1->A  ,  CH=2->B
	BYTE lb = 0;
	BYTE hb = 0x40; // buffered
	BYTE g = 0;

	if (ch == 2) hb |= 0x80;  //chb=2


	g = gain >> 4;
	hb |= g;
	g = gain << 4;
	lb |= g;

	CloseShutter(drvno);// IFC=lo
	RSEC(drvno);
	Sleep(1);
	SendCommand(drvno, 0xA1, hb); //send hi byte
	SendCommand(drvno, 0xB2, lb); //send lo byte
	SendCommand(drvno, 0xC3, 1); //load SH
	SendCommand(drvno, 0xC3, 4); // SH send to DA
	Sleep(1);
	OpenShutter(drvno);		// IFC=hi
	Sleep(1);
}//SetDA







BOOL CheckFFTrig(UINT drvno) //ext trigger in FF for short pulses
{	// CtrlA register Bit 6 reads trigger FF 
	// if CtrlA bit 4 was set FF is activated, write 0 to bit4 clears&disables FF
	BYTE ReadCtrlA = 0;
	BYTE data = 0;
	ReadByteS0(drvno, &ReadCtrlA, 4);
	ReadCtrlA |= 0x080; // set to edge triggered
	ReadCtrlA |= 0x010; //activate TrigFF
	WriteByteS0(drvno, ReadCtrlA, 4);
	if ((ReadCtrlA & 0x40) == 0x040)
	{// D6 high, recognize pulse
		data = ReadCtrlA & 0x0EF;
		WriteByteS0(drvno, data, 4);//clears Trigger FF	
		data = ReadCtrlA | 0x010;
		WriteByteS0(drvno, data, 4);//activate again
		return TRUE;
	}
	return FALSE;
}//CheckFFTrig


// ExpTime is passed as global var here
// function is not used in WCCD
#ifndef ExpTime
ULONG ExpTime; //in micro sec - needed only in DLL, defined in DLL.h
#endif




//FIFO
//***************  Fifo only Functions   ***************

void StartReadWithDma(UINT drvno){

	//old startringreadthread routine
//	dwDMASubBufSize = (DMABufSizeInScans + 10) * aPIXEL[drvno] * sizeof(USHORT);// +10;//+100 safty first if it is not right calculated
//	dwDMASubBufSize = DMABufSizeInScans  * _PIXEL * sizeof(USHORT);// +10;//+100 safty first if it is not right calculated
	if (_HWCH2) dwDMASubBufSize *= 2;

	if (!DMAAlreadyStarted){
		if (!SetupPCIE_DMA(DRV)) return;
		DMAAlreadyStarted = TRUE;
	}

	//old ReadRingThread Routine
	//read the actual trigger input of trigger or OPTO1 & 2 in CtrlC
	if (RingCtrlRegOfs>0) ReadByteS0(Ringdrvno, &RingCtrlReg, RingCtrlRegOfs);

	//old ReadPCIeFifo Routine
	pArrayT pReadArray;
	pArrayT	pDiodenBase;
	pArrayT	pDiodenBase2;

	ULONG length = 0;
	ULONG i = 0;
	ULONG lwritten = 0;
	BOOL addalloc = FALSE;

	if (!aINIT[drvno]) return FALSE;	// return with error if no init

	//pReadArray = pDMASubBuf;
	//	pReadArray = pReadArray + (db-1) * pixel;
	pDiodenBase = pReadArray;

	if (_FKT == 0) // set array to 0
	{
		for (i = 0; i< _PIXEL; i++)
			*(pReadArray++) = 0;
		return ;
	}

	if (_FKT == 5) // set array to i
	{
		for (i = 0; i< _PIXEL; i++)
			*(pReadArray++) = (USHORT)i;
		return ;
	}

	if (_FKT>9)
		return ;  // function not implemented


	if ((_FKT == 2) || (_FKT == -1)) //read in our local array ladioden - add and clrread
	{
		//alloc local array dioden, so we don't overwrite our DIODEN
		pReadArray = (USHORT)malloc(aPIXEL[drvno]);
		if (pReadArray == 0)
		{
			ErrorMsg("alloc ADD/CLR Buffer failed");
			return ;
		}
		addalloc = TRUE;
	}

	if (_FKT == -1)
	{ // return , nothing else to do
		if (addalloc) free(pReadArray);
		return ;
	}

	if (_RESORT) Resort(drvno, pReadArray, pReadArray);  //pixel resort




	//clrread and add fkt=-1 and 2 could not be implemented with dma
	//so we do it here after reading
	if (_FKT == 2) // we must now add our data to DIODEN for online add
	{
		pDiodenBase2 = pReadArray;
		for (i = 0; i<_PIXEL; i++)
			* (pDiodenBase++) += *(pDiodenBase2++);
	}

	if (addalloc) free(pReadArray);
}//StartReadWithDma



//  read a range from start to stop relative to actual Counter to user buffer
UINT8 ReadRingBlock(void* pdioden, INT32 start, INT32 stop)
// when calling this function, the actual RingWRCnt value is used as relative zero
// if start or stop is negative it is in the past 
// if start or stop is positive it is in the future - waits until stop reached
// if it is 0 the actual line RingWRCnt is used
// returns 0 if no error, else error codes see below
// in fact if stop>0 thread is waiting until stop reached
// after that the last range of lines is copied to user buffer
{
	//set globals
	INT32 range = 0;
	ULONG pixel = aPIXEL[Ringdrvno];

	//here the user buffer is set
	pUserBuf = (PUCHAR)pdioden;
	UserBufValid = TRUE;

	RingFetchFutureAct = FALSE;
	RingFutureWrCnt = 0;

	RingCopyStop = stop;
	RingCopyAct = FALSE;

	//check range
	range = stop - start + 1;
	RingCopyRange = range;

	//check for size with reserve 50, ringbuffer must be bigger then userbuf
	if (range>RingFifoDepth - 2)
	{
		ErrorMsg("ring buffer depth too small");
		RingThreadOn = FALSE;
		return 1; // range bigger then ringbuffer
	}
	if (range <= 0)
	{
		ErrorMsg("ring buffer range zero");
		RingThreadOn = FALSE;
		return 2; // range bigger then ringbuffer
	}
	if (stop<start)
	{
		ErrorMsg("stop must be > start");
		RingThreadOn = FALSE;
		return 3; // range bigger then ringbuffer
	}
	if (!RingThreadOn) // thread is not on
	{
		ErrorMsg("thread not running");
		return 4; // thread is not on
	}

	if (stop> 0) //in the future: wait until end reached
	{// ringthread is counting from now up to stop
		RingFetchFutureAct = TRUE;
		do
		{
			//check for ESC keypressed
		} while ((RingFetchFutureAct) && (RingThreadOn));//wait for readthread to reach stop
	}


	//wait until 1st run reached range
	do {} while (RingFirstRun< range + 5);

	//ValMsg(range);

	RingCopyAct = TRUE; //start ring copy
	//wait until data is copied to userbuf
	do {} while ((RingCopyAct) && (RingThreadOn));

	return 0;
}  // ReadRingBlock


void CopyRingtoUserBuf(INT32 fetchzero)
{	//reads fifo data to user buffer dioden, fetchzero is the actual ring pointer
	ULONG pixel = aPIXEL[Ringdrvno];
	ULONG linesize = pixel * sizeof(ArrayT);
	INT32 blocksize1 = 0;
	INT32 blocksize2 = 0;
	INT32 range = 0;
	INT32 range1 = 0;
	INT32 range2 = 0;
	INT32 lno = 0;
	INT32 lno2 = 0;
	PUCHAR puser = NULL;


	//in the first run buffer is not set
	//don't copy if so
	if (UserBufValid == FALSE) return;
	puser = pUserBuf;
	if (_HWCH2) linesize *= 2;
	range = RingCopyRange;

	//keep actual ring pointer fetchzero, this is our trigger point
	//if (RingWRCnt<fetchzero+stop) return 3; //user stop

	//if counter was wrapped at 0
	//wrap at 0 <-> RingFifoDepth
	if (fetchzero + 1 - range < 0)
	{// is wrapped around 0
		//	wrapped=TRUE;

		range2 = fetchzero + 1;
		range1 = range - range2;

		//	lno = RingFifoDepth-range+fetchzero+1;
		lno = RingFifoDepth - range1;
		lno2 = 0;

		blocksize1 = linesize * range1; //block in bytes
		blocksize2 = linesize * range2;

		//tests
		//memset((PUCHAR) puser ,0x88,blocksize1);//range1
		//memset((PUCHAR) puser + blocksize1 ,0xee,blocksize2);//range2
		//ValMsg(puser);
		//memset((PUCHAR) puser ,range1,1); //show split ranges in 1st pixel
		//memset((PUCHAR) puser+1,range2,1);

		memcpy((PUCHAR)pUserBuf, (PUCHAR)pRingBuf + lno*linesize, blocksize1);//before depth
		memcpy((PUCHAR)pUserBuf + blocksize1, (PUCHAR)pRingBuf + lno2*linesize, blocksize2);//from zero
	}
	else //not wrapped
	{
		//	wrapped = FALSE;
		lno = fetchzero + 1 - range;
		blocksize1 = linesize * range;
		//memset((PUCHAR) pUserBuf,0x22,blocksize1);
		memcpy((PUCHAR)pUserBuf, (PUCHAR)pRingBuf + lno*linesize, blocksize1);
	}

	UserBufValid = FALSE; //set FALSE here if next buffer has a different address
} // CopyRingtoUserBuf



//***** Ring Fifo fkts **************************************
//starts an own thread for writing to a ring buffer of size FifoDepth
//allocates the buffer here
//read is done by ReadRing if tread>twrite
// or by ReadLastRing if tread<twrite

//replaced by StartReadWithDma
void __cdecl ReadRingThread(void *dummy)

{// max priority
	UINT i = 0;
	UINT j = 0;
	int k = 0;
	BOOL Space = FALSE;
	BOOL Abbruch = FALSE;
	//alloc Fifo
	ULONG pixel = aPIXEL[Ringdrvno]; //use _NO_TLPS instead
	ULONG linesize = pixel * sizeof(ArrayT);

	volatile BYTE linestofetch = 0;
	ULONG lwritten = 0;

	if (_HWCH2) linesize *= 2;
	MaxLineCounter = 0;

#define testdata FALSE  //for test purpose only, generates dummy data


	//SetThreadIdealProcessor(GetC urrentThread(),3);


	SetPriority(READTHREADPriority);
#if (_ERRTEST)
	ErrCnt = 0;
#endif
	RingFirstRun = 0;
	RingWRCnt = -1;
	RingThreadOFF = FALSE;

	RingFetchFutureAct = FALSE;
	RingFutureWrCnt = 0;
	RingCopyAct = FALSE;
	RingCtrlReg = 0;


	// Timer on loop
	RingThreadOn = TRUE;// before loop
	while (RingThreadOn == TRUE) // || FFValid(FFdrvno))
	{
		//read the actual trigger input of trigger or OPTO1 & 2 in CtrlC
		if (RingCtrlRegOfs>0) ReadByteS0(Ringdrvno, &RingCtrlReg, RingCtrlRegOfs);

		linestofetch = ReadFFCounter(Ringdrvno);
		//if(linestofetch > 0)WDC_Err("linestofetch: %i \n", linestofetch);
		//	if (FFValid(Ringdrvno))
		//	if (linestofetch!=0)
		
		if (linestofetch > 0)
		{
			//linestofetch = ReadFFCounter(Ringdrvno);
			//keep and show how many lines were written to fifo
			if (MaxLineCounter<linestofetch) MaxLineCounter = linestofetch;

			for (i = 1; i <= linestofetch; i++)
			{//read all whats there
				RingFirstRun += 1; //count first run before usercopy may start.
				if (RingFirstRun>MAXINT) RingFirstRun = MAXINT;
				RingWRCnt += 1;
				if (RingWRCnt>RingFifoDepth - 1) RingWRCnt = 0;//wrap counter
#if (!testdata)
				//read fifo is DMA so here it can be back before ready

				//			WriteFile(ahCCDDRV[Ringdrvno], pRingBuf + RingWRCnt*pixel, pixel*2, &lwritten, NULL); //write to PC RAM
				//pRingBuf will crash even if written to DMA write addr reg
				//this function wraps the call, pixel does not matter here, but must be > as 1088
				ReadPCIEFifo(Ringdrvno, (pArrayT)pRingBuf + RingWRCnt*pixel, _FKT); //*linesize in short

				//			ReadFifo(Ringdrvno, (PUCHAR) pRingBuf+RingWRCnt*linesize, _FKT);	
				//			memset((PUCHAR) pRingBuf+RingWRCnt*linesize,RingWRCnt,linesize);//test cnt
				//			*((PUINT)pRingBuf+RingWRCnt*pixel+4) = RingWRCnt; //test: write cnter to pixel=4

				//if ReadRingBlock want's to read in future
				if (RingFetchFutureAct)
				{
					RingFutureWrCnt += 1;
					if (RingFutureWrCnt>RingCopyStop)
					{
						RingFutureWrCnt = 0;
						RingFetchFutureAct = FALSE;
					}
				}

#else		//testdata
				memset(pRingBuf + RingWRCnt*pixel, RingWRCnt, pixel * 2);//sets bytes
#endif


#if (_TESTRUP) //set outtrig if pixel 1 and 256 found
				if (*(pRingBuf + RingWRCnt*pixel + 4) == 1) OutTrigHigh(Ringdrvno);
				if (*(pRingBuf + RingWRCnt*pixel + 4) == Roilines - 2) OutTrigLow(Ringdrvno);
#endif

#if (_ERRTEST) //test data for integrity	
				//for (k=100;k<4500;k++)
				{
					//if (*(pRingBuf + RingWRCnt*_NO_TLPS * 128 / 2 + 7) < 0x4000) //wert etwa > 0x4000
					if (*(pDMASubBuf/*pRingBuf + RingWRCnt*pixel*/ + 1083) != 539)
					{//FetchActLine=TRUE;
						ErrVal = *(/*pRingBuf + RingWRCnt*pixel*/pDMASubBuf + 1083);
						ErrCnt += 1;
					}}
#else
				//FetchActLine=FALSE;// dont display if no error
#endif

			}//for (i=1;i<=linestofetch;i++)
		}//if (FFValid(Ringdrvno))
		else //no valid line, use time to check for copy user data
		{
			//be shure the first range is in ring buffer before getting here (1st call)
			if (RingCopyAct == TRUE)// if userbuf needs data, copy it here
			{//todo: if range is too big (>1000) the copy should be broken into smaller pieces
				// one block per loop ..
				CopyRingtoUserBuf(RingWRCnt);
				RingCopyAct = FALSE;
			}
			//we use waittrigger just for checking keys
			//WaitTrigger(Ringdrvno,FALSE,&Space, &Abbruch);
			//if (Abbruch==TRUE)RingThreadOn=FALSE;
		}//else
		//	if (RELEASETHREADms>=0) Sleep(RELEASETHREADms); // <0 don' release		

	}//while
	free(pRingBuf);
	RingCopyAct = FALSE;
	UserBufValid = FALSE;
	ResetPriority();
	RingThreadOFF = TRUE;

	_endthread();
}//ReadRingThread


//replaced by StartReadWithDma
void StartRingReadThread(UINT drvno, ULONG ringfifodepth, ULONG threadp, __int16 releasems)
{
	ULONG pixel = aPIXEL[drvno];
	ULONG linesize = pixel * sizeof(ArrayT);
	//set globals in BOARD
	Ringdrvno = drvno;
	RingRDCnt = 0;
	RingWRCnt = 0;

	ULONG ctrlcode = 0;
	ULONG fResult = 0;
	ULONG Errorcode = 0;
	ULONG ReturnedLength = 0;

	dwDMASubBufSize = 100 * RAMPAGESIZE * 2;// 100: ringbufsize 2:because  we need the size in bytes
	if (!DMAAlreadyStarted){
		SetupPCIE_DMA(DRV);
		DMAAlreadyStarted = TRUE;
	}

#if (_TESTRUP)
	Roilines = ROILINES;
#endif	
	READTHREADPriority = threadp; // pass vals as globals
	RELEASETHREADms = releasems;

	if (_HWCH2) linesize *= 2;
	pRingBuf = (pArrayT)calloc(linesize, ringfifodepth); //allooc buffer
	if (pRingBuf == 0) {
		ErrorMsg("alloc RingFifo Buffer failed, abort function");
		return;
	}
	RingCopyAct = FALSE;

	/*
	ctrlcode = pRingBuf;//set data array address
	fResult = DeviceIoControl(ahCCDDRV[1], IOCTL_SET_DMA_WRITE_ADDR,
	&ctrlcode,        // Buffer to driver.
	sizeof(ctrlcode),
	&Errorcode, sizeof(Errorcode), &ReturnedLength, NULL);
	if (!fResult)  { ErrorMsg("SetupAddress failed"); };

	if (!DMAAlreadyStarted){
	SetupPCIE_DMA(DRV);
	DMAAlreadyStarted = TRUE;
	}
	*/
	//StartDMA first time
	//StartPCIE_DMAWrite(DRV);

	RingFifoDepth = ringfifodepth;
	//ReadRingThread(NULL);
	_beginthread(ReadRingThread, 0, NULL); // starts get loop in an own thread

	return;
}
//replaced by StartReadWithDma
void StopRingReadThread(void)
{
	RingCopyAct = FALSE;
	RingThreadOn = FALSE;// global variable ends thread and frees mem
}//StopRingFFTimer


//  call of the read function if write is faster then read
// ReadRingFifoThread is writing fast to the ring buffer
// if global flag FetchActLine is set, the thread copies last line to pCopyDispBuf

void StartFetchRingBuf(void)
{//pdioden points to the user buffer
	RingCopyAct = TRUE; //set global flag starting the copy
}

BOOL RingThreadIsOFF(void)
{//return state of thread
	//use this for sync to outside
	return RingThreadOFF;
}

// FetchLastRingLine gets latest valid displ buf
void FetchLastRingLine(void* pdioden)
{	//reads displ buf data to user buffer dioden
	int i = 0;
	ULONG pixel = aPIXEL[Ringdrvno];
	//ULONG linesize = pixel * sizeof(ArrayT);
	//if (_HWCH2) linesize *=2;

	//memset(pdioden,16, pixel*2 +2 );//*range);
	ReadRingBlock(pdioden, 0, 0);
};  // FetchLastRingLine


ULONG GetLastMaxLines(void)
{	//returns the max no. of lines which accumulated
	return MaxLineCounter;
};  // GetLastMaxLines

UINT64 GetISRTime(void)
{	//returns the timespan for the ISR
	return MaxISRTime;
};

//  call of the read function if write is slower then read
void ReadRingLine(void* pdioden, UINT32 lno)
{	//reads fifo data to user buffer dioden
	ULONG pixel = aPIXEL[Ringdrvno];
	ULONG linesize = pixel * sizeof(ArrayT);

	memcpy(pdioden, pRingBuf + lno*pixel, linesize);
};  // ReadRingLine




ULONG ReadRingCounter()
{
	ULONG diff = 0;
	diff = RingWRCnt - RingRDCnt;
	return  diff;
}//ReadRingCounter

BOOL RingValid()
{
	if ((RingWRCnt - RingRDCnt) == 0) { return FALSE; }
	else return TRUE;
}//RingValid


BOOL RingBlockTrig(UINT8 ch)
{	//return state of trigger in signal
	//global value RingCtrlReg is updated in every loop of ringreadthread
	//in ringreadthread also the board drv is set
	//ch=0 -> no read of state is performed
	//ch=1 is pci tig in
	//ch=2 is opto1
	//ch=3 is opto2
	BOOL state = FALSE;
	RingCtrlRegOfs = 0;//disable update
	switch (ch)
	{
	case 1:
		RingCtrlRegOfs = 4;//CtrlA
		if ((RingCtrlReg & 0x40)>0) return TRUE;
		break;
	case 2:
		RingCtrlRegOfs = 6;//CtrlC
		if ((RingCtrlReg & 0x02)>0) return TRUE;
		break;
	case 3:
		RingCtrlRegOfs = 6;//CtrlC
		if ((RingCtrlReg & 0x04)>0) return TRUE;
		break;
	}
	return FALSE;
}


void SetExtSWTrig(BOOL ext)
{//set the global flag - used in ringreadthread
	if (ext) RRT_ExtTrigFlag = TRUE;
	else RRT_ExtTrigFlag = FALSE;
}//SetExtSWTrig


//*************** Hardware Fifo fkts ******************

void StartFFTimer(UINT drvno, ULONG exptime)
{//exptime in microsec
	ULONG data = 0;
	ReadLongS0(drvno, &data, 0x08); //reset	
	data &= 0xF0000000;
	data = exptime & 0x0FFFFFFF;
	data |= 0x40000000;
	WriteLongS0(drvno, data, 0x08); //set
}


void SWTrig(UINT drvno)
{//start 1 write trigger to FIFO by software
	UCHAR reg = 0;
	//	ReadByteS0(drvno,&reg,11);  //enable timer
	//	reg |= 0x40;  
	//	WriteByteS0(drvno,reg,11);	
	ReadByteS0(drvno, &reg, 0x12);
	reg |= 0x40;
	WriteByteS0(drvno, reg, 0x12); //set Trigger
	reg &= 0xBF;
	WriteByteS0(drvno, reg, 0x12); //reset
}


void StopFFTimer(UINT drvno)
{
	BYTE data = 0;
	ReadByteS0(drvno, &data, 11);
	data &= 0xBF;
	WriteByteS0(drvno, data, 11);
}


BOOL FFValid(UINT drvno)
{	// not empty & XCK = low -> true
	BYTE data = 0;
	ReadByteS0(drvno, &data, 0x13);
	data &= 0x80;
	if (data>0) return TRUE;

	return FALSE;
}


BOOL FFFull(UINT drvno)
{	// Fifo is full
	BYTE data = 0;
	ReadByteS0(drvno, &data, 0x13);
	data &= 0x20;
	if (data>0) return TRUE; //empty

	return FALSE;
}

BOOL FFOvl(UINT drvno)
{	// had Fifo overflow
	BYTE data = 0;
	ReadByteS0(drvno, &data, 0x13);
	data &= 0x08; //0x20; if not saved
	if (data>0) return TRUE; //empty

	return FALSE;
}

BOOL FlagXCKI(UINT drvno)
{	// XCKI write to FIFO is active 
	BYTE data = 0;
	ReadByteS0(drvno, &data, 0x13);
	data &= 0x10;
	if (data>0) return TRUE; //is running

	return FALSE;
}

void RSFifo(UINT drvno)
{	//reset FIFO and FFcounter
	BYTE data = 0;
	ReadByteS0(drvno, &data, 0x12);
	data |= 0x80;
	WriteByteS0(drvno, data, 0x12);
	data &= 0x7F;
	WriteByteS0(drvno, data, 0x12);
}


void DisableFifo(UINT drvno)
{	//reset FIFO and FFcounter
	BYTE data = 0;
	ReadByteS0(drvno, &data, 0x12);
	data |= 0x80;
	WriteByteS0(drvno, data, 0x12);
	//	data &= 0x7F;
	//	WriteByteS0(drvno,data,0x12);
}


void EnableFifo(UINT drvno)
{	//reset FIFO and FFcounter
	BYTE data = 0;
	ReadByteS0(drvno, &data, 0x12);
	//	data |= 0x80;
	//	WriteByteS0(drvno,data,0x12);
	data &= 0x7F;
	WriteByteS0(drvno, data, 0x12);
}

void SetExtFFTrig(UINT drvno)  // set external Trigger
{
	BYTE data = 0;
	ReadByteS0(drvno, &data, 11);
	data |= 0x80;
	WriteByteS0(drvno, data, 11);

}//SetExtFFTrig


void SetIntFFTrig(UINT drvno) // set internal Trigger
{
	BYTE data = 0;
	ReadByteS0(drvno, &data, 11);
	data &= 0x7F;
	WriteByteS0(drvno, data, 11);
}//SetIntFFTrig


BYTE ReadFFCounter(UINT drvno)
{   //count number of lines in FIFO 
	//max. 16 || capacity of FIFO /(pixel*sizeof(ArrayT)) (7205=8k)
	//new: if _CNT255 ff counts up to 255
	BYTE data = 0;
	ReadByteS0(drvno, &data, 0x14);
	if (_CNT255) {}
	else data &= 0x0f;
	return data;
}

void SetupVCLKReg(UINT drvno, ULONG lines, UCHAR vfreq)
{
	FFTLINES = lines; //set global var
	if (!HA_MODULE)
	{
		WriteLongS0(drvno, lines * 2, 0x18);// write no of vclks=2*lines
		WriteByteS0(drvno, vfreq, 0x1b);// write v freq
		VFREQ = vfreq;//keep freq global
	}
}//SetupVCLKReg

void SetupVCLKrt(ULONG vfreq)
{
	VFREQ = vfreq;// VFREQ is global in BOARD
}//Setup VFREQ from setup

//not yet working
void SetupVPB(UINT drvno, UINT range, UINT lines, BOOL keep)
{	//partial binning in registers R10,R11 and and R12
	//range: specifies R 1..5
	//lines: number of vertical clks for next read
	//keep: TRUE if scan should be written to FIFO
	ULONG adr = 0;
	lines *= 2; //vclks=lines*2
	switch (range)
	{
	case 1:	adr = 0x40;
		if (keep) { lines |= 0x8000; }
		else  { lines &= 0x7fff; }
		break;
	case 2:	adr = 0x42;
		if (keep) { lines |= 0x8000; }
		else  { lines &= 0x7fff; }
		break;
	case 3:	adr = 0x44;
		if (keep) { lines |= 0x8000; }
		else  { lines &= 0x7fff; }
		break;
	case 4:	adr = 0x46;
		if (keep) { lines |= 0x8000; }
		else  { lines &= 0x7fff; }
		break;
	case 5:	adr = 0x48;
		if (keep) { lines |= 0x8000; }
		else  { lines &= 0x7fff; }
		break;
	case 6:	adr = 0x4A;
		if (keep) { lines |= 0x8000; }
		else  { lines &= 0x7fff; }
		break;
	case 7:	adr = 0x4C;
		if (keep) { lines |= 0x8000; }
		else  { lines &= 0x7fff; }
		break;
	case 8:	adr = 0x4E;
		if (keep) { lines |= 0x8000; }
		else  { lines &= 0x7fff; }
		break;
	}
	//TODO make function write word or split in writebytes
	//WriteWordS0(drvno, lines, adr);// write range
}// SetupVPB


void SetupDELAY(UINT drvno, ULONG delay)
{
	ULONG reg = 0;
	DELAY = delay;// DELAY is global in BOARD

	//new for new boards check for S0 length if DELAY reg is there
	ReadLongIOPort(drvno, &reg, 0); //read LCR0 for check length 0xffffffco
	reg = ~reg; //length is inverted
	if (reg>0x34)
	{
		WriteByteS0(drvno, DELAY & 0x0ff, 0x34);	//set WRFFDELAY
		WriteByteS0(drvno, DELAY >> 8, 0x35);
	}
}//SetupDELAY

void SetupHAModule(BOOL irsingle, ULONG fftlines)
{//set to module for C8061 & C7041
	//set the globals in BOARD
	FFTLINES = fftlines;
	HA_MODULE = TRUE;
	HA_IRSingleCH = irsingle;
}//SetupHAModule



void PickOneFifoscan(UINT drvno, pArrayT pdioden, BOOL* pabbr, BOOL* pspace, ULONG fkt)
{	//get one scan off free running fifo timer
	//don't enable Fifo during a read
	//so wait for a complete read and enable afterwards
	ULONG lwritten = 0;
	ULONG pixel = aPIXEL[drvno];

	do {//here used for Keypressed
		WaitTrigger(drvno, FALSE, pspace, pabbr);//test abbruch
	} while ((!FlagXCKI(drvno)) && (*pabbr == FALSE));//sync to active read

	//don't reset Fifo during a read
	do {//here used for Keypressed
		WaitTrigger(drvno, FALSE, pspace, pabbr);//test abbruch
	} while ((FlagXCKI(drvno)) && (*pabbr == FALSE));//wait for not active read

	RSFifo(drvno);

	do {//here used for Keypressed and 1 line valid
		WaitTrigger(drvno, FALSE, pspace, pabbr);//test abbruch
	} while ((!FFValid(drvno)) && (*pabbr == FALSE));//wait for 1 line in Fifo

	//copy data from fifo to buffer pDIODEN
	//ReadFifo(drvno,pdioden,fkt);
	WriteFile(ahCCDDRV[drvno], pdioden, pixel * 2, &lwritten, NULL); //write to PC RAM

	//	DisableFifo(drvno);
}//PickOneFifoscan



//********************  thread priority stuff

BOOL ThreadToPriClass(ULONG threadp, DWORD *priclass, DWORD *prilevel)
{ //converts threadp value (1..31) to process priority class and thread priority level 

	DWORD propriclass[31] = { IDLE_PRIORITY_CLASS, IDLE_PRIORITY_CLASS, IDLE_PRIORITY_CLASS, IDLE_PRIORITY_CLASS, IDLE_PRIORITY_CLASS, IDLE_PRIORITY_CLASS,
		NORMAL_PRIORITY_CLASS, NORMAL_PRIORITY_CLASS, NORMAL_PRIORITY_CLASS, NORMAL_PRIORITY_CLASS, NORMAL_PRIORITY_CLASS,
		HIGH_PRIORITY_CLASS, HIGH_PRIORITY_CLASS, HIGH_PRIORITY_CLASS, HIGH_PRIORITY_CLASS,
		REALTIME_PRIORITY_CLASS, REALTIME_PRIORITY_CLASS, REALTIME_PRIORITY_CLASS, REALTIME_PRIORITY_CLASS, REALTIME_PRIORITY_CLASS,
		REALTIME_PRIORITY_CLASS, REALTIME_PRIORITY_CLASS, REALTIME_PRIORITY_CLASS, REALTIME_PRIORITY_CLASS, REALTIME_PRIORITY_CLASS,
		REALTIME_PRIORITY_CLASS, REALTIME_PRIORITY_CLASS, REALTIME_PRIORITY_CLASS, REALTIME_PRIORITY_CLASS, REALTIME_PRIORITY_CLASS,
		REALTIME_PRIORITY_CLASS };

	DWORD threprilevel[31] = { THREAD_PRIORITY_IDLE, THREAD_PRIORITY_LOWEST, THREAD_PRIORITY_BELOW_NORMAL, THREAD_PRIORITY_NORMAL, THREAD_PRIORITY_ABOVE_NORMAL,
		THREAD_PRIORITY_HIGHEST, THREAD_PRIORITY_LOWEST, THREAD_PRIORITY_BELOW_NORMAL, THREAD_PRIORITY_NORMAL, THREAD_PRIORITY_ABOVE_NORMAL,
		THREAD_PRIORITY_HIGHEST, THREAD_PRIORITY_BELOW_NORMAL, THREAD_PRIORITY_NORMAL, THREAD_PRIORITY_ABOVE_NORMAL, THREAD_PRIORITY_HIGHEST,
		THREAD_PRIORITY_IDLE, THREAD_PRIORITY_IDLE, THREAD_PRIORITY_IDLE, THREAD_PRIORITY_IDLE, THREAD_PRIORITY_IDLE, THREAD_PRIORITY_IDLE,
		THREAD_PRIORITY_LOWEST, THREAD_PRIORITY_BELOW_NORMAL, THREAD_PRIORITY_NORMAL, THREAD_PRIORITY_ABOVE_NORMAL, THREAD_PRIORITY_HIGHEST,
		THREAD_PRIORITY_HIGHEST, THREAD_PRIORITY_HIGHEST, THREAD_PRIORITY_HIGHEST, THREAD_PRIORITY_HIGHEST, THREAD_PRIORITY_TIME_CRITICAL };
	// range check
	if ((0 < threadp) && (threadp < 32)) {
		threadp -= 1; //array index from 0..30
		*priclass = propriclass[threadp];
		*prilevel = threprilevel[threadp];
		return TRUE;
	}
	else {
		return FALSE;
	}
}



BOOL SetPriority(ULONG threadp)
{
	//Thread on
	if (!ThreadToPriClass(threadp, &NEWPRICLASS, &NEWPRILEVEL))
	{
		ErrorMsg(" threadp off range ");
		return FALSE;
	}

	hPROCESS = GetCurrentProcess();
	OLDPRICLASS = GetPriorityClass(hPROCESS);

	if (!SetPriorityClass(hPROCESS, NEWPRICLASS))
	{
		ErrorMsg(" No Class set ");
		return FALSE;
	}

	hTHREAD = GetCurrentThread();
	OLDTHREADLEVEL = GetThreadPriority(hTHREAD);

	if (!SetThreadPriority(hTHREAD, NEWPRILEVEL))
	{
		ErrorMsg(" No Thread set ");
		return FALSE;
	}
	return TRUE;
}//SetPriority



BOOL ResetPriority()
{
	// reset the class Priority and stop thread
	if (!SetPriorityClass(hPROCESS, OLDPRICLASS))
	{
		ErrorMsg(" No Class reset ");
		return FALSE;
	}
	if (!SetThreadPriority(hTHREAD, OLDTHREADLEVEL))
	{
		ErrorMsg(" No Thread reset ");
		return FALSE;
	}

	return TRUE;
}


//***********************  System Timer in Ticks
UINT64 LargeToInt(LARGE_INTEGER li)
{ //converts Large to Int64
	UINT64 res = 0;
	res = li.HighPart;
	res = res << 32;
	res = res + li.LowPart;
	return res;
} //LargeToInt



UINT64 InitHRCounter()
{//returns TPS ticks per sec
	// init high resolution counter 
	BOOL ifcounter;
	UINT64 tps = 0;
	LARGE_INTEGER freq;
	freq.LowPart = 0;
	freq.HighPart = 2;

	//tps:: ticks per second = freq
	ifcounter = QueryPerformanceFrequency(&freq);
	tps = LargeToInt(freq); //ticks per second

	if (tps == 0) ErrorMsg(" System Timer Error ");

	return tps;

} // InitHRCounter

UINT64 ticksTimestamp()
{
	LARGE_INTEGER PERFORMANCECOUNTERVAL = { 0, 0 };

	QueryPerformanceCounter(&PERFORMANCECOUNTERVAL);
	return PERFORMANCECOUNTERVAL.QuadPart;

}//ticksTimestamp


//calc delay in ticks from us
UINT64 ustoTicks(ULONG us)
{// init high resolution counter 
	// and calcs DELAYTICKS from m_belPars.m_belDelayMsec
	BOOL ifcounter;
	UINT64 delaytks = 0;
	UINT64 tps = 0; //ticks per second
	LARGE_INTEGER freq;
	freq.LowPart = 0;
	freq.HighPart = 0;

	//get tps: ticks per second
	ifcounter = QueryPerformanceFrequency(&freq);
	tps = LargeToInt(freq); //ticks per second

	if (tps == 0) return FALSE; // no counter available

	delaytks = us;
	delaytks = delaytks * tps;
	delaytks = delaytks / 1000000;
	return delaytks;
} // ustoTicks



UINT32 Tickstous(UINT64 tks)
{// init high resolution counter 
	// and returns ms
	BOOL ifcounter;
	UINT64 delay = 0;
	UINT64 tps = 0; //ticks per second
	LARGE_INTEGER freq;
	freq.LowPart = 0;
	freq.HighPart = 0;

	//get tps: ticks per second
	ifcounter = QueryPerformanceFrequency(&freq);
	tps = LargeToInt(freq); //ticks per second

	if (tps == 0) return 0; // no counter available

	delay = tks * 1000000;
	delay = delay / tps;
	return (UINT32)delay;
} // Tickstous


// ************************  COOLER Functions  *********************

void ActCooling(UINT drvno, BOOL on)
{//activates cooling with IFC signal
	if (on) { OpenShutter(drvno); }
	else CloseShutter(drvno);
}


BOOL TempGood(UINT drvno, UINT ch)
{//reads EOI Signal = D4 CTRLC
	BYTE CtrlC = 0;
	ReadByteS0(drvno, &CtrlC, 6);

	if (ch == 1)
	{
		if ((CtrlC & 0x10) == 0x10)
		{
			return FALSE;
		}
		else return TRUE;
	}
	if (ch == 2)
	{
		if ((CtrlC & 0x20) == 0x20)
		{
			return FALSE;
		}
		else return TRUE;
	}
	if (ch == 3)
	{
		if ((CtrlC & 0x40) == 0x40)
		{
			return FALSE;
		}
		else return TRUE;
	}
	if (ch == 4)
	{
		if ((CtrlC & 0x80) == 0x80)
		{
			return FALSE;
		}
		else return TRUE;
	}

	return FALSE;
}//TempGood


void SetTemp(UINT drvno, ULONG level)
{// set temperature controler (8 levels)
	CloseShutter(drvno);// IFC=lo
	Sleep(1);

	if (level >= 8) level = 0;
	SendCommand(drvno, 0xA1, (BYTE)level);

	Sleep(1);
	OpenShutter(drvno);		// IFC=hi
	Sleep(1);

	if (level == 0) ActCooling(drvno, FALSE);
}






void SetADGain(UINT32 drvno, UINT8 fkt, UINT8 g1, UINT8 g2, UINT8 g3, UINT8 g4, UINT8 g5, UINT8 g6, UINT8 g7, UINT8 g8)
{	//set gain for ADS5294
	//fkt =0 reset to db=0, fkt=1 set to g1..g8
	DWORD data = 0;
	BYTE a, b, c, d, e, f, g, h;

	a = 0;
	b = 0;
	c = 0;
	d = 0;
	e = 0;
	f = 0;
	g = 0;
	h = 0;

	if (fkt != 0)
	{
		a = g1;  //values 0..f
		b = g2;
		c = g3;
		d = g4;
		e = g5;
		f = g6;
		g = g7;
		h = g8;
	}

	data = a;
	data = data << 4;
	data |= e;
	data = data << 4;
	data |= b;
	data = data << 4;
	data |= f;
	SendFLCAM(drvno, 1, 0x02A, data);	//gain1..4
	data = h;
	data = data << 4;
	data |= d;
	data = data << 4;
	data |= g;
	data = data << 4;
	data |= c;
	SendFLCAM(drvno, 1, 0x02B, data);	//gain7..8
}//SetGain