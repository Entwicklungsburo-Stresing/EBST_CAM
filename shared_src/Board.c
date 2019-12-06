
//   UNIT BOARD.C for all examples equal
//	PCIE version with DMA and INTR
//	V2.1  BB  7/2019
//	- kopierbar für DLL und Examp mit neuem Flag CCDEXAMP , was nur in der Global vo, CCD Example gesetzt werden darf

***REMOVED******REMOVED******REMOVED******REMOVED******REMOVED******REMOVED******REMOVED***//#define LSCPCIEJ_DRIVER_NAME "lscpciej"
//#define LSCPCIEJ_DEFAULT_DRIVER_NAME WD_DEFAULT_DRIVER_NAME_BASE
#define LSCPCIEJ_STRESING_DRIVER_NAME "lscpciej"

//#include "stdafx.h"		// use in C++ only
//#include "global.h"		// use in C++ only
#include "ccdctl.h" //"ccdctrl.h"
#include "board.h"
#include <limits.h>
#include <process.h>
#include "Jungo/windrvr.h"
#include "Jungo/wdc_lib.h"
#include "Jungo/wdc_defs.h"
#include "wchar.h"
#include "lscpciej_lib.c"
//#include "kp_lscpciej.c"

//#include "wd_kp.h"
//siehe beginn functions
//#include "lscpciej_lib.h" 
/*************************************************************
Internal definitions
*************************************************************/
/* WinDriver license registration string */

//Dont trust the debugger its CRAP


/*************************************************************
General definitions
*************************************************
**********/
/* Error messages display */
#define LSCPCIEJ_ERR printf
#define _CNT255 TRUE   //TRUE if FIFO version has 8bit Counter / TRUE is default
// use LSCPCI1 on PCI Boards
#define	DRIVERNAME	"\\\\.\\LSCPCIE"

//****************************   !!!!!!!!!!!!!!!!!!!!!!!!    ****************************
//try different methodes - only one may be TRUE!
#define DMA_CONTIGBUF TRUE		// use if DMABigBuf is set by driver (data must be copied afterwards to DMABigBuf)
#define DMA_SGBUF FALSE			// use if DMABigBuf is set by application (pointer must be passed to SetupPCIE_DMA)

// ExpTime is passed as global var here
// function is not used in WCCD
#ifndef ExpTime
ULONG ExpTime; //in micro sec - needed only in DLL, defined in DLL.h
#endif

// globals within board
// don't change values here - are set within functions SetBoardVars...

//DMA Addresses
enum dma_addresses {
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
	//for extenden S0-Space:
	DmaAddr_PCIEFLAGS = 0x40,
	DmaAddr_NOS = 0x44,
	DmaAddr_ScanIndex = 0x48,
	DmaAddr_DmaBufSizeInScans = 0x04C,		// length in scans
	DmaAddr_DMAsPerIntr = 0x050,
	DmaAddr_NOB = 0x054,
	DmaAddr_BLOCKINDEX = 0x058,
	DmaAddr_CAMCNT = 0x05C
};

//PCIe Addresses
enum pcie_addresses {
	PCIeAddr_devCap = 0x5C,
	PCIeAddr_devStatCtrl = 0x60
};

//S0 Addresses
enum s0_addresses {
	S0Addr_DBR = 0x0,
	S0Addr_CTRLA = 0x4,
	S0Addr_CTRLB = 0x5,
	S0Addr_CTRLC = 0x6,
	S0Addr_XCKLL = 0x8,
	S0Addr_XCKLH = 0x9,
	S0Addr_XCKHL = 0xa,
	S0Addr_XCKMSB = 0xb,
	S0Addr_XCKCNTLL = 0xc,
	S0Addr_XCKCNTLH = 0xd,
	S0Addr_XCKCNTHL = 0xe,
	S0Addr_XCKCNTMSB = 0xf,
	S0Addr_PIXREGlow = 0x10,
	S0Addr_PIXREGhigh = 0x11,
	S0Addr_FREQREG = 0x12,
	S0Addr_FF_FLAGS = 0x13,
	S0Addr_FIFOCNT = 0x14,
	S0Addr_VCLKCTRL = 0x18,
	S0Addr_VCLKFREQ = 0x1b,
	S0Addr_EBST = 0x1C,
	S0Addr_DAT = 0x20,
	S0Addr_EC = 0x24,
	S0Addr_TOR = 0x28,
	S0Addr_ARREG = 0x2C,
	S0Addr_GIOREG = 0x30,
	S0Addr_DELAYEC = 0x34,
	S0Addr_IRQREG = 0x38,
	S0Addr_PCI = 0x3C
};

//Cam Addresses könnten später bei unterschiedlichen cam systemen vaariieren
enum cam_addresses {
	maddr_cam = 0x00,
	maddr_adc = 0x01,
	maddr_ioctrl = 0x02,
	cam_adaddr_gain_led = 0x00,
	cam_adaddr_pixel = 0x01,
	cam_adaddr_trig_in = 0x02,
	cam_adaddr_ch = 0x03,
	cam_adaddr_vclk = 0x04,
	cam_adaddr_LEDoff = 0x05,
	adc_ltc2271_regaddr_reset = 0x00,
	adc_ltc2271_regaddr_outmode = 0x02,
	adc_ltc2271_regaddr_custompattern_msb = 0x03,
	adc_ltc2271_regaddr_custompattern_lsb = 0x04,
	adc_ads5294_regaddr_reset = 0x00,
	adc_ads5294_regaddr_mode = 0x25,
	adc_ads5294_regaddr_custompattern = 0x26,
	adc_ads5294_regaddr_gain_1_to_4 = 0x2A,
	adc_ads5294_regaddr_gain_5_to_8 = 0x2B,
};

enum cam_messages {
	adc_ltc2271_msg_reset = 0x80,
	adc_ltc2271_msg_normal_mode = 0x01,
	adc_ltc2271_msg_custompattern = 0x05,
	adc_ads5294_msg_reset = 0x01,
	adc_ads5294_msg_ramp = 0x40,
	adc_ads5294_msg_custompattern = 0x10,
};

//extern volatile PUSHORT pDMABigBufBase[3];
//jungodriver specific variables
WD_PCI_CARD_INFO deviceInfo[MAXPCIECARDS];
WDC_DEVICE_HANDLE hDev[MAXPCIECARDS];
ULONG DMACounter = 0;//for debugging
//Buffer of WDC_DMAContigBufLock function = one DMA sub block - will be copied to the big pDMABigBuf later
//INT_PTR *pDMASubBuf[3] = { NULL, NULL, NULL };
USHORT* pDMASubBuf[3] = { NULL, NULL, NULL };

//pDMASubBuf = NULL;
//PUSHORT *pDMASubBuf;  //!!GS
WD_DMA *pDMASubBufInfos[3] = { NULL, NULL, NULL }; //there will be saved the neccesary parameters for the dma buffer
BOOL DMAAlreadyStarted = FALSE;
BOOL escape_readffloop = FALSE;
BOOL contffloop = FALSE;
DWORD64 IsrCounter = 0;
//DWORD64 ISRCounter[2] = { 0, 0};
DWORD64 SubBufCounter[3] = { 0, 0, 0 };
SHORT DMAUserBufIndex = 0;
DWORD64 val = 0x0;
//BYTE DontReadUserBufIndex = 0;
INT_PTR pTestBuf_index;
DWORD64 DMA_bufsizeinbytes = 0;
WDC_PCI_SCAN_RESULT scanResult;
UINT8 NUMBER_OF_BOARDS = 0;
UINT32 BOARD_SEL = 1;
// handle array for our drivers
HANDLE ahCCDDRV[5] = { INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE };
ULONG aFLAG816[5] = { 1, 1, 1, 1, 1 };  //AD-Flag
ULONG aCAMCNT[5] = { 1, 1, 1, 1, 1 };	// cameras parallel
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
volatile UCHAR RingCtrlReg = 0; // was not volatile 22.1.2019
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
BOOL DMAISRunning = FALSE;
//ULONG TRMSVals[4][300];
#ifndef _CCDEXAMP
double TRMSval[4];
#endif
// ***********     functions    ********************** 
//weg? wrapped in vi
void ErrMsgBoxOn(void)
{//general switch to suppress error mesage box
	_SHOW_MSG = TRUE;
}
//weg?
void ErrMsgBoxOff(void)
{//general switch to suppress error mesage box
	_SHOW_MSG = FALSE;
}
//weg?
void ErrorMsg(char ErrMsg[100])
{
	if (_SHOW_MSG)
	{
		if (MessageBox(GetActiveWindow(), ErrMsg, "ERROR", MB_OK | MB_ICONEXCLAMATION) == IDOK) {};
	}
};
//weg?
void ValMsg(UINT64 val)
{
	char AString[60];
	if (_SHOW_MSG)
	{
		sprintf_s(AString, 60, "%s%d 0x%I64x", "val= ", val, val);
		if (MessageBox(GetActiveWindow(), AString, "ERROR", MB_OK | MB_ICONEXCLAMATION) == IDOK) {};
	}
};

void AboutDMARegs(UINT32 drv)
{//regs des Contig DMA Buf 
	HWND hWnd = GetActiveWindow();
	int j = 0;
#define s_size 1000
	char fn[s_size];
	PUCHAR pmaxaddr = NULL;

	if (!pDMASubBufInfos[drv]) { j = sprintf_s(fn, s_size, " pBufInfo = 0x%p \n", pDMASubBufInfos[drv]); }
	else {
		j = 0;
		if (DMA_SGBUF) 		j = sprintf_s(fn, s_size, " SG version \n\n");
		if (DMA_CONTIGBUF) 		j = sprintf_s(fn, s_size, " Contig Buf version \n\n");

		if (j == 0)
		{
			ErrorMsg("not SG and not Contig");
			return;
		}
		for (int i = 1; i < 3; i++) {
			pmaxaddr = pDMASubBufInfos[i]->pUserAddr;// calc the upper addr
			pmaxaddr += pDMASubBufInfos[i]->dwBytes;
			j += sprintf_s(fn + j, s_size, " pBufInfo = 0x%p \n", pDMASubBufInfos[i]);
			j += sprintf_s(fn + j, s_size, " hDMA = 0x%I32x \n", pDMASubBufInfos[i]->hDma);
			j += sprintf_s(fn + j, s_size, " pUserAddr = 0x%p \n", pDMASubBufInfos[i]->pUserAddr);
			j += sprintf_s(fn + j, s_size, " pMAXUserAddr = 0x%p \n", pmaxaddr);
			j += sprintf_s(fn + j, s_size, " pKernelAddr = 0x%p \n", pDMASubBufInfos[i]->pKernelAddr);
			j += sprintf_s(fn + j, s_size, " dwBytes = 0x%I32x = %d\n", pDMASubBufInfos[i]->dwBytes, pDMASubBufInfos[i]->dwBytes);
			j += sprintf_s(fn + j, s_size, " dwOptions = 0x%I32x \n", pDMASubBufInfos[i]->dwOptions);
			j += sprintf_s(fn + j, s_size, " dwPages = 0x%I32x \n", pDMASubBufInfos[i]->dwPages);
			j += sprintf_s(fn + j, s_size, " hCard = 0x%I32x \n", pDMASubBufInfos[i]->hCard);
			j += sprintf_s(fn + j, s_size, " physAddr(page0) = 0x%p \n", pDMASubBufInfos[i]->Page[0].pPhysicalAddr);
			j += sprintf_s(fn + j, s_size, " MAXphysAddr(page0) = 0x%p \n", pDMASubBufInfos[i]->Page[0].pPhysicalAddr + pDMASubBufInfos[i]->dwBytes);
			j += sprintf_s(fn + j, s_size, " pagesize(page0) = 0x%I32x \n", pDMASubBufInfos[i]->Page[0].dwBytes);
			j += sprintf_s(fn + j, s_size, " physAddr(page1) = 0x%p \n", pDMASubBufInfos[i]->Page[1].pPhysicalAddr);
			j += sprintf_s(fn + j, s_size, " pagesize(page1) = 0x%I32x \n", pDMASubBufInfos[i]->Page[1].dwBytes);
		}
	}
	if (MessageBox(hWnd, fn, " DMA Buf Regs ", MB_OK | MB_ICONEXCLAMATION) == IDOK) {};

}

AboutTLPs(UINT32 drvno)
{
	ULONG BData = 0;
	ULONG j = 0;
	char fn[600];
	ULONG actpayload = 0;

	j += sprintf(fn + j, "PAY_LOAD values : 0 = 128 bytes, 1 = 256 bytes, 2 = 512 bytes\n");
	ReadLongIOPort(drvno, &BData, PCIeAddr_devCap);//0x4c		
	j += sprintf(fn + j, "PAY_LOAD Supported : 0x%x\n", BData & 0x7);

	//		WriteLongIOPort(DRV,0x2840,0x60);  not working  !! destroys PC? !!
	ReadLongIOPort(drvno, &BData, PCIeAddr_devStatCtrl);
	actpayload = (BData >> 5) & 0x7;
	j += sprintf(fn + j, "PAY_LOAD : 0x%x\n", actpayload);



	ReadLongIOPort(drvno, &BData, PCIeAddr_devStatCtrl);
	j += sprintf(fn + j, "MAX_READ_REQUEST_SIZE : 0x%x\n\n", (BData >> 12) & 0x7);

	//BData &= 0xFFFF8FFF;//set from 256 to 128 max read request size
	//BData |= 0x00001000;//set from 128 to 256 
	//WriteLongIOPort(DRV, BData, PCIeAddr_devStatCtrl);

	//ReadLongIOPort(DRV, &BData, PCIeAddr_devStatCtrl);
	//j += sprintf(fn + j, "MAX_READ_REQUEST_SIZE after : 0x%x\n\n", (BData >> 12) & 0x7);

	BData = aPIXEL[drvno];
	j += sprintf(fn + j, "pixel: %d \n", BData);

	switch (actpayload)
	{
	case 0: BData = 0x20;		break;
	case 1: BData = 0x40;		break;
	case 2: BData = 0x80;		break;
	case 3: BData = 0x100;		break;
	}

	j += sprintf(fn + j, "TLP_SIZE is: %d DWORDs = %d BYTEs\n", BData, BData * 4);

	ReadLongDMA(drvno, &BData, DmaAddr_WDMATLPS);
	j += sprintf(fn + j, "TLPS in DMAReg is: %d \n", BData);

	BData = (_PIXEL - 1) / (BData * 2) + 1 + 1;
	j += sprintf(fn + j, "number of TLPs should be: %d\n", BData);
	ReadLongDMA(drvno, &BData, DmaAddr_WDMATLPC);
	j += sprintf(fn + j, "number of TLPs is: %d \n", BData);

	MessageBox(GetActiveWindow(), fn, "DMA transfer payloads", MB_OK | MB_DEFBUTTON2);

}//AboutTLPs

void AboutS0(UINT32 drvno)
{
	int i, j = 0;
	int numberOfBars = 0;
	char fn[1000];
	ULONG S0Data = 0;
	ULONG length = 0;
	HWND hWnd;
	char LUTS0Reg[32][30] = {
		"DBR \t",
		"CTRLA \t",
		"XCKLL \t",
		"XCKCNTLL",
		"PIXREG \t",
		"FIFOCNT \t",
		"VCLKCTRL",
		"'EBST' \t",
		"DAT \t",
		"XDLY \t",
		"TOR \t",
		"ARREG \t",
		"GIOREG \t",
		"nc\t ",
		"IRQREG \t",
		"PCI board version",
		"R0 PCIEFLAGS",
		"R1 NOS\t",
		"R2 SCANINDEX",
		"R3 DMABUFSIZE",
		"R4 DMASPERINTR",
		"R5 BLOCKS",
		"R6 BLOCKINDEX",
		"R7 CAMCNT",
		"R8 \t",
		"R9 TRIGCNT",
		"R10 \t",
		"R11 \t",
		"R12 \t",
		"R13 TIMEMEAS",
		"R14 \t ",
		"R15 \t"
	}; //Look-Up-Table for the S0 Registers

	hWnd = GetActiveWindow();

	j = sprintf(fn, "S0- registers   \n");

	//Hier werden alle 6 Adressen der BARs in Hex abgefragt

	for (i = 0; i <= 31; i++)
	{
		ReadLongS0(drvno, &S0Data, i * 4);
		j += sprintf(fn + j, "%s \t: 0x%I32x\n", LUTS0Reg[i], S0Data);
	}

	MessageBox(hWnd, fn, "S0 regs", MB_OK);

	AboutTLPs(drvno);

}//AboutS0

BOOL CCDDrvInit(void)
{// returns true if driver was found
	//WDC_Err(drvno);
	ULONG MAXDMABUFLENGTH = 0x07fff; //val look in registry driver parameters
	//depends on os, how big a buffer can be
	BOOL fResult = FALSE;
	char AString[80] = "";
	HANDLE hccddrv = INVALID_HANDLE_VALUE;


	if ((ULONG)_PIXEL > (ULONG)(MAXDMABUFLENGTH / 4))
	{
		ErrorMsg("DMA Buffer length > 0x7fff/4 -> need special driver!");
		return FALSE;
	}


	//PWDC_DEVICE pDev = (PWDC_DEVICE)hDev;
	volatile DWORD dwStatus = 0;

	PLSCPCIEJ_DEV_CTX pDevCtx = NULL;

#if KER_MODE
	LSCPCIEJ_DEV_ADDR_DESC devAddrDesc;
#endif

#if defined(WD_DRIVER_NAME_CHANGE)
	/* Set the driver name */

	if (!WD_DriverName(LSCPCIEJ_STRESING_DRIVER_NAME))
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
#if defined(_DEBUG)		
	dwStatus = WDC_SetDebugOptions(WDC_DBG_DEFAULT, NULL);
#else				
	dwStatus = WDC_SetDebugOptions(WDC_DBG_NONE, NULL);
#endif	
	if (WD_STATUS_SUCCESS != dwStatus)
	{
		ErrLog("Failed to initialize debug options for WDC library.\n"
			"Error 0x%lx - %s\n", dwStatus, Stat2Str(dwStatus));
		WDC_Err("%s", LSCPCIEJ_GetLastErr());
		ErrorMsg("Failed to initialize debug options for WDC library.\n"
			"Error 0x%lx - %s\n", dwStatus, Stat2Str(dwStatus));
		WDC_DriverClose();
		ErrorMsg("driver closed.\n");
		return FALSE;
	}
	//ErrorMsg("CCDDrvInit start of %x \n", drvno);
	/* Open a handle to the driver and initialize the WDC library */

***REMOVED***	if (WD_STATUS_SUCCESS != dwStatus)
	{
		ErrLog("Failed to initialize the WDC library. Error 0x%lx - %s\n",
			dwStatus, Stat2Str(dwStatus));
		//doesnt work at this moment before debugsetup
		WDC_Err("%s", LSCPCIEJ_GetLastErr());
		ErrorMsg("Failed to initialize the WDC library. Maybe the driver was not unloaded correctly.\n");
		WDC_DriverClose();
		ErrorMsg("driver closed.\n");
		return FALSE;
	}


	BZERO(scanResult);
	dwStatus = WDC_PciScanDevices(LSCPCIEJ_DEFAULT_VENDOR_ID, LSCPCIEJ_DEFAULT_DEVICE_ID, &scanResult); //VendorID, DeviceID
	if (WD_STATUS_SUCCESS != dwStatus)
	{

		ErrLog("DeviceFind: Failed scanning the PCI bus.\n"
			"Error: 0x%lx - %s\n", dwStatus, Stat2Str(dwStatus));
		WDC_Err("%s", LSCPCIEJ_GetLastErr());
		ErrorMsg("Device not found");
		WDC_DriverClose();
		ErrorMsg("driver closed.\n");
		return FALSE;
	}
	NUMBER_OF_BOARDS = scanResult.dwNumDevices;

	TPS = InitHRCounter();//for ticks function

	return TRUE;	  // no Error, driver found

}; //CCDDrvInit


void CCDDrvExit(UINT32 drvno)
{
	WDC_Err("drvexit\n");
	if (WDC_IntIsEnabled(hDev[drvno]))
	{
		WDC_Err("cleanup dma\n");
		CleanupPCIE_DMA(drvno);
	}
	WDC_DriverClose();
	WDC_PciDeviceClose(hDev[drvno]);
	WDC_Err("Driver closed and PciDeviceClosed \n");
	//if (ahCCDDRV[drvno]!=INVALID_HANDLE_VALUE)
	//CloseHandle(ahCCDDRV[drvno]);	   // close driver
	ahCCDDRV[drvno] = INVALID_HANDLE_VALUE;
	aINIT[drvno] = FALSE;
};

BOOL InitBoard(UINT32 drvno)
{

	if ((drvno < 1) || (drvno > 2)) return FALSE;
	//PWDC_DEVICE pDev = (PWDC_DEVICE)hDev;
	volatile DWORD dwStatus = 0;



	WDC_Err("Info: scan result: a board found:%lx , dev=%lx, ven=%lx \n",
		scanResult.dwNumDevices, scanResult.deviceId[drvno - 1].dwDeviceId, scanResult.deviceId[drvno - 1].dwVendorId);


	//gives the information received from PciScanDevices to PciGetDeviceInfo
	BZERO(deviceInfo[drvno]);
	memcpy(&deviceInfo[drvno].pciSlot, &scanResult.deviceSlot[drvno - 1], sizeof(deviceInfo[drvno].pciSlot));

	/* Retrieve the device's resources information */

	dwStatus = WDC_PciGetDeviceInfo(&deviceInfo[drvno]);
	if (WD_STATUS_SUCCESS != dwStatus)
	{
		ErrLog("DeviceOpen: Failed retrieving the device's resources information.\n"
			"Error 0x%lx - %s\n", dwStatus, Stat2Str(dwStatus));
		WDC_Err("%s", LSCPCIEJ_GetLastErr());
		ErrorMsg("WDC_PciGetDeviceInfo failed");
		WDC_DriverClose();
		ErrorMsg("driver closed.\n");
		return FALSE;
	}

	WDC_Err("Info: device info: bus:%lx , slot=%lx, func=%lx \n", deviceInfo[drvno].pciSlot.dwBus, deviceInfo[drvno].pciSlot.dwSlot, deviceInfo[drvno].pciSlot.dwFunction);
	WDC_Err("Info: device info: items:%lx , item[0]=%lx \n", deviceInfo[drvno].Card.dwItems, deviceInfo[drvno].Card.Item[0]);

	hDev[drvno] = LSCPCIEJ_DeviceOpen(&deviceInfo[drvno]);
	if (!hDev[drvno])
	{
		LSCPCIEJ_ERR("DeviceOpen: Failed opening a handle to the device: %s\n",
			LSCPCIEJ_GetLastErr());
		ErrorMsg("DeviceOpen failed\n");
		WDC_Err("DeviceOpen failed %s\n", LSCPCIEJ_GetLastErr());
		WDC_DriverClose();
		ErrorMsg("driver closed.\n");
		return NULL;
	}


	PWDC_DEVICE pDev = ((PWDC_DEVICE)hDev[drvno]);
	WDC_Err("DRVInit hDev id % x, hDev pci slot %x, hDev pci bus %x, hDev pci function %x, hDevNumAddrSp %x \n"
		, pDev->id, pDev->slot.dwSlot, pDev->slot.dwBus, pDev->slot.dwFunction, pDev->dwNumAddrSpaces);

	/*hccddrv = WDC_GetWDHandle();
	if (hccddrv == (HANDLE)INVALID_HANDLE_VALUE)
	{
	WDC_Err("createHandle failed");
	return FALSE;
	}*/	// false if LSCPCIn not there 
	//save handle in global array
	ahCCDDRV[drvno] = hDev[drvno];//hccddrv;


	/*
	//for testing
	KP_LSCPCIEJ_VERSION Data;
	KP_LSCPCIEJ_VERSION *pData;
	pData = &Data;
	DWORD dwResult;
	PDWORD	pdwResult = &dwResult;

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



	//Sleep(1000);

	return TRUE;

};  // InitBoard

//weg?
char CntBoards(void)
{//get how many PCI boards are there
	int i = 0;
	int foundBoards = 0;
	ErrorMsg("CntBoards.\n");
	for (i = 1; i < 5; i++) // check for max 4
		if (InitBoard(i)) //call once per cam
		{
			foundBoards += 1;
		}
	return foundBoards;
}//CntBoards



//**************  new for PCIE   *******************************

BOOL SetDMAReg(ULONG Data, ULONG Bitmask, ULONG Address, UINT32 drvno) {//the bitmask have "1" on the data dates like Bitmask: 1110 Data:1010 
	ULONG OldRegisterValues;
	ULONG NewRegisterValues;
	//read the old Register Values in the DMA Address Reg
	if (!ReadLongDMA(drvno, &OldRegisterValues, Address)) {
		ErrLog("ReadLong DMA Failed in SetDMAReg \n");
		WDC_Err("%s", LSCPCIEJ_GetLastErr());
		return FALSE;
	}
	//save the bits, which shall not changed
	OldRegisterValues = OldRegisterValues & ~Bitmask; //delete the bits, which are "1" in the bitmask
	Data = Data & Bitmask; //to make sure that there are no bits, where the bitmask isnt set

	NewRegisterValues = Data | OldRegisterValues;
	//write the data to the DMA controller
	if (!WriteLongDMA(drvno, NewRegisterValues, Address)) {
		ErrLog("WriteLong DMA Failed in SetDMAReg \n");
		WDC_Err("%s", LSCPCIEJ_GetLastErr());
		return FALSE;
	}
	return TRUE;
}

BOOL SetS0Reg(ULONG Data, ULONG Bitmask, CHAR Address, UINT32 drvno) {
	ULONG OldRegisterValues, Setbit_mask, OldRegVals_and_SetBits, Clearbit_mask, NewRegisterValues;

	//read the old Register Values in the S0 Address Reg
	if (!ReadLongS0(drvno, &OldRegisterValues, Address)) {
		ErrLog("ReadLong S0 Failed in SetDMAReg \n");
		WDC_Err("%s", LSCPCIEJ_GetLastErr());
		return FALSE;
	}
	//step 0: delete not needed "1"s
	Data &= Bitmask;
	//step 1: save Data as setbitmask for making this part humanreadable
	Setbit_mask = Data;
	//step 2: setting high bits in the Data
	OldRegVals_and_SetBits = OldRegisterValues | Setbit_mask;
	//step 3: prepare to clear bits
	Clearbit_mask = Data | ~Bitmask;
	//step 4: clear the low bits in the Data
	NewRegisterValues = OldRegVals_and_SetBits & Clearbit_mask;

	//write the data to the S0 controller
	if (!WriteLongS0(drvno, NewRegisterValues, Address)) {
		ErrLog("WriteLong S0 Failed in SetDMAReg \n");
		WDC_Err("%s", LSCPCIEJ_GetLastErr());
		return FALSE;
	}
	return TRUE;
}


BOOL SetS0Bit(ULONG bitnumber, CHAR Address, UINT32 drvno) {
	//bitnumber: 0...31
	ULONG bitmask = 0x1 << bitnumber;
	if (!SetS0Reg(0xFFFFFFFF, bitmask, Address, drvno)) {
		ErrLog("WriteLong S0 Failed in SetDMAReg \n");
		WDC_Err("%s", LSCPCIEJ_GetLastErr());
		return FALSE;
	}
	return TRUE;

}

BOOL ResetS0Bit(ULONG bitnumber, CHAR Address, UINT32 drvno) {

	ULONG bitmask = 0x1 << bitnumber;

	if (!SetS0Reg(0x0, bitmask, Address, drvno)) {
		ErrLog("WriteLong S0 Failed in SetDMAReg \n");
		WDC_Err("%s", LSCPCIEJ_GetLastErr());
		return FALSE;
	}
	return TRUE;

}


BOOL SetDMAAddrTlpRegs(UINT64 PhysAddrDMABuf64, ULONG tlpSize, ULONG no_tlps, UINT32 drvno) {

	UINT64 PhysAddrDMABuf;
	ULONG RegisterValues;
	ULONG wr_addr_64bit_en = 0x1 << 19; //bit 19 is for 64bit enable
	ULONG BitMask;
	//proof the physical address for the max. size of the DMA Controller
	/*this error never happens. I think there is no need for this check
	if (PhysAddrDMABuf64 > 0x3fffffffff){
	ErrLog("Physical DMA buffer address is more than 5bytes: 0x%016x \n",
	PhysAddrDMABuf64);
	WDC_Err("%s", LSCPCIEJ_GetLastErr());
	return FALSE;
	}*/



	//WDMATLPA (Reg name): write the lower part (bit 02:31) of the DMA adress to the DMA controller
	//WDC_Err("64 bit Address 0x%llx\n", PhysAddrDMABuf64);
	RegisterValues = (ULONG)PhysAddrDMABuf64;
	BitMask = 0xFFFFFFFC;
	//WDC_Err("lower part of 64 bit Address 0x%x\n", RegisterValues);
	if (!SetDMAReg(RegisterValues, BitMask, DmaAddr_WDMATLPA, drvno)) {
		WDC_Err("Set the lower part of the DMA Address failed");
		return FALSE;
	}

	//	ErrorMsg("vor Addr 1te SetDMAReg");  //hier gehts noch

	//WDMATLPS: write the upper part (bit 32:39) of the address	
	PhysAddrDMABuf = ((UINT64)PhysAddrDMABuf64 >> 32);

	//WDC_Err("upper part of 64 bit Address 0x%x\n", PhysAddrDMABuf);
	//PhysAddrDMABuf >> 32;	//shift to the upper part 
	PhysAddrDMABuf = PhysAddrDMABuf << 24;		//shift to prepare for the Register
	BitMask = 0xFF081FFF;
	//CHECK proof 0x20 from old driver
	//ULONG TLPSize = 0x20;//(*ppDma)->Page->dwBytes / sizeof(WORD);//divide by 4 because of the conversion from bytes to DWORD
	RegisterValues = (ULONG)PhysAddrDMABuf;
	RegisterValues |= tlpSize;

	//64bit address enable
	if (DMA_64BIT_EN) {
		RegisterValues |= wr_addr_64bit_en;
	}

	if (!SetDMAReg(RegisterValues, BitMask, DmaAddr_WDMATLPS, drvno)) {
		WDC_Err("Set the upper part of the DMA Address and the TLPsize failed");
		return FALSE;
	}


	//WDMATLPC: Set the number of DMA transfer count
	BitMask = 0xFFFF;
	if (!SetDMAReg(no_tlps, BitMask, DmaAddr_WDMATLPC, drvno)) {
		WDC_Err("Set the number of DMA transfer count failed");
		return FALSE;
	}
	return TRUE;
}//SetDMAAddrTlpRegs

BOOL SetDMAAddrTlp(UINT32 drvno) {
	WD_DMA **ppDma = &pDMASubBufInfos[drvno];
	UINT64 PhysAddrDMABuf64;
	ULONG BitMask;
	ULONG BData = 0;
	int tlpmode = 0;

	ReadLongIOPort(drvno, &BData, PCIeAddr_devCap);
	tlpmode = BData & 0x7;//0xE0 ;
	//tlpmode = tlpmode >> 5;
	if (_FORCETOPLS128) tlpmode = 0;

	BData &= 0xFFFFFF1F;//delete the old values
	BData |= (0x2 << 12);//set maxreadrequestsize
	switch (tlpmode)
	{
	case 0:
		BData |= 0x00;//set to 128 bytes = 32 DWORDS 
		//BData |= 0x00000020;//set from 128 to 256 
		WriteLongIOPort(drvno, BData, PCIeAddr_devStatCtrl);
		//NO_TLPS setup now in setboardvars
		TLPSIZE = 0x20;
		break;
	case 1:
		BData |= 0x20;//set to 256 bytes = 64 DWORDS 
		//BData |= 0x00000020;//set to 256 
		WriteLongIOPort(drvno, BData, PCIeAddr_devStatCtrl);
		//NO_TLPS = 0x9;//x9 was before. 0x10 is calculated in aboutlp and 0x7 is working;
		TLPSIZE = 0x40;
		break;
	case 2:
		BData |= 0x40;//set to 512 bytes = 128 DWORDS 
		//BData |= 0x00000020;//set to 512 
		WriteLongIOPort(drvno, BData, PCIeAddr_devStatCtrl);
		//NO_TLPS = 0x5;
		TLPSIZE = 0x80;
		break;
	}

	//write Address
	PhysAddrDMABuf64 = (*ppDma)->Page[0].pPhysicalAddr;
	if (!SetDMAAddrTlpRegs(PhysAddrDMABuf64, TLPSIZE, NO_TLPS, drvno))
		return FALSE;
	return TRUE;
}
BOOL SetDMABufRegs(UINT32 drvno, ULONG nos, ULONG nob, ULONG camcnt) {
	//set DMA_BUFSIZEINSCANS
	//set DMA_DMASPERINTR
	//set NOS
	//here: make one big buffer for nos scans
	BOOL error = FALSE;

	if (!SetS0Reg(DMA_BUFSIZEINSCANS, 0xffffffff, DmaAddr_DmaBufSizeInScans, drvno))//DMABufSizeInScans - use 1 block
		error = TRUE;

	//scans per intr must be 2x per DMA_BUFSIZEINSCANS to copy hi/lo part
	//aCAMCNT: double the INTR if 2 cams
	if (!SetS0Reg(DMA_DMASPERINTR, 0xffffffff, DmaAddr_DMAsPerIntr, drvno))
		error = TRUE;
	WDC_Err("spi/camcnt: %x \n", DMA_DMASPERINTR / camcnt);

	if (!SetS0Reg(nos, 0xffffffff, DmaAddr_NOS, drvno))
		error = TRUE;

	if (!SetS0Reg(nob, 0xffffffff, DmaAddr_NOB, drvno))
		error = TRUE;

	if (!SetS0Reg(camcnt, 0xffffffff, DmaAddr_CAMCNT, drvno))
		error = TRUE;

	if (error)
	{
		ErrorMsg("SetDMABufRegs failed");
		return FALSE;
	}
	//ReadLongS0(DRV, &reg, DmaAddr_DMAsPerIntr);
	//WDC_Err("readreg DMASPERINTR: %x \n", reg);
	return TRUE;
}
void SetDMAReset(UINT32 drvno) {
	ULONG BitMask;
	ULONG RegisterValues;
	BitMask = 0x1;
	RegisterValues = 0x1;
	if (!SetDMAReg(RegisterValues, BitMask, DmaAddr_DCSR, drvno)) {
		ErrorMsg("switch on the Initiator Reset for the DMA failed");
		return;
	}
	// DCSR: reset the Iniator Reset 
	RegisterValues = 0x0;
	if (!SetDMAReg(RegisterValues, BitMask, DmaAddr_DCSR, drvno)) {
		ErrorMsg("switch off the Initiator Reset for the DMA failed");
		return;
	}
}
void SetDMAStart(UINT32 drvno) {
	ULONG BitMask;
	ULONG RegisterValues;
	BitMask = 0x1;
	RegisterValues = 0x1;
	if (!SetDMAReg(RegisterValues, BitMask, DmaAddr_DDMACR, drvno)) {
		ErrorMsg("Set the Start Command for th DMA failed");
		return;
	}
}
/*
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
	return TRUE;
}
*/

ULONG GetScanindex(UINT32 drvno)
{
	ULONG ldata = 0;
	if (!ReadLongS0(drvno, &ldata, DmaAddr_ScanIndex))
	{
		ErrorMsg("Error GetScanindex");
		return 0;
	}
	return ldata;
}

//for the restpart of the buffer
void GetLastBufPart(UINT32 drvno) {
	//return;
	//get the rest if buffer is not multiple of 500 (BUFSIZEINSCANS/2)
	//also if nos is < BUFSIZEINSCANS/2 - here: no intr occurs
	ULONG nos = 0;
	ULONG nob = 0;
	ULONG spi = 0;
	ULONG halfbufsize = 0;
	ULONG camcnt = 0;
	//size_t rest_in_bytes = 0;

	ReadLongS0(drvno, &nob, DmaAddr_NOB);
	ReadLongS0(drvno, &nos, DmaAddr_NOS);
	ReadLongS0(drvno, &spi, DmaAddr_DMAsPerIntr); //get scans per intr
	ReadLongS0(drvno, &camcnt, DmaAddr_CAMCNT);
	//!! aCAMCNT  löschen !

	//halfbufize is 500 with default values
	halfbufsize = DMA_BUFSIZEINSCANS / DMA_HW_BUFPARTS;
	ULONG scans_all_cams = nos * nob *camcnt;
	ULONG rest_overall = scans_all_cams % halfbufsize;
	size_t rest_in_bytes;
	rest_in_bytes = rest_overall * _PIXEL * sizeof(USHORT);

	WDC_Err("*GetLastBufPart():\n");
	WDC_Err("nos: 0x%x, nob: 0x%x, spi: 0x%x, camcnt: 0x%x\n", nos, nob, spi, camcnt);
	WDC_Err("scans_all_cams: 0x%x \n", scans_all_cams);
	WDC_Err("rest_overall: 0x%x, rest_in_bytes: 0x%x\n", rest_overall, rest_in_bytes);
	WDC_Err("DMA_bufsizeinbytes: 0x%x \n", DMA_bufsizeinbytes);

	if (rest_overall) { // if (rest_per_block)
		WDC_Err("has rest_overall:\n");
		INT_PTR pDMASubBuf_index = pDMASubBuf[drvno];
		pDMASubBuf_index += SubBufCounter[drvno] * DMA_bufsizeinbytes / DMA_HW_BUFPARTS;

		memcpy(pDMABigBufIndex[drvno], pDMASubBuf_index, rest_in_bytes);
		//memset(pDMABigBufIndex[drvno], 0x01, restlength); //  0xAAAA=43690 , 0101= 257

		//if (nos < spi)  // do not use INTR, but GetLastBufPart instead if nos is small enough
		//	{pDMABigBufIndex[drvno] += rest_summary; } // may only be added here if no isr
	}
	SubBufCounter[drvno] = 0; //reset for next block

}//GetLastBufPart

void isr(UINT drvno, PVOID pData)
{	//this call comes every DMASPERINTR=500 here a DMASubBuf could be copied to the DMABigBuf
	// the INTR occurs every DMASPERINTR and copies this block of scans in sub blocks
	WDC_Err("*isr(): 0x%x\n", IsrCounter);
	WDC_Err("DMA_bufsizeinbytes: 0x%x \n", DMA_bufsizeinbytes);
	SetS0Bit(3, DmaAddr_PCIEFLAGS, drvno);//set INTRSR flag for TRIGO

	ULONG nos = 0;
	//ULONG nob = 0;
	ULONG blocks = 0;
	ULONG val = 0;
	ULONG spi = 0;//scans_per_intr
	size_t subbuflengthinbytes = DMA_bufsizeinbytes / DMA_HW_BUFPARTS; //1088000 bytes
	//usually DMA_bufsizeinbytes = 1000scans 
	//subbuflengthinbytes = 1000 * pixel *2 -> /2 = 500 scans = 1088000 bytes
	// that means one 500 scan copy block has 1088000 bytes

	ULONG dma_subbufinscans = DMA_BUFSIZEINSCANS / DMA_HW_BUFPARTS; //500

	ReadLongS0(drvno, &nos, DmaAddr_NOS);
	//ReadLongS0(drvno, &nob, DmaAddr_NOB);
	ReadLongS0(drvno, &blocks, DmaAddr_NOB);

	USHORT* pdmasubbuf_base = pDMASubBuf[drvno];
	ULONG introverall;


	if (BOARD_SEL > 2)//! >2 oder?
		introverall = blocks * nos * aCAMCNT[drvno] * NUMBER_OF_BOARDS / dma_subbufinscans - 2;//- 2 because intr counter starts with 0
	else
		introverall = blocks * nos * aCAMCNT[drvno] / dma_subbufinscans - 1;//- 1 because intr counter starts with 0
	//!GS sometimes (all 10 minutes) one INTR more occurs -> just do not serve it and return
	// Fehler wenn zu viele ISRs -> memcpy out of range

	WDC_Err("introverall: 0x%x \n", introverall);
	WDC_Err("ISR Counter : 0x%x \n", IsrCounter);
	if (IsrCounter > introverall)
	{
		WDC_Err("introverall: 0x%x \n", introverall);
		WDC_Err("ISR Counter overflow: 0x%x \n", IsrCounter);
		ResetS0Bit(3, DmaAddr_PCIEFLAGS, drvno);//reset INTRSR flag for TRIGO
		return;
	}


	ReadLongS0(drvno, &spi, DmaAddr_DMAsPerIntr); //get scans per intr = 500
	//WDC_Err("DmaAddr_DMAsPerIntr: 0x%x \n", val);
	WDC_Err("in isr -- nos: 0x%x, nob: 0x%x, spi: 0x%x, blocks: 0x%x\n", nos, Nob, spi, blocks);
	//if (rest < spi) return without interrupt; // do not use INTR, but GetLastBufPart instead if rest is small enough
	if ((nos*blocks * aCAMCNT[drvno]) < spi)
	{
		WDC_Err("must get rest: 0x%x \n", nos*blocks * aCAMCNT[drvno] % spi);
		GetLastBufPart(drvno);
		ResetS0Bit(3, DmaAddr_PCIEFLAGS, drvno);//reset INTRSR flag for TRIGO
		return;
	}

	//ValMsg(val);

	ReadLongS0(drvno, &val, DmaAddr_PCIEFLAGS); //set INTRSR flag for TRIGO signal to monitor the signal
	val |= 0x08;
	WriteLongS0(drvno, val, DmaAddr_PCIEFLAGS);

	pdmasubbuf_base += SubBufCounter[drvno] * subbuflengthinbytes / sizeof(USHORT);  // cnt in USHORT

	/*
	UINT64 DMAbigbufsize = nos * blocks *_PIXEL;// +subbuflengthinbytes / sizeof(USHORT);  //in USHORT
	if ((pDMABigBufIndex[drvno] > pDMABigBufBase[drvno] + DMAbigbufsize) || (pDMABigBufIndex[drvno] < pDMABigBufBase[drvno]))
	{
		WDC_Err("subbuflengthinbytes: 0x%x \n", subbuflengthinbytes);
		WDC_Err("pDMABigBufBase: 0x%x \n", pDMABigBufBase[drvno]);
		WDC_Err("DMAbigbufsize: 0x%x \n", DMAbigbufsize);
		WDC_Err("pDMABigBufIndex: 0x%x \n", pDMABigBufIndex[drvno]);
		pDMABigBufIndex[drvno] = pDMABigBufBase[drvno]; //wrap if error - but now all is mixed up!
	}
	*/
	//WDC_Err("pDMABigBufIndex: 0x%x \n", pDMABigBufIndex[drvno]);

	START = ticksTimestamp(); //set global START for next loop
	WaitforTelapsed(900);//bugfix: if not, the last scan is not in the smallbuf, 700 is too small
	//here  the copyprocess happens
	//! 
	memcpy_s(pDMABigBufIndex[drvno], subbuflengthinbytes, pdmasubbuf_base, subbuflengthinbytes);//DMA_bufsizeinbytes/10
	// A.M. 08.Jan.2018 subbuflengthinbytes/ aCAMCNT[drvno]
	//memset(pDMABigBufIndex[drvno], IsrCounter, subbuflengthinbytes  ); //  0xAAAA=43690 , 0101= 257
	WDC_Err("pDMABigBufIndex: 0x%x \n", pDMABigBufIndex[drvno]);

	SubBufCounter[drvno]++;
	if (SubBufCounter[drvno] >= DMA_HW_BUFPARTS)		//number of ISR per dmaBuf - 1
		SubBufCounter[drvno] = 0;						//SubBufCounter is 0 or 1 for buffer devided in 2 parts



	pDMABigBufIndex[drvno] += subbuflengthinbytes / sizeof(USHORT); //!!GS  calc for USHORT

	//error prevention...not needed if counter counts correct
	//ReadLongS0(drvno, &blocks, DmaAddr_NOB);

	//!!GS  //add space for last val and reset base if error
	//UINT64 pDMAbigbufsize = nos * blocks *_PIXEL;// +subbuflengthinbytes / sizeof(USHORT);  //in USHORT
/*	if ((pDMABigBufIndex[drvno] > pDMABigBufBase[drvno] + pDMAbigbufsize) || (pDMABigBufIndex[drvno] < pDMABigBufBase[drvno]))
	{
		ErrorMsg("ISR: buffer overrun !");

		WDC_Err("pDMABigBufBase: 0x%x \n", pDMABigBufBase[drvno]);
		WDC_Err("pDMAbigbufsize: 0x%x \n", pDMAbigbufsize);
		WDC_Err("pDMABigBufIndex: 0x%x \n", pDMABigBufIndex[drvno]);
		pDMABigBufIndex[drvno] = pDMABigBufBase[drvno]; //wrap if error - but now all is mixed up!
	}
*/
	ResetS0Bit(3, DmaAddr_PCIEFLAGS, drvno);//reset INTRSR flag for TRIGO
	IsrCounter++;

	//WDC_Err("ISR: pix42 of ReturnFrame: 0x%d \n", *(USHORT*)(pDMABigBufBase[drvno] + 420));

}//DLLCALLCONV interrupt_handler

VOID DLLCALLCONV interrupt_handler1(PVOID pData) { isr(1, pData); }

VOID DLLCALLCONV interrupt_handler2(PVOID pData) { isr(2, pData); }

BOOL SetupPCIE_DMA(UINT32 drvno, ULONG nos, ULONG nob)
{	//alloc DMA buffer - should only be called once
	//gets address of DMASubBuf from driver and copy it later to our pDMABigBuf
	DWORD dwStatus;
	PUSHORT tempBuf;
	WDC_Err("entered SetupPCIE_DMA\n");


	//tempBuf = (PUSHORT)pDMABigBufBase[drvno] + 500 * sizeof(USHORT);
	//WDC_Err("setupdma: bigbuf Pixel500: %i\n", *tempBuf);

	DMA_bufsizeinbytes = DMA_BUFSIZEINSCANS * _PIXEL * sizeof(USHORT);

	DWORD dwOptions = DMA_FROM_DEVICE | DMA_KERNEL_BUFFER_ALLOC;// | DMA_ALLOW_64BIT_ADDRESS;// DMA_ALLOW_CACHE ;
	if (DMA_64BIT_EN)
		dwOptions |= DMA_ALLOW_64BIT_ADDRESS;

#if (DMA_SGBUF)
	if (!pDMABigBuf)
	{
		ErrLog("Failed: buf pointer not valid.\n");
		WDC_Err("%s", "Failed: buf pointer not valid.\n");
		ErrorMsg("DMA buffer addr is not valid");
		return FALSE;
	}
	// pDMABigBuf is the big space which is passed to this function = input - must be global
	dwStatus = WDC_DMASGBufLock(hDev[drvno], pDMABigBuf, dwOptions, DMA_bufsizeinbytes, &pDMASubBufInfos); //size in Bytes
#endif

#if (DMA_CONTIGBUF)		//usually we use contig buf: here we get the buffer address from labview.
	// pDMASubBuf is the space which is allocated by this function = output - must be global
	dwStatus = WDC_DMAContigBufLock(hDev[drvno], &pDMASubBuf[drvno], dwOptions, DMA_bufsizeinbytes, &pDMASubBufInfos[drvno]); //size in Bytes
	if (WD_STATUS_SUCCESS != dwStatus)
	{
		ErrLog("Failed locking a contiguous DMA buffer. Error 0x%lx - %s\n",
			dwStatus, Stat2Str(dwStatus));
		WDC_Err("%s", LSCPCIEJ_GetLastErr());
		ErrorMsg("DMA buffer not sufficient");
		return FALSE;
	}
	// data must be copied afterwards to user Buffer 
#endif

	pDMABigBufIndex[drvno] = pDMABigBufBase[drvno];	//reset destination buffer to start value
	IsrCounter = 0;
	WD_DMA **ppDma = &pDMASubBufInfos[drvno];

	//WDC_Err("RAM Adresses for DMA Buf: %x ,DMA Buf Size: %x\n", (*ppDma)->Page[0].pPhysicalAddr, (*ppDma)->dwBytes);
	//WDC_Err("RAM Adresses for BigBufBase: %x ,DMA BufSizeinbytes: %x\n", pDMABigBufBase[drvno], DMA_bufsizeinbytes);

	//	ErrorMsg("nach WDC_DMAContigBufLock");
	//	AboutDMARegs();

	/*for KP
	if (HWDREQ_EN)
	if(!SendDMAInfoToKP())
	WDC_Err("sendDMAInfoToKP failed");
	/*for KP
	if (HWDREQ_EN)
	if(!SendDMAInfoToKP())
	WDC_Err("sendDMAInfoToKP failed");
	*/
	/*for KP
	if (HWDREQ_EN)
	if(!SendDMAInfoToKP())
	WDC_Err("sendDMAInfoToKP failed");
	*/
	//

	//set Init Regs
	if (!SetDMAAddrTlp(drvno)) {
		ErrLog("DMARegisterInit for TLP and Addr failed \n");
		WDC_Err("%s", LSCPCIEJ_GetLastErr());
		ErrorMsg("DMARegisterInit for TLP and Addr failed");
		return FALSE;
	}

	//count only whats needed - was set in nDLLSetupDMA
	if (!SetDMABufRegs(drvno, nos, nob, aCAMCNT[drvno])) {  //set hardware regs
		ErrLog("DMARegisterInit for Buffer failed \n");
		WDC_Err("%s", LSCPCIEJ_GetLastErr());
		ErrorMsg("DMARegisterInit for Buffer failed");
		return FALSE;
	}

	// DREQ: every XCK h->l starts DMA by hardware
	//set hardware start des dma  via DREQ withe data = 0x4000000
	ULONG mask = 0x40000000;
	ULONG data = 0;// 0x40000000;
	if (HWDREQ_EN)
		data = 0x40000000;
	SetS0Reg(data, mask, S0Addr_IRQREG, drvno);

	// Enable DMA interrupts (if not polling)
	// INTR should copy DMA buffer to user buf: 
	if (INTR_EN)
	{
		switch (drvno) {
		case 1:
			dwStatus = LSCPCIEJ_IntEnable(hDev[drvno], interrupt_handler1);
			if (WD_STATUS_SUCCESS != dwStatus)
			{
				WDC_Err("Failed to enable the Interrupts1. Error 0x%lx - %s\n",
					dwStatus, Stat2Str(dwStatus));
				//(ErrorMsg("Failed to enable the Interrupts");
				return FALSE;
			}
			break;

		case 2:
			dwStatus = LSCPCIEJ_IntEnable(hDev[drvno], interrupt_handler2);
			if (WD_STATUS_SUCCESS != dwStatus)
			{
				WDC_Err("Failed to enable the Interrupts2. Error 0x%lx - %s\n",
					dwStatus, Stat2Str(dwStatus));
				//ErrorMsg("Failed to enable the Interrupts");
				return FALSE;
			}
			break;
		}
	}
	WDC_Err("finished SetupDMA\n");
	return TRUE;
}//SetupPCIE_DMA

void StartPCIE_DMAWrite(UINT32 drvno)
{	// starts transfer from PCIe board to PCs main RAM
	if (!HWDREQ_EN) {

		//WDC_Err("entered StartPCIE_DMAWrit\n");
		// DCSR: set the Iniator Reset 
		SetDMAReset(drvno);

		/* Flush the I/O caches (see documentation of WDC_DMASyncIo()) */
		//WDC_DMASyncIo(pDMASubBufInfos);
		/****DMA Transfer start***/
		/* Flush the CPU caches (see documentation of WDC_DMASyncCpu()) */
		//WDC_DMASyncCpu(pDMASubBufInfos);

		//SetDMADataPattern();
		/* DDMACR: Start DMA - write to the device to initiate the DMA transfer */
		SetDMAStart(drvno);
	}
}

void CleanupPCIE_DMA(UINT32 drvno)
{
	DWORD dwStatus;
	/* Disable DMA interrupts */
	WDC_IntDisable(hDev[drvno]);
	/* Unlock and free the DMA buffer */
	dwStatus = WDC_DMABufUnlock(pDMASubBufInfos[drvno]);
	if (WD_STATUS_SUCCESS != dwStatus)
	{
		ErrLog("Failed UNlocking a contiguous DMA buffer. Error 0x%lx - %s\n",
			dwStatus, Stat2Str(dwStatus));
		WDC_Err("%s", LSCPCIEJ_GetLastErr());
		return FALSE;
	}
#ifndef _CCDEXAMP
	if (newDLL == 1) {//checks if a new instance called the programm and the buffer is initialized in the dll
		WDC_Err("free in CleanupPCIE_DMA\n");
		free(pDMABigBufBase[drvno]);
	}
#endif
	WDC_Err("Unlock DMABuf Succesfull\n");
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

//weg?
int GetNumofProcessors()
{
	SYSTEM_INFO siSysInfo;
	GetSystemInfo(&siSysInfo);
	return siSysInfo.dwNumberOfProcessors;
}//GetNumofProcessors

//weg?
void RSInterface(UINT32 drvno)
{
	ULONG reg = 0;
	ULONG i = 0;

	ReadLongIOPort(drvno, &reg, 0); //read LCR0 for check length 0xffffffco
	reg = ~reg; //length is inverted

	//set all to zero
	for (i = 0; i < reg / 4; i++) WriteLongS0(drvno, 0, i * 4);
}

BOOL SetBoardVars(UINT32 drvno, UINT32 camcnt, ULONG pixel, ULONG flag816, ULONG xckdelay)
{	//	initiates board   Registers
	//  flag816 =1 for 16 bit (also 14 or 12bit), =2 for 8bit
	//	pclk -> not used
	//  xckdelay set delay between XCK goes high and start of hor. clocks in reg XDLY 0x24
	//	returns TRUE if ok

	BYTE data = 0;
	ULONG reg = 0;
	ULONG i = 0;
	BOOL result = FALSE;

	if (ahCCDDRV[drvno] == INVALID_HANDLE_VALUE)
	{
		WDC_Err("Handle is invalid of drvno: %i", drvno);
		return FALSE;
	}

	//set startval for CTRLA Reg  +slope, IFC=h, VON=1 

	if (!WriteByteS0(drvno, 0x23, S0Addr_CTRLA)) return FALSE;  //write CTRLA reg in S0
	//if (_COOLER) ActCooling(drvno, FALSE); //deactivate cooler
	if (!WriteByteS0(drvno, 0, S0Addr_CTRLB)) return FALSE;;  //write CTRLB reg in S0

	if (!WriteByteS0(drvno, 0, S0Addr_CTRLC)) return FALSE;;  //write CTRLC reg in S0

	/*Pixelsize with matching TLP Count (TLPC).
	Pixelsize = TLPS * TLPC - 1*TLPS
	(TLPS TLP size = 64)
	TLPC	Pixelsize
		2	64
		3	128
		4	192
		5	256
		6	320
		7	384
		8	448
		9	512
		a	576
		b	640
		c	704
		d	768
		e	832
		f	896
		10	960
		11	1024
		12	1088
		13	1152
		14	1216
		15	1280
		16	1344
		17	1408
		18	1472
		19	1536
		1a	1600
		1b	1664
		1c	1728
		1d	1792
		1e	1856
		1f	1920
		20	1984
		21	2048
		22	2112
		*/
	switch (pixel)
	{
	case 192:
		NO_TLPS = 0x4;//4
		break;
	case 320:
		NO_TLPS = 0x6;//6
		break;
	case 576:
		NO_TLPS = 0xa;//a
		break;
	case 1088:
		NO_TLPS = 0x12;//12
		break;
	case 2112:
		NO_TLPS = 0x22;//22
		break;
	default:
		return FALSE;
	}
	//set global vars if driver is there

	aFLAG816[drvno] = flag816;
	aPIXEL[drvno] = pixel;
	aCAMCNT[drvno] = camcnt;
	aXCKDelay[drvno] = xckdelay;

	//	if (flag816 == 1) { reg = 2 * aPIXEL[drvno]; }//16 bit
	//	else { reg = aPIXEL[drvno]; };

	//	OpenShutter(drvno);//set to FFT

	//write pixel to PIXREG  & stop timer & int trig
	reg = pixel;
	if (!WriteLongS0(drvno, reg, S0Addr_PIXREGlow)) return FALSE;; //set pixelreg -> counts received FFwrites and resets XCKI
	reg = camcnt;
	if (!SetS0Reg(reg, 0xF, DmaAddr_CAMCNT, drvno)) return FALSE;

	WDC_Err("camcnt: %i", camcnt);

	// XDLY must be send to camera

	aINIT[drvno] = TRUE;  //init done

	WDC_Err("*** SetBoardVars done.\n");

	return TRUE; //no error
};  // SetBoardVars

#ifndef _DLL
BOOL BufLock(UINT drvno, UINT camcnt, int nob, int nospb)
{
	if (WDC_IntIsEnabled(hDev[drvno]))
		CleanupPCIE_DMA(drvno);
	aCAMCNT[drvno] = camcnt;
	volatile int size = nob * nospb * _PIXEL * sizeof(USHORT);

	pBLOCKBUF[drvno] = calloc(camcnt, nob *  nospb * _PIXEL * sizeof(USHORT));//B! "2 *" because the buffer is just 2/3 of the needed size 
	//pDIODEN = (pArrayT)calloc(nob, nospb * _PIXEL * sizeof(ArrayT));

	if (pBLOCKBUF[drvno] != 0) {
		//pDMABigBufBase = pBLOCKBUF;
		//make init here, that CCDExamp can be used to read the act regs...
		if (!SetBoardVars(drvno, aCAMCNT[drvno], _PIXEL, FLAG816, XCKDELAY))
		{
			ErrorMsg("Error in SetBoardVars");
			return FALSE;
		}
		pDMABigBufBase[drvno] = pBLOCKBUF[drvno];

		if (!SetupPCIE_DMA(drvno, Nospb, Nob))  //get also buffer address
		{
			ErrorMsg("Error in SetupPCIE_DMA");
			return FALSE;
		}


		return TRUE;
	}
	else
		return FALSE;
}
#endif

// ***************************  camera read stuff  **********
//*********************************************************

//weg? wird viel gecalled
void Resort(UINT32 drvno, void* ptarget, void* psource)
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



	for (i = 0; i < larraylength; i++)
		pltarray[i] = plsarray[4 * i + 3];   // CH od 
	if (_HWCH2) // 2cams parallel -> append 2nd array
		for (i = 0; i < larraylength; i++)
			pltarray[i + _PIXEL] = plsarray[_PIXEL * 4 + 4 * i + 3];
	//pltarray[i] = plsarray[4*i+2];  // CH ev



#else	// two channel sensors with 512 pixel

	larraylength = _PIXEL / 2;

	for (i = 0; i < larraylength; i++)
	{ //test:600 fifo
		pltarray[2 * i + 1] = plsarray[4 * i + 1]; //was 2!!must change direction up/dn or modulation
		pltarray[2 * i] = plsarray[4 * i + 2]; //was 3

		if (_HWCH2) // 2cams parallel -> append 2nd array
			for (i = 0; i < larraylength; i++)
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



//weg!?
//replaced by StartReadWithDma
BOOL CallWRFile(UINT32 drvno, void* pdioden, ULONG arraylength, ULONG fkt)
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

	//	pDioden = &DIODEN;// (USHORT*)calloc(arraylength, sizeof(USHORT));


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



//weg!?
//replaced by StartReadWithDma
BOOL ReadPCIEFifo(UINT32 drvno, void* pdioden, long fkt)
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
		for (i = 0; i < _PIXEL; i++)
			*(pReadArray++) = 0;
		return TRUE;
	}

	if (fkt == 5) // set array to i
	{
		for (i = 0; i < _PIXEL; i++)
			*(pReadArray++) = (ArrayT)i;
		return TRUE;
	}

	if (fkt > 9)
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
	if (!CallWRFile(drvno, pReadArray, NO_TLPS * 128, fkt))
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
		for (i = 0; i < _PIXEL; i++)
			* (pDiodenBase++) += *(pDiodenBase2++);
	}

	if (addalloc) free(pReadArray);

	return TRUE; // no Error, all went fine
};  // ReadPCIEFifo






// *********************** PCI board registers

//weg!? verknüpft mir vi ReadFifo->alles weg!
BOOL CallIORead(UINT32 drvno, void* pdioden, ULONG fkt)
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
			for (i = 0; i < aPIXEL[drvno]; i++) //_PIXEL
			{//standard resort for FIFO 1 channel
				ptarray[i * 4] = psarray[i]; // target1 lo byte
				ptarray[i * 4 + 1] = 0;
				ptarray[i * 4 + 2] = 0;
				ptarray[i * 4 + 3] = 0;
			}
		}; //32bit

		if (sizeof(ArrayT) == 2) //target 16 bit
		{
			for (i = 0; i < aPIXEL[drvno]; i++)
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
				for (i = 0; i < pixel; i++) //_PIXEL
				{
					//resort for 16bit 2 channel 2word->1long cha/chb  
					//2.227 hi/lo changed  
					a = 0; b = 2;
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
					for (i = 0; i < pixel; i++) //_PIXEL
					{// resort for 1 channel word->long
						// for HA Module C8061 and C7041
						//if FFT first clks for vclk are not valid
						/* take every value
						ptarray[i*4] =  psarray[2*i+a+firstFFTclks*4]; // target1 lo byte
						ptarray[i*4+1] = psarray[2*i+b+firstFFTclks*4];    // target1 hi byte
						*/
						if (HA_IRSingleCH)
						{
							a = 9; b = 8;
							//IR 256 resort
							ptarray[i * 4] = psarray[16 * i + a]; // target1 lo byte
							ptarray[i * 4 + 1] = psarray[16 * i + b];    // target1 hi byte
							ptarray[i * 4 + 2] = 0;
							ptarray[i * 4 + 3] = 0;
						}
						else
						{
							//FFT resort
							a = 3; b = 2;
							ptarray[i * 4] = psarray[8 * i + a + firstFFTclks * 4]; // target1 lo byte
							ptarray[i * 4 + 1] = psarray[8 * i + b + firstFFTclks * 4];    // target1 hi byte
							ptarray[i * 4 + 2] = 0;
							ptarray[i * 4 + 3] = 0;
						}
					}//for
				}//HA_MODULE
				else //not HA_MODULE
				{

					for (i = 0; i < pixel; i++) //_PIXEL
					{//standard resort for FIFO 1 channel word->long
						//2.227 hi/lo changed -> for FFT and PDA

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
				for (i = 0; i < pixel - 1; i++)
				{//here the 2cam 12bit is appended
					//append 2nd cam first
					a = 0; b = 2;
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
						a = 9; b = 8;
						for (i = 0; i < pixel; i++)
						{//change lo-hi
							ptarray[i * 2] = psarray[i * 16 + a];	// lo byte
							ptarray[i * 2 + 1] = psarray[i * 16 + b];	// hi byte
						}
					}
					else//FFT
					{
						a = 3; b = 2;
						for (i = 0; i < pixel; i++)
						{//change lo-hi
							ptarray[i * 2] = psarray[i * 8 + a + firstFFTclks * 4];	// lo byte
							ptarray[i * 2 + 1] = psarray[i * 8 + b + firstFFTclks * 4];	// hi byte
						}
					}

				}
				else
				{//standard read
					for (i = 0; i < pixel; i++)
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

//weg!? verknüpft mir vi ReadFifo->alles weg!
//  call of the read function - FIFO version
BOOL ReadFifo(UINT32 drvno, void* pdioden, long fkt)
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
		for (i = 0; i < _PIXEL; i++)
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
	if (fkt > 9)
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
		for (i = 0; i < _PIXEL; i++)
			* (pDiodenBase++) += *(pDiodenBase2++);
	}

	if (addalloc) free(pReadArray);

	return TRUE; // no Error, all went fine
};  // ReadFifo


BOOL ReadLongIOPort(UINT32 drvno, ULONG *DWData, ULONG PortOff)
//this function reads the memory mapped data , not the I/O Data
{// reads long of PCIruntime register LCR
	// PortOff: Reg Offset from BaseAdress - in bytes
	// on return -> TRUE if success
	volatile DWORD dwStatus = 0;
	DWORD   ReturnedLength;
	ULONG	PortOffset;

	PortOffset = PortOff;
	dwStatus = WDC_PciReadCfg(hDev[drvno], PortOff, DWData, sizeof(ULONG));
	if (WD_STATUS_SUCCESS != dwStatus)
	{
		WDC_Err("ReadLongIOPort in address 0x%x failed\n", PortOff);
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

BOOL ReadLongS0(UINT32 drvno, ULONG *DWData, ULONG PortOff)
{// reads long on space0 area
	// PortOff: Offset from BaseAdress - in Bytes !
	// return -> TRUE if success
	volatile DWORD dwStatus = 0;
	ULONG	PortOffset;

	//space0 starts at S0-Offset=0x80 in BAR0
	PortOffset = PortOff + 0x80;

	dwStatus = WDC_ReadAddrBlock(hDev[drvno], 0, PortOffset, sizeof(ULONG), DWData, WDC_MODE_8, WDC_ADDR_RW_DEFAULT);
	//the second parameter gives the memory space 0:mem mapped cfg/S0-space 1:I/O cfg/S0-space 2:DMA-space
	if (WD_STATUS_SUCCESS != dwStatus)
	{
		WDC_Err("ReadLongS0 in address 0x%x failed\n", PortOff);
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

BOOL ReadLongDMA(UINT32 drvno, PULONG pDWData, ULONG PortOff)
{// reads long on DMA area
	// PortOff: Offset from BaseAdress - in Bytes !
	// return -> TRUE if success
	volatile DWORD dwStatus = 0;
	DWORD   ReturnedLength;
	ULONG	PortOffset;

	PortOffset = PortOff;
	dwStatus = WDC_ReadAddrBlock(hDev[drvno], 0, PortOff, sizeof(ULONG), pDWData, WDC_MODE_8, WDC_ADDR_RW_DEFAULT);
	//the second parameter gives the memory space 0:mem mapped cfg/S0-space 1:I/O cfg/S0-space 2:DMA-space
	if (WD_STATUS_SUCCESS != dwStatus)
	{
		WDC_Err("ReadLongDMA in address 0x%x failed\n", PortOff);
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

BOOL ReadByteS0(UINT32 drvno, BYTE *data, ULONG PortOff)
{// reads byte in space0 area except r10-r1f
	// PortOff: Offset from BaseAdress - in Bytes !
	// returns TRUE if success
	volatile DWORD dwStatus = 0;
	DWORD   ReturnedLength;
	ULONG	PortOffset;

	PortOffset = PortOff + 0x80;
	dwStatus = WDC_ReadAddrBlock(hDev[drvno], 0, PortOffset, 1/*sizeof(BYTE)*/, data, WDC_MODE_8, WDC_ADDR_RW_DEFAULT);
	//the second parameter gives the memory space 0:mem mapped cfg/S0-space 1:I/O cfg/S0-space 2:DMA-space
	if (WD_STATUS_SUCCESS != dwStatus)
	{
		WDC_Err("ReadByteS0 in address 0x%x failed\n", PortOff);
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

BOOL WriteLongIOPort(UINT32 drvno, ULONG DWData, ULONG PortOff)
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

	dwStatus = WDC_PciWriteCfg(hDev[drvno], PortOff, data, sizeof(ULONG));
	if (WD_STATUS_SUCCESS != dwStatus)
	{
		WDC_Err("WriteLongIOPort in address 0x%x with data: 0x%x failed\n", PortOff, DWData);
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

BOOL WriteLongS0(UINT32 drvno, ULONG DWData, ULONG PortOff)
{	// writes long to space0 register
	// PortOff: Reg Offset from BaseAdress - in bytes
	// returns TRUE if success


	volatile DWORD dwStatus = 0;
	ULONG	PortOffset;
	PULONG data = &DWData;

	PortOffset = PortOff + 0x80;
	dwStatus = WDC_WriteAddrBlock(hDev[drvno], 0, PortOffset, sizeof(ULONG), data, WDC_MODE_8, WDC_ADDR_RW_DEFAULT);
	//the second parameter gives the memory space 0:mem mapped cfg/S0-space 1:I/O cfg/S0-space 2:DMA-space
	if (WD_STATUS_SUCCESS != dwStatus)
	{
		WDC_Err("WriteLongS0 in address 0x%x with data: 0x%x failed\n", PortOff, DWData);
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

BOOL WriteLongDMA(UINT32 drvno, ULONG DWData, ULONG PortOff)
{	// writes long to space0 register
	// PortOff: Reg Offset from BaseAdress - in bytes
	// returns TRUE if success
	BOOL fResult = FALSE;
	sDLDATA WriteData;
	DWORD dwStatus = 0;
	PULONG data = &DWData;

	dwStatus = WDC_WriteAddrBlock(hDev[drvno], 0, PortOff, sizeof(ULONG), data, WDC_MODE_8, WDC_ADDR_RW_DEFAULT);
	//the second parameter gives the memory space 0:mem mapped cfg/S0-space 1:I/O cfg/S0-space 2:DMA-space
	if (WD_STATUS_SUCCESS != dwStatus)
	{
		WDC_Err("WriteLongDMA in address 0x%x with data: 0x%x failed\n", PortOff, DWData);
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

/*
BOOL WriteByteDMA(UINT32 drvno, USHORT DWData, ULONG PortOff)
{	// writes long to space0 register
// PortOff: Reg Offset from BaseAdress - in bytes
// returns TRUE if success
BOOL fResult = FALSE;
sDLDATA WriteData;
ULONG	DataLength;
DWORD   ReturnedLength;
volatile DWORD dwStatus = 0;
PUSHORT data = &DWData;

dwStatus = WDC_WriteAddrBlock(hDev[drvno], 0, PortOff, sizeof(USHORT), data, WDC_MODE_8, WDC_ADDR_RW_DEFAULT);
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

BOOL WriteByteS0(UINT32 drvno, BYTE DWData, ULONG PortOff)
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

	dwStatus = WDC_WriteAddrBlock(hDev[drvno], 0, PortOffset, 1/*sizeof(BYTE)*/, data, WDC_MODE_8, WDC_ADDR_RW_DEFAULT);
	//the second parameter gives the memory space 0:mem mapped cfg/S0-space 1:I/O cfg/S0-space 2:DMA-space
	if (WD_STATUS_SUCCESS != dwStatus)
	{
		WDC_Err("WriteByteS0 in address 0x%x with data: 0x%x failed\n", PortOff, DWData);
		ErrorMsg("WriteByteS0 failed");
		return FALSE;
	}

	//no comparison possible because some Read-Only-Register are changing when we are writing in the same register
	BYTE checkdata;
	ReadByteS0(drvno, &checkdata, PortOff);
	if (*data != checkdata) {
		WDC_Err("\nWriteByteError in address 0x%x:\ndata to write: %x\n", PortOff, DWData);
		WDC_Err("data read: %x\n", checkdata);
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

//weg? verknüpft mit vi...es wird doch aber nur AbotDMA und AboutTLP und so genutz`t?
void AboutDrv(UINT32 drvno)
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
	ReadLongS0(drvno, &S0Data, S0Addr_CTRLA); // Board ID =5053
	S0Data = S0Data >> 16;

	//or
	//S0Data = (UCHAR)ReadByteS0(8); // ID=53
	sprintf_s(pstring, 80, " Board #%i    ID = 0x%I32x", drvno, S0Data);
	if (MessageBox(hWnd, pstring, " Board ID=53 ", MB_OK | MB_ICONEXCLAMATION) == IDOK) {};

	//ReadLongIOPort(drvno, &S0Data, 0); //read LCR0 for check length 0xffffffco
	//S0Data = ~S0Data; //length is inverted
	//GS
	S0Data = 0x07FF;

	if (S0Data == 0) { ErrorMsg("Board #%i  no Space0!", drvno); return; }

	sprintf_s(pstring, 80, "Board #%i     length = 0x%I32x", drvno, S0Data);
	if (MessageBox(hWnd, pstring, "  PCI space0 length=", MB_OK | MB_ICONEXCLAMATION) == IDOK) {};

	if (S0Data >= 0x1F)
	{//if WE -> has space 0x20
		ReadByteS0(drvno, &udata1, 0x1C);
		ReadByteS0(drvno, &udata2, 0x1D);
		ReadByteS0(drvno, &udata3, 0x1E);
		ReadByteS0(drvno, &udata4, 0x1F);
		sprintf_s(pstring, 80, "Board #%i  ven ID = %c%c%c%c", drvno, udata1, udata2, udata3, udata4);
		if (MessageBox(hWnd, pstring, " Board vendor=EBST ", MB_OK | MB_ICONEXCLAMATION) == IDOK) {};
	}

	if (S0Data >= 0x3F)
	{//if 9056 -> has space 0x40
		ReadLongS0(drvno, &S0Data, S0Addr_PCI);
		sprintf_s(pstring, 80, "Board #%i   board version = 0x%I32x", drvno, S0Data);
		if (MessageBox(hWnd, pstring, "Board version ", MB_OK | MB_ICONEXCLAMATION) == IDOK) {};
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
void LowSlope(UINT32 drvno)
{// clear bit D5
	BYTE CtrlA;

	NotBothSlope(drvno);
	ReadByteS0(drvno, &CtrlA, S0Addr_CTRLA);
	CtrlA &= 0x0df;
	WriteByteS0(drvno, CtrlA, S0Addr_CTRLA);
}; //LowSlope

void HighSlope(UINT32 drvno)
{// set bit D5
	BYTE CtrlA;

	NotBothSlope(drvno);
	ReadByteS0(drvno, &CtrlA, S0Addr_CTRLA);
	CtrlA |= 0x20;
	WriteByteS0(drvno, CtrlA, S0Addr_CTRLA);
}; //HighSlope

void BothSlope(UINT32 drvno)
{// set bit D4
	BYTE CtrlA;

	HighSlope(drvno);
	ReadByteS0(drvno, &CtrlA, S0Addr_CTRLA);
	CtrlA |= 0x10;
	WriteByteS0(drvno, CtrlA, S0Addr_CTRLA);
}; //BothSlope

void NotBothSlope(UINT32 drvno)
{// set bit D4
	BYTE CtrlA;
	ReadByteS0(drvno, &CtrlA, S0Addr_CTRLA);
	CtrlA &= 0xEF;
	WriteByteS0(drvno, CtrlA, S0Addr_CTRLA);
}; //NotBothSlope

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Ausgabe eines High-Signals an Pin 17                                      */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void OutTrigLow(UINT32 drvno)
{
	BYTE CtrlA;
	ReadByteS0(drvno, &CtrlA, S0Addr_CTRLA);
	CtrlA &= 0xf7;
	WriteByteS0(drvno, CtrlA, S0Addr_CTRLA);
};						//OutTrigLow

/*---------------------------------------------------------------------------*/


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Ausgabe eines Low-Signals an Pin 17                                       */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void OutTrigHigh(UINT32 drvno)
{
	BYTE CtrlA;
	ReadByteS0(drvno, &CtrlA, S0Addr_CTRLA);
	CtrlA |= 0x08;
	WriteByteS0(drvno, CtrlA, S0Addr_CTRLA);
}; //OutTrigHigh


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Ausgabe eines PulseWidth breiten Rechteckpulses an Pin 17                 */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void OutTrigPulse(UINT32 drvno, ULONG PulseWidth)
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

//weg? ->in vi verknüpft und T elapsed ,  muss bleiben, oder? folgende triggerfkt. auch?
void WaitTrigger(UINT32 drvno, BOOL ExtTrigFlag, BOOL *SpaceKey, BOOL *AbrKey)
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
			ReadByteS0(drvno, &ReadTrigPin, S0Addr_CTRLA);
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
		if (GetAsyncKeyState(VK_ESCAPE))
			Abbr = TRUE;
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

void WaitTriggerShort(UINT32 drvno, BOOL ExtTrigFlag, BOOL *SpaceKey, BOOL *AbrKey)
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
			ReadByteS0(drvno, &ReadTrigPin, S0Addr_CTRLA);
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
		if (GetAsyncKeyState(VK_ESCAPE))
			Abbr = TRUE;
		if (GetAsyncKeyState(VK_SPACE))  Space = TRUE;
#endif
	} while ((!HiEdge) && (!Abbr));
	if (Abbr) *AbrKey = TRUE;	//stops immediately
	if (Space) *SpaceKey = TRUE;	//stops after next trigger
	RSTrigShort(drvno);
};// WaitTrigger^Short


void EnTrigShort(UINT32 drvno)
{//use the short trig pulse FF for ext TrigIn
	UCHAR CtrlA;
	ReadByteS0(drvno, &CtrlA, S0Addr_CTRLA);
	CtrlA |= 0x080;	// set trigger path to FF
	CtrlA |= 0x010; // enable FF
	WriteByteS0(drvno, CtrlA, S0Addr_CTRLA);
}; //EnTrigShort

void RSTrigShort(UINT32 drvno)
{//reset the short trig pulse FF 
	UCHAR CtrlA;
	ReadByteS0(drvno, &CtrlA, S0Addr_CTRLA);
	CtrlA &= 0x0EF; // write CLR to FF
	WriteByteS0(drvno, CtrlA, S0Addr_CTRLA);
	CtrlA |= 0x010; // arm FF again
	WriteByteS0(drvno, CtrlA, S0Addr_CTRLA);
}; //RSTrigShort

void DisTrigShort(UINT32 drvno)
{//use the direct input for ext TrigIn
	UCHAR CtrlA;
	ReadByteS0(drvno, &CtrlA, S0Addr_CTRLA);
	CtrlA &= 0x07F;	// set trigger path to FF
	CtrlA &= 0x0EF; // clr  FF
	WriteByteS0(drvno, CtrlA, S0Addr_CTRLA);
}; //SetTrigShort


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void CloseShutter(UINT32 drvno)   // ehemals IFC = low, in CTRLA
{
	UCHAR CtrlB;
	ReadByteS0(drvno, &CtrlB, S0Addr_CTRLB);
	CtrlB &= ~0x08; // clr bit D3 (MSHT) in CtrlB, ehemals 0x0fd;	/* $FD = 1111 1101 */
	WriteByteS0(drvno, CtrlB, S0Addr_CTRLB);
}; //CloseShutter

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void OpenShutter(UINT32 drvno)   // ehemals IFC = low, in CTRLA
{
	UCHAR CtrlB;
	ReadByteS0(drvno, &CtrlB, S0Addr_CTRLB);
	CtrlB |= 0x08; // set bit D3 (MSUT) in CtrlB
	WriteByteS0(drvno, CtrlB, S0Addr_CTRLB);
}; //OpenShutter


BOOL GetShutterState(UINT32 drvno)
{
	UCHAR CtrlB;
	ReadByteS0(drvno, &CtrlB, S0Addr_CTRLB);
	CtrlB &= 0x08; // read bit D3 (MSUT) in CtrlB
	if (CtrlB == 0) return FALSE;
	return TRUE;
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* set V_ON low (V = V_Fak)                                               */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

//weg? ->vi, bleibt?!
void V_On(UINT32 drvno)
{
	UCHAR CtrlA;
	ReadByteS0(drvno, &CtrlA, S0Addr_CTRLA);
	CtrlA |= 0x01;
	WriteByteS0(drvno, CtrlA, S0Addr_CTRLA);
}; //V_On

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* set V_ON high (V = 1)                                                  */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void V_Off(UINT32 drvno)
{
	UCHAR CtrlA;
	ReadByteS0(drvno, &CtrlA, S0Addr_CTRLA);
	CtrlA &= 0xfe;	// $FE = 1111 1110 
	WriteByteS0(drvno, CtrlA, S0Addr_CTRLA);
}; //V_Off


// optional Opto Couplers
//weg? 
void SetOpto(UINT32 drvno, BYTE ch)
{//sets signal=low
	BYTE ctrlc;
	ReadByteS0(drvno, &ctrlc, S0Addr_CTRLC);
	if (ch == 2) { ctrlc |= 0x04; }
	else ctrlc |= 0x02;
	WriteByteS0(drvno, ctrlc, S0Addr_CTRLC);
}; //SetOpto


void RsetOpto(UINT32 drvno, BYTE ch)
{ //sets signal=high
	BYTE ctrlc;
	ReadByteS0(drvno, &ctrlc, S0Addr_CTRLC);
	if (ch == 2) { ctrlc &= 0xfb; }
	else ctrlc &= 0xfd;
	WriteByteS0(drvno, ctrlc, S0Addr_CTRLC);
}; //RsetOpto


BOOL GetOpto(UINT32 drvno, BYTE ch)
{//no input or low -> high / high input -> low 
	BYTE ctrlc;
	ReadByteS0(drvno, &ctrlc, S0Addr_CTRLC);
	if (ch == 2) { ctrlc &= 0x04; }
	else ctrlc &= 0x02;
	if (ctrlc > 0) return TRUE;
	return FALSE;
}; //GetOpto

//weg? 
void SetDAT(UINT32 drvno, ULONG datin100ns)
{//delay after trigger HW register
	datin100ns |= 0x80000000; // enable delay
	WriteLongS0(drvno, datin100ns, S0Addr_DAT);
}; //SetDAT

void RSDAT(UINT32 drvno)
{//delay after trigger HW register
	WriteLongS0(drvno, 0, S0Addr_DAT);
}; //RSDAT

//weg? 
void SetEC(UINT32 drvno, ULONG ecin100ns)
{//delay after trigger HW register
	//ULONG data = 0;
	//ReadLongS0(drvno, &data, S0Addr_EC);
	//ecin100ns |= data;
	ecin100ns |= 0x80000000; // enable delay
	WriteLongS0(drvno, ecin100ns, S0Addr_EC);
}; //SetEC

void ResetEC(UINT32 drvno)
{//delay after trigger HW register
	WriteLongS0(drvno, 0, S0Addr_EC);
}; //ResetEC

void SetTORReg(UINT32 drvno, BYTE fkt)
{
	BYTE val = 0; //defaut XCK= high during read
	BYTE val2 = 0;
	if (fkt == 0) val = 0x00; // set to XCK
	if (fkt == 1) val = 0x10; // set to REG -> OutTrig
	if (fkt == 2) val = 0x20; // set to TO_CNT
	if (fkt == 3) val = 0x30; // set to XCKDLY
	if (fkt == 4) val = 0x40; // set to FFRead
	if (fkt == 5) val = 0x50; // set to TIN
	if (fkt == 6) val = 0x60; // set to DAT
	if (fkt == 7) val = 0x70; // set to BlockTrig
	if (fkt == 8) val = 0x80; // set to INTRSR
	if (fkt == 9) val = 0x90; // set to INTRSR
	if (fkt == 10) val = 0xa0; // set to INTRSR
	if (fkt == 11) val = 0xb0; // set to BlockOn
	if (fkt == 12) val = 0xc0; // set to MeasureOn
	if (fkt == 13) val = 0xd0; // set to XCKDLYON
	if (fkt == 14) val = 0xe0; // set to VON
	if (fkt == 15) val = 0xf0; // set to IFC

	ReadByteS0(drvno, &val2, 0x2B);
	val2 &= 0x0f; //dont disturb lower bits
	val |= val2;
	WriteByteS0(drvno, val, 0x2B);
}//SetTORReg
//weg?
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
//				for AD set maddr=01, adaddr address of reg
void SendFLCAM(UINT32 drvno, BYTE maddr, BYTE adaddr, USHORT data)
{
	ULONG ldata = 0;

	ldata = maddr;
	ldata = ldata << 8;
	ldata |= adaddr;
	ldata = ldata << 16;
	ldata |= data;
	WriteLongS0(drvno, ldata, S0Addr_DBR);
	ldata |= 0x4000000;		//load val
	WriteLongS0(drvno, ldata, 0x0);
	ldata = 0;		//rs load
	WriteLongS0(drvno, ldata, S0Addr_DBR);
	Sleep(1);

}//SendFLCAM


//weg?
void ClrRead(UINT32 drvno, ULONG fftlines, ULONG zadr, ULONG ccdclrcount)
//normal clear for Kamera is a complete read out
//most cams needs up to 10 complete reads for resetting the sensor
//depends how much it was overexposured
{
	UINT32 i;
	SetIntFFTrig(drvno);
	StartFFTimer(drvno, 1000);
	//	RSFifo(drvno);
	for (i = 0; i < ccdclrcount; i++)
	{
		SWTrig(drvno);
		while (FlagXCKI(drvno))
		{
		}//wait until its ready
	}
	StopFFTimer(drvno);
	RSFifo(drvno);
}; //ClrRead


//weg?
void ClrShCam(UINT32 drvno, UINT32 zadr) //clear for Shutter cameras
{
	pArrayT dummy = NULL;
	CloseShutter(drvno);              //IFC=low
	Sleep(5);
	//GETCCD(drvno, dummy, 0, -1, zadr);
	Sleep(5);
	OpenShutter(drvno);               //IFC=High
	Sleep(5);
}; //ClrShCam

//weg? ->alte treiberfunktion drinne deviceiocontrol
UCHAR ReadKeyPort(UINT32 drvno)
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
void SendCommand(UINT32 drvno, BYTE adr, BYTE data)
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
//weg?
void RSEC(UINT32 drvno)
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

//weg? - >connectet with set Temp
void SendCommand(UINT32 drvno, BYTE adr, BYTE data)
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



//weg?
void SetHiamp(UINT32 drvno, BOOL hiamp)
{

	//	if (_HA_IR) CloseShutter(drvno);// IR uses #11 or #14
	if (hiamp) { V_On(drvno); }	//standard use #11 VON
	else { V_Off(drvno); }
}//SetHiamp





//weg?
BOOL CheckFFTrig(UINT32 drvno) //ext trigger in FF for short pulses
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


//FIFO
//***************  Fifo only Functions   ***************

//weg-> alte dma routine
void StartReadWithDma(UINT32 drvno) {

	//old startringreadthread routine
	//	DMA_bufsizeinbytes = (DMABufSizeInScans + 10) * aPIXEL[drvno] * sizeof(USHORT);// +10;//+100 safty first if it is not right calculated
	//	DMA_bufsizeinbytes = DMABufSizeInScans  * _PIXEL * sizeof(USHORT);// +10;//+100 safty first if it is not right calculated
	if (_HWCH2) DMA_bufsizeinbytes *= 2;

	if (!DMAAlreadyStarted) {
		if (!SetupPCIE_DMA(drvno, UserBufInScans, Nob)) return;
		DMAAlreadyStarted = TRUE;
	}

	//old ReadRingThread Routine
	//read the actual trigger input of trigger or OPTO1 & 2 in CtrlC
	if (RingCtrlRegOfs > 0) ReadByteS0(Ringdrvno, &RingCtrlReg, RingCtrlRegOfs);

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
		for (i = 0; i < _PIXEL; i++)
			*(pReadArray++) = 0;
		return;
	}

	if (_FKT == 5) // set array to i
	{
		for (i = 0; i < _PIXEL; i++)
			*(pReadArray++) = (USHORT)i;
		return;
	}

	if (_FKT > 9)
		return;  // function not implemented


	if ((_FKT == 2) || (_FKT == -1)) //read in our local array ladioden - add and clrread
	{
		//alloc local array dioden, so we don't overwrite our DIODEN
		pReadArray = (USHORT)malloc(aPIXEL[drvno]);
		if (pReadArray == 0)
		{
			ErrorMsg("alloc ADD/CLR Buffer failed");
			return;
		}
		addalloc = TRUE;
	}

	if (_FKT == -1)
	{ // return , nothing else to do
		if (addalloc) free(pReadArray);
		return;
	}

	if (_RESORT) Resort(drvno, pReadArray, pReadArray);  //pixel resort




	//clrread and add fkt=-1 and 2 could not be implemented with dma
	//so we do it here after reading
	if (_FKT == 2) // we must now add our data to DIODEN for online add
	{
		pDiodenBase2 = pReadArray;
		for (i = 0; i < _PIXEL; i++)
			* (pDiodenBase++) += *(pDiodenBase2++);
	}

	if (addalloc) free(pReadArray);
}//StartReadWithDma


//weg? ->and remove vi
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
	if (range > RingFifoDepth - 2)
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
	if (stop < start)
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

	if (stop > 0) //in the future: wait until end reached
	{// ringthread is counting from now up to stop
		RingFetchFutureAct = TRUE;
		do
		{
			//check for ESC keypressed
		} while ((RingFetchFutureAct) && (RingThreadOn));//wait for readthread to reach stop
	}


	//wait until 1st run reached range
	do {} while (RingFirstRun < range + 5);

	//ValMsg(range);

	RingCopyAct = TRUE; //start ring copy
	//wait until data is copied to userbuf
	do {} while ((RingCopyAct) && (RingThreadOn));

	return 0;
}  // ReadRingBlock

//weg?!
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

		memcpy((PUCHAR)pUserBuf, (PUCHAR)pRingBuf + lno * linesize, blocksize1);//before depth
		memcpy((PUCHAR)pUserBuf + blocksize1, (PUCHAR)pRingBuf + lno2 * linesize, blocksize2);//from zero
	}
	else //not wrapped
	{
		//	wrapped = FALSE;
		lno = fetchzero + 1 - range;
		blocksize1 = linesize * range;
		//memset((PUCHAR) pUserBuf,0x22,blocksize1);
		memcpy((PUCHAR)pUserBuf, (PUCHAR)pRingBuf + lno * linesize, blocksize1);
	}

	UserBufValid = FALSE; //set FALSE here if next buffer has a different address
} // CopyRingtoUserBuf



//***** Ring Fifo fkts **************************************
//starts an own thread for writing to a ring buffer of size FifoDepth
//allocates the buffer here
//read is done by ReadRing if tread>twrite
// or by ReadLastRing if tread<twrite

//weg?!
//replaced by StartReadWithDma
void __cdecl ReadRingThread(void *dummy)

{// max priority
	UINT32 i = 0;
	UINT32 j = 0;
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
		if (RingCtrlRegOfs > 0) ReadByteS0(Ringdrvno, &RingCtrlReg, RingCtrlRegOfs);

		linestofetch = ReadFFCounter(Ringdrvno);
		//if(linestofetch > 0)WDC_Err("linestofetch: %i \n", linestofetch);
		//	if (FFValid(Ringdrvno))
		//	if (linestofetch!=0)

		if (linestofetch > 0)
		{
			//linestofetch = ReadFFCounter(Ringdrvno);
			//keep and show how many lines were written to fifo
			if (MaxLineCounter < linestofetch) MaxLineCounter = linestofetch;

			for (i = 1; i <= linestofetch; i++)
			{//read all whats there
				RingFirstRun += 1; //count first run before usercopy may start.
				if (RingFirstRun > MAXINT) RingFirstRun = MAXINT;
				RingWRCnt += 1;
				if (RingWRCnt > RingFifoDepth - 1) RingWRCnt = 0;//wrap counter
#if (!testdata)
				//read fifo is DMA so here it can be back before ready

				//			WriteFile(ahCCDDRV[Ringdrvno], pRingBuf + RingWRCnt*pixel, pixel*2, &lwritten, NULL); //write to PC RAM
				//pRingBuf will crash even if written to DMA write addr reg
				//this function wraps the call, pixel does not matter here, but must be > as 1088
				ReadPCIEFifo(Ringdrvno, (pArrayT)pRingBuf + RingWRCnt * pixel, _FKT); //*linesize in short

				//			ReadFifo(Ringdrvno, (PUCHAR) pRingBuf+RingWRCnt*linesize, _FKT);	
				//			memset((PUCHAR) pRingBuf+RingWRCnt*linesize,RingWRCnt,linesize);//test cnt
				//			*((PUINT32)pRingBuf+RingWRCnt*pixel+4) = RingWRCnt; //test: write cnter to pixel=4

				//if ReadRingBlock want's to read in future
				if (RingFetchFutureAct)
				{
					RingFutureWrCnt += 1;
					if (RingFutureWrCnt > RingCopyStop)
					{
						RingFutureWrCnt = 0;
						RingFetchFutureAct = FALSE;
					}
				}

#else		//testdata
				memset(pRingBuf + RingWRCnt * pixel, RingWRCnt, pixel * 2);//sets bytes
#endif


#if (_TESTRUP) //set outtrig if pixel 1 and 256 found
				if (*(pRingBuf + RingWRCnt * pixel + 4) == 1) OutTrigHigh(Ringdrvno);
				if (*(pRingBuf + RingWRCnt * pixel + 4) == Roilines - 2) OutTrigLow(Ringdrvno);
#endif

#if (_ERRTEST) //test data for integrity	
				//for (k=100;k<4500;k++)
				{
					//if (*(pRingBuf + RingWRCnt*_NO_TLPS * 128 / 2 + 7) < 0x4000) //wert etwa > 0x4000
					if (*(pDMASubBuf[drvno]/*pRingBuf + RingWRCnt*pixel*/ + 1083) != 539)
					{//FetchActLine=TRUE;
						ErrVal = *(/*pRingBuf + RingWRCnt*pixel*/pDMASubBuf[drvno] + 1083);
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

//weg?!
//replaced by StartReadWithDma
void StartRingReadThread(UINT32 drvno, ULONG ringfifodepth, ULONG threadp, __int16 releasems)
{//TODO still thread...has to be converted to threadex
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

	//DMA_bufsizeinbytes = 100 * RAMPAGESIZE * 2;// 100: ringbufsize 2:because  we need the size in bytes
	if (!DMAAlreadyStarted) {
		SetupPCIE_DMA(drvno, UserBufInScans, Nob);
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
//weg?!
//replaced by StartReadWithDma
void StopRingReadThread(void)
{
	RingCopyAct = FALSE;
	RingThreadOn = FALSE;// global variable ends thread and frees mem
}//StopRingFFTimer
//weg?!
void initReadFFLoop(UINT32 drv, UINT32 exptus, UINT8 exttrig, ULONG* Blocks) {
	ULONG dwdata = 0;
	ULONG nos = 0;
	ULONG val = 0;
	BOOL ExTrig = FALSE;

	ReadLongS0(drv, &dwdata, 0x44); //NOS is in reg R1

	nos = dwdata;

	// ErrorMsg("jump to DLLReadFFLoop");

	//WDC_Err("entered DLLReadFFLoop of PCIEcard #%i\n", drv);

	if (!DBGNOCAM)
	{
		//Check if Camera there
		if (!FindCam(drv))
		{
			ErrorMsg("no Camera found");
			return;
		}
	}

	if (exptus == 0) {
		ErrorMsg(" exposure time = 0 ");
		return;
	}

	//reset the internal block counter and ScanIndex before START
	//set to hw stop of timer hwstop=TRUE
	RS_DMAAllCounter(drv, TRUE);
	//reset intr copy buf function

	SubBufCounter[drv] = 0;
	pDMABigBufIndex[drv] = pDMABigBufBase[drv]; // reset buffer index to base we got from InitDMA
	WDC_Err("RESET BIGBUF to%x\n", pDMABigBufIndex[drv]);
	IsrCounter = 0;


	//ErrorMsg("in DLLReadFFLoop - start timer");
	if (exttrig != 0) {
		ExTrig = TRUE;
		SetExtFFTrig(drv);
	}
	else {
		ExTrig = FALSE;
		SetIntFFTrig(drv);

	}
	ReadLongS0(drv, &val, DmaAddr_NOB); //get the needed Blocks
	*Blocks = val;

	//set MeasureOn Bit
	SetS0Reg(0x20, 0x20, DmaAddr_PCIEFLAGS, drv);

}

void allBlocksOnSingleTrigger( UINT32 board_sel, UINT8 btrig_ch, BOOL* StartByTrig ) { // A.M. 22.Okt.19

	while (!(*StartByTrig)) {
		if (keyCheckForBlockTrigger( board_sel )) {
			*StartByTrig = TRUE;
		}
		if (!BlockTrig( 1, btrig_ch )) {
			*StartByTrig = TRUE;
		}
	}
}

void oneTriggerPerBlock(UINT32 board_sel, UINT8 btrig_ch) { // A.M. 22.Okt.19

	if( !BlockTrig(1, btrig_ch) ) { // if trigger is Lo
		while ( !BlockTrig(1, btrig_ch) ) { // wait for Hi
			if ( keyCheckForBlockTrigger(board_sel) ) {
				return; }
		} // leave (and scan)
	}
	else { // if trigger is Hi
		while ( BlockTrig(1, btrig_ch) ) { // wait for Lo
			if (keyCheckForBlockTrigger( board_sel )) {
				return; }
		}
		while ( !BlockTrig(1, btrig_ch) ) { // wait for Hi
			if (keyCheckForBlockTrigger( board_sel )) {
				return; }
		} // leave (and scan)
	}
}

int keyCheckForBlockTrigger(UINT32 board_sel) { // A.M. 22.Okt.19
	if ( GetAsyncKeyState(VK_ESCAPE) ) { //stop if ESC was pressed
		if (board_sel == 1 || board_sel == 3) {
			StopFFTimer(1);
			//SetIntFFTrig(drv);//disable ext input
			SetDMAReset(1);	//Initiator reset
			}
		if (NUMBER_OF_BOARDS == 2 && (board_sel == 2 || board_sel == 3)) {
			StopFFTimer(2);
			//SetIntFFTrig(drv);//disable ext input
			SetDMAReset(2);	//Initiator reset
			}
		return 1;
	}

	if (GetAsyncKeyState(VK_SPACE)) { //start if Space was pressed
		while (GetAsyncKeyState(VK_SPACE) & 0x8000 == 0x8000) {}; //wait for release
		return 2;
		}
	return 0;
	}

void ReadFFLoop(UINT32 board_sel, UINT32 exptus, UINT8 exttrig, UINT8 blocktrigger, UINT8 btrig_ch)
{//const burst loop with DMA initiated by hardware DREQ
	//read nos lines from FIFO
	//exttrig is TRUE if every single scan is triggered externally
	//blocktrigger is TRUE if each block is triggered externally (by Input or btrigger generator
	//btrig_ch=0 -> no read of state is performed
	//btrig_ch=1 is pci tig in
	//btrig_ch=2 is opto1
	//btrig_ch=3 is opto2

	//local declarations
	char string[20] = "";
	void *dummy = NULL;

	BOOL	Abbruch		= FALSE;
	BOOL	Space		= FALSE;
	BOOL	StartByTrig	= FALSE;
	ULONG	lcnt		= 0;
	PUSHORT pdest;
	BYTE	cnt			= 0;
	ULONG	Blocks;
	int		i			= 0;

	if (board_sel == 1 || board_sel == 3)
		initReadFFLoop(1, exptus, exttrig, &Blocks);
	if (NUMBER_OF_BOARDS == 2 && (board_sel == 2 || board_sel == 3))
		initReadFFLoop(2, exptus, exttrig, &Blocks);
	
	//WDC_Err("ReadFFLoop: Block Trigger is set to%d\n", blocktrigger);

	//SetThreadPriority()
	for (int blk_cnt = 0; blk_cnt < Blocks; blk_cnt++) { //do //block read function

		if (1 == blocktrigger) {
			allBlocksOnSingleTrigger( board_sel, btrig_ch, &StartByTrig ); // A.M. 22.Okt.19
		}
		if (2 == blocktrigger) {
			oneTriggerPerBlock( board_sel, btrig_ch ); // A.M. 22.Okt.19
		}
		/*
		if (blocktirgger !=0) {
			BOOL StartByTrig = FALSE;
			while (!StartByTrig) { // check for kill ?
				//	Sleep(1);
				if (GetAsyncKeyState(VK_ESCAPE))
				{ //stop if ESC was pressed
					if (board_sel == 1 || board_sel == 3) {
						StopFFTimer(1);
						//SetIntFFTrig(drv);//disable ext input
						SetDMAReset(1);	//Initiator reset
					}
					if (NUMBER_OF_BOARDS == 2 && (board_sel == 2 || board_sel == 3)) {
						StopFFTimer(2);
						//SetIntFFTrig(drv);//disable ext input
						SetDMAReset(2);	//Initiator reset
					}
				}

				if (GetAsyncKeyState(VK_SPACE))
				{ //start if Space was pressed

					while (GetAsyncKeyState(VK_SPACE) & 0x8000 == 0x8000) {}; //wait for release
					StartbyTrig = TRUE;
					}
				// only look on drv=1
				//wait for low
				if (!BlockTrig(1, btrig_ch))	StartbyTrig = TRUE;
			}//while !StartByTrig
		}
		*/

		if (board_sel == 1 || board_sel == 3) {
			//make signal on trig out plug via PCIEFLAGS:D4 - needed to count Blocks
			ReadLongS0(1, &val, DmaAddr_PCIEFLAGS); //set TrigStart flag for TRIGO signal to monitor the signal
			val |= 0x10;
			WriteLongS0(1, val, DmaAddr_PCIEFLAGS); //make pulse for BlockTrigger
			val &= 0xffffffef;  //set R1(4)
			WriteLongS0(1, val, DmaAddr_PCIEFLAGS); //reset signal
			RS_ScanCounter(1); //reset scan counter for next block - or timer is disabled

			if (board_sel != 3)		//start Timer !!!
				StartFFTimer(1, exptus);
		}
		if (NUMBER_OF_BOARDS == 2 && (board_sel == 2 || board_sel == 3)) {
			//make signal on trig out plug via PCIEFLAGS:D4 - needed to count Blocks
			ReadLongS0(2, &val, DmaAddr_PCIEFLAGS); //set TrigStart flag for TRIGO signal to monitor the signal
			val |= 0x10;
			WriteLongS0(2, val, DmaAddr_PCIEFLAGS); //make pulse for BlockTrigger
			val &= 0xffffffef;  //set R1(4)
			WriteLongS0(2, val, DmaAddr_PCIEFLAGS); //reset signal
			RS_ScanCounter(2); //reset scan counter for next block - or timer is disabled

			if (board_sel != 3)					 //start Timer !!!
				StartFFTimer(2, exptus);
		}
		//for synchronising the both cams
		if (board_sel == 3) {					 //start Timer !!!
			//StartFFTimer(1, exptus);
			//StartFFTimer(2, exptus);

			ULONG data1 = 0;
			ULONG data2 = 0;

			ReadLongS0(1, &data1, 0x08); //reset	
			data1 &= 0xF0000000;
			data1 |= exptus & 0x0FFFFFFF;
			data1 |= 0x40000000;			//set timer on

			ReadLongS0(2, &data2, 0x08); //reset	
			data2 &= 0xF0000000;
			data2 |= exptus & 0x0FFFFFFF;
			data2 |= 0x40000000;			//set timer on

			ULONG	PortOffset = 0x08 + 0x80;

			//old
			//WDC_WriteAddrBlock(hDev[2], 0, PortOffset, sizeof(ULONG), &data2, WDC_MODE_8, WDC_ADDR_RW_DEFAULT);
			//WDC_WriteAddrBlock(hDev[1], 0, PortOffset, sizeof(ULONG), &data1, WDC_MODE_8, WDC_ADDR_RW_DEFAULT);

			//faster
			WDC_WriteAddr32(hDev[1], 0, PortOffset, data1);
			WDC_WriteAddr32(hDev[2], 0, PortOffset, data2);

			/* maybe more faster
			WD_TRANSFER Trans;
			BZERO(Trans);
			Trans.cmdTrans = WP_DWORD;
			Trans.pPort = 0x210;
			WD_Transfer(hDev[1], &Trans);
			*/

			//StartFFTimer(1, exptus);
			//StartFFTimer(2, exptus);
		}

		//	WDC_Err("before scan loop start\n");
			//main read loop - wait here until nos is reached or ESC key
			//if nos is reached the flag RegXCKMSB:b30 = TimerOn is reset by hardware if flag HWDREQ_EN is TRUE
	//extended stoTimer_routine for all variants of one and  two boards
		if (board_sel == 1) {
			while (IsTimerOn(1)) {
				if (GetAsyncKeyState(VK_ESCAPE) | !FindCam(1) | escape_readffloop) // check for kill ?
				{ //stop if ESC was pressed
					StopFFTimer(1);
					SetIntFFTrig(1);//disable ext input
					SetS0Reg(0x00, 0x20, DmaAddr_PCIEFLAGS, 1);	//reset MeasureOn bit
					SetDMAReset(1);
					return;
				}
			}
		}
		if (NUMBER_OF_BOARDS == 2 && board_sel == 2) {
			while (IsTimerOn(2)) {
				if (GetAsyncKeyState(VK_ESCAPE) | !FindCam(2) | escape_readffloop) // check for kill ?
				{ //stop if ESC was pressed
					StopFFTimer(2);
					SetIntFFTrig(2);//disable ext input
					SetS0Reg(0x00, 0x20, DmaAddr_PCIEFLAGS, 2);	//reset MeasureOn bit
					SetDMAReset(2);
					return;
				}
			}
		}

		if (NUMBER_OF_BOARDS == 2 && board_sel == 3) {
			while (IsTimerOn(1) || IsTimerOn(2)) {
				BOOL return_flag_1 = FALSE;
				BOOL return_flag_2 = FALSE;

				if (!return_flag_1) {
					if (GetAsyncKeyState(VK_ESCAPE) | !FindCam(1) | escape_readffloop) // check for kill ?
					{ //stop if ESC was pressed
						StopFFTimer(1);
						SetIntFFTrig(1);//disable ext input
						SetS0Reg(0x00, 0x20, DmaAddr_PCIEFLAGS, 1);	//reset MeasureOn bit
						SetDMAReset(1);
						return_flag_1 = TRUE;
					}
				}

				if (!return_flag_2) {
					if (GetAsyncKeyState(VK_ESCAPE) | !FindCam(2) | escape_readffloop) // check for kill ?
					{ //stop if ESC was pressed
						StopFFTimer(2);
						SetIntFFTrig(2);//disable ext input
						SetS0Reg(0x00, 0x20, DmaAddr_PCIEFLAGS, 2);	//reset MeasureOn bit
						SetDMAReset(2);
						return_flag_2 = TRUE;
					}
				}
				if (return_flag_1 && return_flag_2) return;
			}
			//	WDC_Err("after timer loop \n\n");
				//if the Buffersize is not a multiple of the DMA Buffer the rest has to be taken  
				// must delay or last block has wrong values
				//Sleep(1);
				//ReadLongS0(drv, &blockcnt, DmaAddr_BLOCKINDEX); //get block counts
				//ReadLongS0(drv, &blockcnt, DmaAddr_BLOCKINDEX); //first read sometimes is wrong
		}

		if (board_sel == 1 || board_sel == 3) {
			Sleep(2); //DMA is not ready
			GetLastBufPart(1);
		}
		if (NUMBER_OF_BOARDS == 2 && (board_sel == 2 || board_sel == 3)) {
			Sleep(2); //DMA is not ready
			GetLastBufPart(2);
		}
	}//  block read function
		//while (blockcnt < Blocks);

		//	WDC_Err("blockcnt %x\n", blockcnt);
			//WDC_Err("Blocks%x\n", Blocks);

	if (board_sel == 1 || board_sel == 3) {
		SetS0Reg(0x00, 0x20, DmaAddr_PCIEFLAGS, 1);	//reset MeasureOn bit 
		StopFFTimer(1);
		SetIntFFTrig(1);//disable ext input
	}
	if (NUMBER_OF_BOARDS == 2 && (board_sel == 2 || board_sel == 3)) {
		SetS0Reg(0x00, 0x20, DmaAddr_PCIEFLAGS, 2);	//reset MeasureOn bit 
		StopFFTimer(2);
		SetIntFFTrig(2);//disable ext input
	}


}//ReadFFLoop

//weg?!
//B!void __cdecl ReadFFLoopThread(void *parg)//thread
unsigned int __stdcall ReadFFLoopThread(void *parg)//threadex
{//const burst loop with DMA initiated by hardware DREQ
	//read nos lines from FIFO
	//struct has to be volatile, if not readffloop is always called with drv=1
	volatile struct ffloopparams *par;
	par = parg;
	UINT32 board_sel = par->board_sel;
	UINT32 exptus = par->exptus;
	UINT8 exttrig = par->exttrig;
	UINT8 blocktrigger = par->blocktrigger;
	UINT8 btrig_ch = par->btrig_ch;

	BOARD_SEL = board_sel;
	//local declarations

	SetPriority(READTHREADPriority);  //run in higher priority
	escape_readffloop = FALSE;
	IsrCounter = 0;
	if (contffloop) //run continiously
	{
		do {


			if (GetAsyncKeyState(VK_ESCAPE))
			{ //stop if ESC was pressed

				if (board_sel == 1 || board_sel == 3) {
					StopFFTimer(1);
					//SetIntFFTrig(drv);//disable ext input
					SetDMAReset(1);	//Initiator reset
				}
				if (NUMBER_OF_BOARDS == 2 && (board_sel == 2 || board_sel == 3)) {
					StopFFTimer(2);
					//SetIntFFTrig(drv);//disable ext input
					SetDMAReset(2);	//Initiator reset
				}
				return 1;
			}

			ReadFFLoop(board_sel, exptus, exttrig, blocktrigger, btrig_ch);
			Sleep(100);
		} while (1);
	}
	else {
		ReadFFLoop(board_sel, exptus, exttrig, blocktrigger, btrig_ch);
	}

	//_endthread();//thread
	return 1;//endthreadex is called automatically

}


//  call of the read function if write is faster then read
// ReadRingFifoThread is writing fast to the ring buffer
// if global flag FetchActLine is set, the thread copies last line to pCopyDispBuf

//weg?!
void StartFetchRingBuf(void)
{//pdioden points to the user buffer
	RingCopyAct = TRUE; //set global flag starting the copy
}
//weg?!
BOOL RingThreadIsOFF(void)
{//return state of thread
	//use this for sync to outside
	return RingThreadOFF;
}
//weg?!
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

//weg?!
ULONG GetLastMaxLines(void)
{	//returns the max no. of lines which accumulated
	return MaxLineCounter;
};  // GetLastMaxLines
//weg?
UINT64 GetISRTime(void)
{	//returns the timespan for the ISR
	return MaxISRTime;
};

//weg?!
//  call of the read function if write is slower then read
void ReadRingLine(void* pdioden, UINT32 lno)
{	//reads fifo data to user buffer dioden
	ULONG pixel = aPIXEL[Ringdrvno];
	ULONG linesize = pixel * sizeof(ArrayT);

	memcpy(pdioden, pRingBuf + lno * pixel, linesize);
};  // ReadRingLine



//weg?
ULONG ReadRingCounter()
{
	ULONG diff = 0;
	diff = RingWRCnt - RingRDCnt;
	return  diff;
}//ReadRingCounter
//weg?!
BOOL RingValid()
{
	if ((RingWRCnt - RingRDCnt) == 0) { return FALSE; }
	else return TRUE;
}//RingValid

//weg? -> unser jungo blockreg?
BOOL BlockTrig(UINT32 drv, UINT8 btrig_ch)
{	//return state of trigger in signal
	//global value RingCtrlReg is updated in every loop of ringreadthread
	//in ringreadthread also the board drv is set
	//btrig_ch=0 -> no read of state is performed
	//btrig_ch=1 is pci tig in
	//btrig_ch=2 is opto1
	//btrig_ch=3 is opto2
	BYTE data = 0;
	BOOL state = FALSE;
	RingCtrlRegOfs = 0;
	switch (btrig_ch)
	{
	case 1:
		RingCtrlRegOfs = 4;//CtrlA
		ReadByteS0(drv, &RingCtrlReg, RingCtrlRegOfs);
		if ((RingCtrlReg & 0x40) > 0) return TRUE;
		break;
	case 2:
		RingCtrlRegOfs = 6;//CtrlC
		ReadByteS0(drv, &RingCtrlReg, RingCtrlRegOfs);
		if ((RingCtrlReg & 0x02) > 0) return TRUE;
		break;
	case 3:
		RingCtrlRegOfs = 6;//CtrlC
		ReadByteS0(drv, &RingCtrlReg, RingCtrlRegOfs);
		if ((RingCtrlReg & 0x04) > 0) return TRUE;
		break;
	}
	return FALSE;
}

//weg?!
void SetExtSWTrig(BOOL ext)
{//set the global flag - used in ringreadthread
	if (ext) RRT_ExtTrigFlag = TRUE;
	else RRT_ExtTrigFlag = FALSE;
}//SetExtSWTrig


//*************** Hardware Fifo fkts ******************

void StartFFTimer(UINT32 drvno, ULONG exptime)
{//exptime in microsec
	ULONG data = 0;
	ReadLongS0(drvno, &data, 0x08); //reset	
	data &= 0xF0000000;
	data |= exptime & 0x0FFFFFFF;
	data |= 0x40000000;			//set timer on
	WriteLongS0(drvno, data, 0x08);
}

//weg?!
void SWTrig(UINT32 drvno)
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


void StopFFTimer(UINT32 drvno)
{
	BYTE data = 0;
	ReadByteS0(drvno, &data, 11);
	data &= 0xBF;
	WriteByteS0(drvno, data, 11);
}

BOOL IsTimerOn(UINT32 drvno)
{	//check if timer is active 
	BYTE data = 0;
	ReadByteS0(drvno, &data, S0Addr_XCKMSB);
	data &= 0x40;
	if (data != 0) return TRUE;
	else return FALSE;
}

//weg?! wenn es bleibt, adresse ändern with enum
BOOL FFValid(UINT32 drvno)
{	// not empty & XCK = low -> true
	WDC_Err("FFValid\n");
	BYTE data = 0;
	ReadByteS0(drvno, &data, 0x13);
	data &= 0x80;
	if (data > 0) return TRUE;

	return FALSE;
}


//weg? wenn es bleibt, adresse ändern with enum
BOOL FFFull(UINT32 drvno)
{	// Fifo is full
	WDC_Err("FFFull\n");
	BYTE data = 0;
	ReadByteS0(drvno, &data, 0x13);
	data &= 0x20;
	if (data > 0) return TRUE; //empty

	return FALSE;
}

//weg? wenn es bleibt, adresse ändern with enum
BOOL FFOvl(UINT32 drvno)
{	// had Fifo overflow
	WDC_Err("FFOvl\n");
	BYTE data = 0;
	ReadByteS0(drvno, &data, 0x13);
	data &= 0x08; //0x20; if not saved
	if (data > 0) return TRUE; //empty

	return FALSE;
}

//weg? wenn es bleibt, adresse ändern with enum
BOOL FlagXCKI(UINT32 drvno)
{	// XCKI write to FIFO is active 
	WDC_Err("FlagXCKI\n");
	BYTE data = 0;
	ReadByteS0(drvno, &data, 0x13);
	data &= 0x10;
	if (data > 0) return TRUE; //is running

	return FALSE;
}

//weg? wenn es bleibt, adresse ändern with enum
void RSFifo(UINT32 drvno)
{	//reset FIFO and FFcounter
	BYTE data = 0;
	ReadByteS0(drvno, &data, 0x12);
	data |= 0x80;
	WriteByteS0(drvno, data, 0x12);
	data &= 0x7F;
	WriteByteS0(drvno, data, 0x12);
}


//weg? wenn es bleibt, adresse ändern with enum
void DisableFifo(UINT32 drvno)
{	//reset FIFO and FFcounter
	BYTE data = 0;
	ReadByteS0(drvno, &data, 0x12);
	data |= 0x80;
	WriteByteS0(drvno, data, 0x12);
	//	data &= 0x7F;
	//	WriteByteS0(drvno,data,0x12);
}


//weg? wenn es bleibt, adresse ändern with enum
void EnableFifo(UINT32 drvno)
{	//reset FIFO and FFcounter
	BYTE data = 0;
	ReadByteS0(drvno, &data, 0x12);
	//	data |= 0x80;
	//	WriteByteS0(drvno,data,0x12);
	data &= 0x7F;
	WriteByteS0(drvno, data, 0x12);
}

//weg? wenn es bleibt, adresse ändern with enum
void SetExtFFTrig(UINT32 drvno)  // set external Trigger
{
	BYTE data = 0;
	ReadByteS0(drvno, &data, 11);
	data |= 0x80;
	WriteByteS0(drvno, data, 11);

}//SetExtFFTrig


//weg? wenn es bleibt, adresse ändern with enum
void SetIntFFTrig(UINT32 drvno) // set internal Trigger
{
	BYTE data = 0;
	ReadByteS0(drvno, &data, 11);
	data &= 0x7F;
	WriteByteS0(drvno, data, 11);
}//SetIntFFTrig




//weg? wenn es bleibt, adresse ändern with enum
BYTE ReadFFCounter(UINT32 drvno)
{   //count number of lines in FIFO 
	//max. 16 || capacity of FIFO /(pixel*sizeof(ArrayT)) (7205=8k)
	//new: if _CNT255 ff counts up to 255
	BYTE data = 0;
	ReadByteS0(drvno, &data, 0x14);
	if (_CNT255) {}
	else data &= 0x0f;
	return data;
}

//weg? wenn es bleibt, adresse ändern with enum
void SetupVCLKReg(UINT32 drvno, ULONG lines, UCHAR vfreq)
{
	FFTLINES = lines; //set global var
	if (!HA_MODULE)
	{
		WriteLongS0(drvno, lines * 2, S0Addr_VCLKCTRL );// write no of vclks=2*lines
		WriteByteS0( drvno, vfreq, S0Addr_VCLKFREQ );//  write v freq
		VFREQ = vfreq;//keep freq global
	}
}//SetupVCLKReg

//weg? wenn es bleibt, adresse ändern with enum
void SetupVCLKrt(ULONG vfreq)
{
	VFREQ = vfreq;// VFREQ is global in BOARD
}//Setup VFREQ from setup

//weg? wenn es bleibt, adresse ändern with enum
//not yet working
void SetupVPB(UINT32 drvno, UINT32 range, UINT32 lines, BOOL keep)
{	//partial binning in registers R10,R11 and and R12
	//range: specifies R 1..5
	//lines: number of vertical clks for next read
	//keep: TRUE if scan should be written to FIFO
	WDC_Err("entered SetupVPB with range: 0x%x , lines: 0x%x and keep: %x\n", range, lines, keep);
	ULONG adr = 0;
	lines *= 2; //vclks=lines*2
	switch (range)
	{
	case 1:	adr = 0x68;//0x40;
		if (keep) { lines |= 0x8000; }
		else { lines &= 0x7fff; }
		break;
	case 2:	adr = 0x6A;//0x42;
		if (keep) { lines |= 0x8000; }
		else { lines &= 0x7fff; }
		break;
	case 3:	adr = 0x6C;//0x44;
		if (keep) { lines |= 0x8000; }
		else { lines &= 0x7fff; }
		break;
	case 4:	adr = 0x6E;//0x46;
		if (keep) { lines |= 0x8000; }
		else { lines &= 0x7fff; }
		break;
	case 5:	adr = 0x70;//0x48;
		if (keep) { lines |= 0x8000; }
		else { lines &= 0x7fff; }
		break;
	case 6:	adr = 0x72;//0x4A;
		if (keep) { lines |= 0x8000; }
		else { lines &= 0x7fff; }
		break;
	case 7:	adr = 0x74;//0x4C;
		if (keep) { lines |= 0x8000; }
		else { lines &= 0x7fff; }
		break;
	case 8:	adr = 0x76;//0x4E;
		if (keep) { lines |= 0x8000; }
		else { lines &= 0x7fff; }
		break;
	}
	//TODO make function write word or split in writebytes
	//WriteWordS0(drvno, lines, adr);// write range
	WriteByteS0(drvno, (BYTE)lines, adr);
	WriteByteS0(drvno, (BYTE)(lines >> 8), adr + 1);
}// SetupVPB


//weg? wenn es bleibt, adresse ändern with enum
void SetupDELAY(UINT32 drvno, ULONG delay)
{
	ULONG reg = 0;
	DELAY = delay;// DELAY is global in BOARD

	//new for new boards check for S0 length if DELAY reg is there
	ReadLongIOPort(drvno, &reg, 0); //read LCR0 for check length 0xffffffco
	reg = ~reg; //length is inverted
	if (reg > 0x34)
	{
		WriteByteS0(drvno, DELAY & 0x0ff, 0x34);	//set WRFFDELAY
		WriteByteS0(drvno, DELAY >> 8, 0x35);
	}
}//SetupDELAY

//weg? wenn es bleibt, adresse ändern with enum
void SetupHAModule(BOOL irsingle, ULONG fftlines)
{//set to module for C8061 & C7041
	//set the globals in BOARD
	FFTLINES = fftlines;
	HA_MODULE = TRUE;
	HA_IRSingleCH = irsingle;
}//SetupHAModule



//weg?!! wenn es bleibt, adresse ändern with enum
void PickOneFifoscan(UINT32 drvno, pArrayT pdioden, BOOL* pabbr, BOOL* pspace, ULONG fkt)
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

//weg?!! connected with readffloop wenn es bleibt, adresse ändern with enum
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



//weg? wenn es bleibt, adresse ändern with enum
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



//weg? wenn es bleibt, adresse ändern with enum
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
//weg? stimmt diese Umrechnung noch?
UINT64 LargeToInt(LARGE_INTEGER li)
{ //converts Large to Int64
	UINT64 res = 0;
	res = li.HighPart;
	res = res << 32;
	res = res + li.LowPart;
	return res;
} //LargeToInt


//weg?
LONGLONG InitHRCounter()
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
	WDC_Err("TPS: %lld\n", tps);

	return tps;

} // InitHRCounter

LONGLONG ticksTimestamp()
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


//weg? wird das real benutzt
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
//weg? ->kombi mit closeshutter
void ActCooling(UINT32 drvno, BOOL on)
{//activates cooling with IFC signal
	if (on) { OpenShutter(drvno); }
	else CloseShutter(drvno);
}


BOOL TempGood(UINT32 drvno, UINT32 ch)
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


void SetTemp(UINT32 drvno, ULONG level)
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


// *****   new HS CAM stuff
//weg??? bleibt, oder?
void RS_ScanCounter(UINT32 drv)
{	//RS scan counter
	//is read only - but highest bit=reset
	ULONG dwdata = 0;
	dwdata = 0x80000000; //set
	WriteLongS0(drv, dwdata, DmaAddr_ScanIndex);
	dwdata &= 0x7fffffff; //reset
	WriteLongS0(drv, dwdata, DmaAddr_ScanIndex);
}//RS_ScanCounter

void RS_BlockCounter(UINT32 drv)
{	// RS_BlockCounter 
	//is read only - but highest bit=reset
	ULONG dwdata = 0;
	dwdata = 0x80000000; //set
	WriteLongS0(drv, dwdata, DmaAddr_BLOCKINDEX);
	dwdata &= 0x7fffffff; //reset
	WriteLongS0(drv, dwdata, DmaAddr_BLOCKINDEX);
}//RS_BlockCounter


void RS_DMAAllCounter(UINT32 drv, BOOL hwstop)
{	//drv : board number
	//hwstop: timer is stopped by hardware if nos is reached
	ULONG dwdata = 0;
	//reset the internal intr collect counter
	//Problem: erste scan löst INTR aus
	//aber ohne: erste Block ist 1 zu wenig!0, -> in hardware RS to 0x1

	ReadLongS0(drv, &dwdata, DmaAddr_DMAsPerIntr);
	dwdata |= 0x80000000;
	WriteLongS0(drv, dwdata, DmaAddr_DMAsPerIntr);
	dwdata &= 0x7fffffff;
	WriteLongS0(drv, dwdata, DmaAddr_DMAsPerIntr);

	//reset the internal block counter - is not BLOCKINDEX!
	ReadLongS0(drv, &dwdata, DmaAddr_DmaBufSizeInScans);
	dwdata |= 0x80000000;
	WriteLongS0(drv, dwdata, DmaAddr_DmaBufSizeInScans);
	dwdata &= 0x7fffffff;
	WriteLongS0(drv, dwdata, DmaAddr_DmaBufSizeInScans);

	//reset the scan counter
	RS_ScanCounter(drv);
	RS_BlockCounter(drv);

	if (hwstop) {
		//set Block end stops timer:
		//when SCANINDEX reaches NOS, the timer is stopped by hardware.
		ReadByteS0(drv, &dwdata, DmaAddr_PCIEFLAGS);
		dwdata |= 0x04; //set bit2 for 
		WriteByteS0(drv, dwdata, DmaAddr_PCIEFLAGS);
	}
	else
	{
		//stop only with write to RS_Timer Reg
		ReadByteS0(drv, &dwdata, DmaAddr_PCIEFLAGS);
		dwdata &= 0xFB; //bit2
		WriteByteS0(drv, dwdata, DmaAddr_PCIEFLAGS);
	}
}//RS_DMAAllCounter

//weg?
BOOL FindCam(UINT32 drv)
{//test if SFP module is there and fiber is linked up
	ULONG dwdata = 0;
	ReadLongS0(drv, &dwdata, 0x40);  // read in PCIEFLAGS register
	if ((dwdata & 0x80000000) > 0) { //SFP error
		ErrorMsg("Fiber or Camera error");
		return FALSE;
	}
	dwdata = 0;
	if ((dwdata & 0x40000000) != 0) {
		ErrorMsg("Fiber connection error");
		return FALSE;
	}


	return TRUE;
}//FindCam



//weg?
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
	SendFLCAM(drvno, maddr_adc, adc_ads5294_regaddr_gain_1_to_4, data);	//gain1..4
	data = h;
	data = data << 4;
	data |= d;
	data = data << 4;
	data |= g;
	data = data << 4;
	data |= c;
	SendFLCAM(drvno, maddr_adc, adc_ads5294_regaddr_gain_5_to_8, data);	//gain7..8
}//SetGain
//weg?
void SendFLCAM_DAC(UINT32 drvno, UINT8 ctrl, UINT8 addr, UINT16 data, UINT8 feature)
{	//send data to DAC8568
	//mapping of bits DAC8568:	4 prefix, 4 control, 4 address, 16 data, 4 feature
	UINT16	hi_bytes = 0,
		lo_bytes = 0;
	BYTE	maddr_DAC = 0b11,
		hi_byte_addr = 0x01,
		lo_byte_addr = 0x02;


	if (ctrl & 0x10) //4 ctrl bits => only lower 4 bits allowed
	{
		ErrorMsg("Only values between 0 and 15 are allowed for control bits.");
		return;
	}
	if (addr & 0x10) //4 addr bits => only lower 4 bits allowed
	{
		ErrorMsg("Only values between 0 and 15 are allowed for address bits.");
		return;
	}
	if (feature & 0x10) //4 ctrl bits => only lower 4 bits allowed
	{
		ErrorMsg("Only values between 0 and 15 are allowed for feature bits.");
		return;
	}

	hi_bytes |= 0x0;	//4 prefix bits, first bit always 0 (0xxx)
	hi_bytes <<= 4;
	hi_bytes |= ctrl & 0x0F;	//4 control bits
	hi_bytes <<= 4;
	hi_bytes |= addr & 0x0F;	//4 address bits
	hi_bytes <<= 4;
	hi_bytes |= data >> 12; //4 data bits (upper 4 bits of 16 bits data)

	lo_bytes |= data & 0x0FFF; //12 data bits (lower 12 bits of 16 bit data)
	lo_bytes <<= 4;
	lo_bytes |= feature;	//4 feature bits

	SendFLCAM(drvno, maddr_DAC, hi_byte_addr, hi_bytes);
	SendFLCAM(drvno, maddr_DAC, lo_byte_addr, lo_bytes);
}


void FreeMemInfo(UINT64 *pmemory_all, UINT64 *pmemory_free)
{		//get info: how much memory is installed and how much is available
	// Use to convert bytes to KB
#define DIV 1024

	// Specify the width of the field in which to print the numbers. 
	// The asterisk in the format specifier "%*I64d" takes an integer 
	// argument and uses it to pad and right justify the number.
#define WIDTH 7


	MEMORYSTATUSEX statex;

	statex.dwLength = sizeof(statex);

	GlobalMemoryStatusEx(&statex);

	//_tprintf(TEXT("There is  %*ld percent of memory in use.\n"),
	//	WIDTH, statex.dwMemoryLoad);


	//_tprintf(TEXT("There are %*I64d total KB of physical memory.\n"),
	//	WIDTH, 

	*pmemory_all = statex.ullTotalPhys;


	// DIV);
	//_tprintf(TEXT("There are %*I64d free  KB of physical memory.\n"),
	//WIDTH, 

	*pmemory_free = statex.ullAvailPhys;


	// DIV);


	//_tprintf(TEXT("There are %*I64d total KB of paging file.\n"),
	//	WIDTH, statex.ullTotalPageFile / DIV);
	//_tprintf(TEXT("There are %*I64d free  KB of paging file.\n"),
	//	WIDTH, statex.ullAvailPageFile / DIV);
	//_tprintf(TEXT("There are %*I64d total KB of virtual memory.\n"),
	//	WIDTH, statex.ullTotalVirtual / DIV);
	//_tprintf(TEXT("There are %*I64d free  KB of virtual memory.\n"),
	//	WIDTH, statex.ullAvailVirtual / DIV);

	// Show the amount of extended memory available.

	//_tprintf(TEXT("There are %*I64d free  KB of extended memory.\n"),
	//	WIDTH, statex.ullAvailExtendedVirtual / DIV);
	return;

}

void GetRmsVal(ULONG nos, ULONG *TRMSVals, double *mwf, double *trms)
{
	*trms = 0.0;
	*mwf = 0.0;
	double sumvar = 0.0;
	unsigned int i = 0;

	for (i = 10; i < nos; i++)
	{//get mean val
		*mwf += TRMSVals[i];//for C-Noobs: this is the same like *(TRMSVals+1)
	}
	*mwf /= (nos - 10);
	for (i = 10; i < nos; i++)
	{// get varianz
		*trms = TRMSVals[i];
		*trms = *trms - *mwf;
		*trms *= *trms;
		sumvar += *trms;
	}
	*trms = sumvar / (nos - 10 + 1);
	*trms = sqrt(*trms);

}//GetRmsVal

/* CalcTrms online calc TRMS noise val of pix
 *	drvno	- indentifier of PCIe card
 *	nos		- number of samples
 *	TRMSpix	- pixel for calculating noise (0...1087)
 *	CAMpos	- index for camcount (0...CAMCNT)
 *	*mwf	- pointer for mean value
 *	*trms	- pointer for noise
 */
void CalcTrms(UINT32 drvno, UINT32 nos, ULONG TRMS_pixel, UINT16 CAMpos, double *mwf, double *trms)
{
	ULONG *TRMSVals;

	TRMSVals = calloc(nos, sizeof(ULONG));

	//storing the values of one pix for the rms analysis
	for (int scan = 10; scan < nos; scan++) {
		int TRMSpix_of_current_scan = GetIndexOfPixel(drvno, TRMS_pixel, scan, 0, CAMpos);
		TRMSVals[scan] = pDMABigBufBase[drvno][TRMSpix_of_current_scan];
	}

	//rms analysis
	GetRmsVal(nos, TRMSVals, mwf, trms);

}//CalcTrms


/* GetIndexOfPixel returns the index of a pixel located in pDMABigBufBase
*	drvno	- indentifier of PCIe card
*	pixel	- position in one scan (0...1087)
*	sample	- position in samples (0...nos)
*   block	- position in blocks (0...nob)
*	CAM		- position in camera count (0...CAMCNT)
*/
int GetIndexOfPixel(UINT32 drvno, ULONG pixel, UINT16 sample, UINT16 block, UINT16 CAM)
{
	//init index with base position of pixel
	int index = pixel;
	//position of index at CAM position
	index += CAM * _PIXEL;
	//position of index at sample
	index += sample * aCAMCNT[drvno] * _PIXEL;
	//position of index at block
	index += block * Nospb * aCAMCNT[drvno] * _PIXEL;
	return index;
}//GetIndexOfPixel

/* GetAdressOfPixel returns the address of a pixel located in pDMABigBufBase
*	drvno	- indentifier of PCIe card
*	pixel	- position in one scan (0...1087)
*	sample	- position in samples (0...nos)
*   block	- position in blocks (0...nob)
*	CAM		- position in camera count (0...CAMCNT)
*/
void* GetAddressOfPixel(UINT32 drvno, ULONG pixel, UINT16 sample, UINT16 block, UINT16 CAM)
{
	return &pDMABigBufBase[drvno][GetIndexOfPixel(drvno, pixel, sample, block, CAM)];
}//GetAdressOfPixel

UINT8 WaitforTelapsed(LONGLONG musec)
{
	BOOL Space = FALSE;
	BOOL Abbruch = FALSE;
	LONGLONG expttics = musec * TPS / 1000000;//TPS
	LONGLONG loopcnt = 0;

	//SetPriority(15);		//set priority threadp 1..31 / 15 = highestnormal

	START = ticksTimestamp();
	//WDC_Err("Startzeit: %lld\n", START);
	while ((expttics + START > ticksTimestamp()) && (!Abbruch))
	{// wait until time elapsed
		//WaitTrigger(1, FALSE, &Space, &Abbruch); //check for ESC key - PS2 only
		loopcnt += 1;
	}

	//START = ticksTimestamp();
	//WDC_Err("Endzeit:  %lld\n", START);

	//ResetPriority(); //set global START for next loop

	if (loopcnt < 100) return 1;
	return 0; // loop was too short - exposure time must be increased
}//WaitforTelapsed

/*
* Init routine for Camera System 3001
* Sets register in camera.
* param1: drvno - selects PCIe board
* param2: pixel - pixel amount of camera
* param3: trigger_input - selects trigger input. 0 - XCK, 1 - EXTTRIG, 2 - DAT
* param4: IS_FFT - turns vclk on
* return: void
*/
void InitCamera3001(UINT32 drvno, UINT16 pixel, UINT16 trigger_input, BOOL IS_FFT)
{
	//set camera pixel register
	SendFLCAM(drvno, maddr_cam, cam_adaddr_pixel, pixel);
	//set trigger input
	SendFLCAM(drvno, maddr_cam, cam_adaddr_trig_in, trigger_input);
	//select vclk on
	SendFLCAM(drvno, maddr_cam, cam_adaddr_vclk, (UINT16)IS_FFT);
}

/*
* Init routine for Camera System 3010
* sensor S12198, frame rate 8kHz, 125us exp time
* Sets register in camera and ADC LTC2271.
* param1: drvno - selects PCIe board
* param2: pixel - pixel amount of camera
* param3: trigger_input - selects trigger input. 0 - XCK, 1 - EXTTRIG, 2 - DAT
* param4: adc_mode - 0: normal mode, 2: custom pattern
* param5: custom_pattern - fixed output for testmode, ignored when testmode FALSE
* param6: LED_ON
* param7: GAIN_HIGH
* return: void
*/
void InitCamera3010(UINT32 drvno, UINT16 pixel, UINT8 trigger_input, UINT8 adc_mode, UINT16 custom_pattern, BOOL led_on, BOOL gain_high)
{
	//reset ADC
	SendFLCAM(drvno, maddr_adc, adc_ltc2271_regaddr_reset, adc_ltc2271_msg_reset);
	//output mode
	switch (adc_mode)
	{
	case 2:
		SendFLCAM(drvno, maddr_adc, adc_ltc2271_regaddr_outmode, adc_ltc2271_msg_custompattern);
		SendFLCAM(drvno, maddr_adc, adc_ltc2271_regaddr_custompattern_msb, custom_pattern >> 8);
		SendFLCAM(drvno, maddr_adc, adc_ltc2271_regaddr_custompattern_lsb, custom_pattern & 0x00FF);
		break;
	case 0:
	default:
		SendFLCAM(drvno, maddr_adc, adc_ltc2271_regaddr_outmode, adc_ltc2271_msg_normal_mode);
		break;
	}
	//set camera pixel register
	SendFLCAM(drvno, maddr_cam, cam_adaddr_pixel, pixel);
	//set gain and led
	SendFLCAM(drvno, maddr_cam, cam_adaddr_gain_led, led_on << 4 & gain_high);
	//set trigger input
	SendFLCAM(drvno, maddr_cam, cam_adaddr_trig_in, trigger_input);
}

/*
* Init routine for Camera System 3030
* Sets register in ADC ADS5294.
* param1: drvno - selects PCIe board
* param2: adc_mode - 0: normal mode, 1: ramp, 2: custom pattern
* param3: custom_pattern - only used when adc_mode = 2, lower 14 bits are used as output of ADC
* param4: gain in ADC
* return: void
*/
void InitCamera3030(UINT32 drvno, UINT8 adc_mode, UINT16 custom_pattern, UINT8 gain) {
	//set gain
	SetADGain(drvno, 1, gain, gain, gain, gain, gain, gain, gain, gain);
	switch (adc_mode)
	{
	case 1:
		//ramp
		SendFLCAM(drvno, maddr_adc, adc_ads5294_regaddr_mode, adc_ads5294_msg_ramp);
		break;
	case 2:
		//custom pattern
		//to activate custom pattern the following messages are necessary:
		//d - data
		//at addr 0x25 (mode and higher bits): 0b00000000000100dd
		SendFLCAM(drvno, maddr_adc, adc_ads5294_regaddr_mode, adc_ads5294_msg_custompattern | ((custom_pattern >> 12) & 0x3));
		//at addr 0x26 (lower bits): 0bdddddddddddd0000
		SendFLCAM(drvno, maddr_adc, adc_ads5294_regaddr_custompattern, custom_pattern << 4);
		break;
	case 0:
	default:
		//normal mode
		SendFLCAM(drvno, maddr_adc, adc_ads5294_regaddr_reset, adc_ads5294_msg_reset);
		break;
	}
}


BOOL SetGPXCtrl( UINT drvno, ULONG GPXAddress, UINT8 gpxread ) {
	ULONG regData, tempData;
	//Read old data of ctrl gpx reg
	if (!ReadLongS0( drvno, &regData, 0x58 )) return FALSE;
	//shift gpx addr to the right place for the gpx ctrl reg
	tempData = GPXAddress << 28;
	//set CSexpand bit if read
	if (gpxread != 0)
	{
		tempData |= 0x08000000;
	}
	//hold the other bits of the ctrl gpx reg
	regData &= 0x0FFFFFFF;
	//combine the old ctrl bits with the new address
	regData |= tempData;
	//write to the gpxctrl reg
	if (!WriteLongS0( drvno, regData, 0x58 )) return FALSE;

	return TRUE;
}


void InitGPX( UINT drvno, ULONG delay ) {
	ULONG regData, regNumber, tempData;
	BOOL space, abbr, irf, empty;

	ULONG mask = 0x3FFFF;
	delay &= mask;
	ULONG regVal = 0x08200000 | delay;



	//	SetGPXCtrl(drvno, 0);
	//ReadLongS0(drvno, &regData, 0x5C);

	// setupo GPX chip for mode M

	//reset GPX  ´bit0 in GPXCTRL reg
	ReadLongS0( drvno, &regData, 0x58 );
	regData |= 0x01;
	WriteLongS0( drvno, regData, 0x58 );
	regData &= 0xFFFFFFFE;
	WriteLongS0( drvno, regData, 0x58 ); //reset bit

	/*
	//setup M mode -> time between start and stop
	// does not work, get no empty
	SetGPXCtrl(drvno, 0, 0); // write to reg0
	WriteLongS0(drvno, 0x08B, 0x5C);				//en ring osc, en pos Dstart and pos DStop1
	SetGPXCtrl(drvno, 1, 0); // write to reg1
	WriteLongS0(drvno, 0x00620620, 0x5C);			//ch adjust

	SetGPXCtrl(drvno, 2, 0); // write to reg2
	//	WriteLongS0(drvno, 0x00000FE4, 0x5C);
	WriteLongS0(drvno, 0x00062E04, 0x5C);		//en ch0..5
	SetGPXCtrl(drvno, 3, 0); // write to reg3
	//	WriteLongS0(drvno, 0x01E, 0x5C);
	WriteLongS0(drvno, 0x0000001E, 0x5C);			//res=30 , no ttl test
	SetGPXCtrl(drvno, 4, 0); // write to reg4
	WriteLongS0(drvno, 0x06000300, 0x5C);			// quite M mode, ef on
	SetGPXCtrl(drvno, 5, 0); // write to reg5
	WriteLongS0(drvno, 0x0, 0x0);
	SetGPXCtrl(drvno, 6, 0); // write to reg6
	WriteLongS0(drvno, 0x080000000, 0x5C);			//ecl
	SetGPXCtrl(drvno, 7, 0); // write to reg7
	WriteLongS0(drvno, 0x00001FB4, 0x5C);			//max resolution
	SetGPXCtrl(drvno, 11, 0); // write to reg11
	WriteLongS0(drvno, 0x07FF0000, 0x5C);
	SetGPXCtrl(drvno, 12, 0); // write to reg12
	WriteLongS0(drvno, 0x02000000, 0x5C);
	SetGPXCtrl(drvno, 14, 0); // write to reg14
	WriteLongS0(drvno, 0x0, 0x5C);
	*/

	//setup R mode -> time between start and stop	
	SetGPXCtrl( drvno, 0, 0 ); // write to reg0
	WriteLongS0( drvno, 0x00000080, 0x5C );		//0x80    disable inputs
	SetGPXCtrl( drvno, 1, 0 ); // write to reg1
	WriteLongS0( drvno, 0x0620620, 0x5C );		//0x0620620 adjust

	SetGPXCtrl( drvno, 2, 0 ); // write to reg2
	WriteLongS0( drvno, 0x00062E04, 0x5C );		//62E04  R-mode, en CH0..5 (3 werte)
	SetGPXCtrl( drvno, 3, 0 ); // write to reg3
	WriteLongS0( drvno, 0x00000000, 0x5C );			//0 set to ecl
	SetGPXCtrl( drvno, 4, 0 ); // write to reg4
	WriteLongS0( drvno, 0x02000000, 0x5C );			//0x02000000 EF flag=on
	SetGPXCtrl( drvno, 5, 0 ); // write to reg5
	WriteLongS0( drvno, regVal, 0x5C );			//82000000 retrigger, disable after start-> reduce to 1 val
	//start offset 0x4da=100
	SetGPXCtrl( drvno, 6, 0 ); // write to reg6
	WriteLongS0( drvno, 0x08000001, 0x5C );        // ecl + FILL=1 
	SetGPXCtrl( drvno, 7, 0 ); // write to reg7
	WriteLongS0( drvno, 0x00001FB4, 0x5C );		//res= 27ps
	SetGPXCtrl( drvno, 11, 0 ); // write to reg11
	WriteLongS0( drvno, 0x07ff0000, 0x5C );		//7ff all error flags (layout flag is not connected)
	SetGPXCtrl( drvno, 12, 0 ); // write to reg12
	WriteLongS0( drvno, 0x00000000, 0x5C );		//no ir flags - is used anyway when 1 hit
	SetGPXCtrl( drvno, 14, 0 ); // write to reg14
	WriteLongS0( drvno, 0x0, 0x5C );

	//scharf setzen
	SetGPXCtrl( drvno, 4, 0 ); // write to reg4 
	WriteLongS0( drvno, 0x02400000, 0x5C );				//master reset

	SetGPXCtrl( drvno, 0, 0 ); // write to reg0
	WriteLongS0( drvno, 0x0000008B, 0x5C );		//0x8B > en pos edge inputs = set inputs active

	SetGPXCtrl( drvno, 8, 1 ); //read access follows                 set addr 8 to bus !!!!
	ReadLongS0( drvno, &regData, 0x5C );

}

void AboutGPX( UINT drvno ) {
	HWND hWnd;
	char pstring[80] = "";
	int i, j = 0;
	char fn[1000];
	ULONG regData, regNumber, tempData;
	BOOL space, abbr, irf, empty;
	char LUTS0Reg[16][30] = {
		"Reg0 \t",
		"Reg1 \t",
		"Reg2\t",
		"Reg3\t",
		"Reg4 \t",
		"Reg5\t",
		"Reg6 \t",
		"Reg7\t",
		"Reg8\t",
		"Reg9 \t",
		"Reg10\t",
		"Reg11 \t",
		"Reg12 \t",
		"Reg13\t ",
		"Reg14 \t",
		"Reg15\t"
	}; //Look-Up-Table for the S0 Registers



	hWnd = GetActiveWindow();

	j = sprintf( fn, "GPX- registers   \n" );

	for (i = 0; i < 8; i++)
	{
		SetGPXCtrl( drvno, i, 1 ); //read access follows
		ReadLongS0( drvno, &regData, 0x5C );
		j += sprintf( fn + j, "%s \t: 0x%I32x\n", LUTS0Reg[i], regData );
	}

	for (i = 11; i < 13; i++)
	{
		SetGPXCtrl( drvno, i, 1 ); //read access follows
		ReadLongS0( drvno, &regData, 0x5C );
		j += sprintf( fn + j, "%s \t: 0x%I32x\n", LUTS0Reg[i], regData );
	}


	MessageBox( hWnd, fn, "GPX regs", MB_OK );


	j = sprintf( fn, "delay- registers   \n" );
	i = 0;
	abbr = FALSE;
	empty = FALSE;
	while (!abbr)
	{
		//master reset - 
//		SetGPXCtrl(drvno, 4, 0); // write to reg4
//		WriteLongS0(drvno, 0x02800000, 0x5C);


		WaitTrigger( 1, FALSE, &space, &abbr );
		irf = FALSE;
		//		while (!irf & !abbr)
		{	//wait for IR
//			ReadLongS0(drvno, &regData, 0x58);
//			if ((regData & 0x02) > 0) { irf = TRUE; }; //02=IR , 04=LF, 08=empty
//			WaitTrigger(1, FALSE, &space, &abbr);
		}

		i = 0;
		j = sprintf( fn, "read- regs   \n" );
		//		while (!abbr & i < 1)
		{	//read 2 vals
			i += 1;
			//		ReadLongS0(drvno, &regData, 0x58);
			//		if (regData &= 0x08 != 0) { empty = TRUE; };
			SetGPXCtrl( drvno, 8, 1 ); //read access follows                 lege addr 8 an bus !!!!
			ReadLongS0( drvno, &regData, 0x5C );
			j += sprintf( fn + j, "%d \t: 0x%I32x\n", i, regData );
		}
		MessageBox( hWnd, fn, "GPX regs", MB_OK );

	}

	SetGPXCtrl( drvno, 11, 1 ); //read access follows
	ReadLongS0( drvno, &regData, 0x5C );
	j += sprintf( fn + j, "%s \t: 0x%I32x\n", " stop hits", regData );
	SetGPXCtrl( drvno, 12, 1 ); //read access follows
	ReadLongS0( drvno, &regData, 0x5C );
	j += sprintf( fn + j, "%s \t: 0x%I32x\n", " flags", regData );

	MessageBox( hWnd, fn, "GPX regs", MB_OK );

	//master reset
//	SetGPXCtrl(drvno, 4, 0); // write to reg4
//	WriteLongS0(drvno, 0x06400300, 0x5C);

}
