/*   **********************************************
	DLL for CCD Camera driver of
	for linking to labview

  Entwicklungsbuero Stresing
  Germany

 Version V2.022  3/2017

  this DLL translates DLL calls from Labview or others
  to the unit Board.c 
  the drivers must have been installed before calling ! 


  for using the PCI Board, copy PCIB\board.c and .h to actual folder
	and make a rebuild all

	V1: 	all declarations use stdcall (WinAPI)
	V2.023: with block read functions
		- new function DLLFreeMemInfo
	V2.030 correct timer stop error in P202.2

	*/

#include <windows.h>
#include <tchar.h> // for FreeMem-Function


#include <stdlib.h> 
#include <conio.h>
#include <string.h>
#include <stdio.h>
#include <process.h>	// for Thread example
#include <malloc.h>		// msize


// Make this data shared among all 
// all applications that use this DLL.
//....................................
#pragma data_seg( ".GLOBALS" )
int nProcessCount = 0;
int nThreadCount  = 0;
//#pragma data_seg()
void	*dummy;

#include "GLOBAL.H"
#include "eslscdll.h"
#include "shared_src/board.c"
#include "shared_src/Direct2dViewer_c.h"
//extern volatile PUSHORT pDMABigBufBase[3];


volatile struct ffloopparams params, params2;

BOOL WINAPI DLLMain( HINSTANCE hInstDLL, DWORD dwNotification, LPVOID lpReserved )
{	
    switch(dwNotification)
    {
        case DLL_PROCESS_ATTACH :
               // DLL initialization code goes here. Formerly this 
               // would be in the LibMain function of a 16-bit DLL.
               //..................................................

               nProcessCount++;
               return( TRUE);

        case DLL_PROCESS_DETACH :
               // DLL cleanup code goes here. Formerly this would
               // be in the WEP function of a 16-bit DLL.
               //................................................

			//	CCDDrvExit(DRV);

               nProcessCount--;
               break;                      

        case DLL_THREAD_ATTACH :
               // Special initialization code for new threads goes here.
               // This is so the DLL can "Thread Protect" itself.
               //.......................................................
               nThreadCount++;
               break;

        case DLL_THREAD_DETACH :
               // Special cleanup code for threads goes here.
               //............................................
               nThreadCount--;
               break;
    }

    return( FALSE );
}


DllAccess int DLLGetProcessCount()
{
   return( nProcessCount );
}

DllAccess int DLLGetThreadCount()
{
   return( nThreadCount );
}

// ******************  attached calls to unit BOARD.C

DllAccess  void DLLErrMsgBoxOn(void)
{ErrMsgBoxOn();}
DllAccess  void DLLErrMsgBoxOff(void)
{ErrMsgBoxOff();}


DllAccess UINT8 DLLCCDDrvInit(UINT32 drv) {			//must be called once before any other
													//is called automatically for 2 boards
	if (CCDDrvInit()) {
		WDC_Err("finished DRVInit back in DLL\n");
		return 1;
	}
	return 0;
}

DllAccess UINT8 nDLLCCDDrvInit(void) {			//must be called once before any other
													//is called automatically for 2 boards
	newDLL = 1;
	if (CCDDrvInit()) {
		WDC_Err("finished DRVInit back in DLL\n");
		return NUMBER_OF_BOARDS;
	}
	return 0;
}

DllAccess void DLLCCDDrvExit(UINT32 drv)		// closes the driver
{

CCDDrvExit(drv);

}


DllAccess UINT8 DLLInitBoard(UINT32 drv,UINT32 pixel,UINT32 flag816,UINT32 pclk, UINT32 xckdelay)		// init the driver -> true if found
{//returns TRUE if success									
	//is called automatically for 2 boards
//if (!InitBoard(drv)) return 0; //must be called once before any other
//if FIFO pclk=waits : read frequency; waits is set = 0 for max. FIFO read frequency
// NO FIFO version: pclk not used.
	UINT32 camcnt = 1; //for backward compatibility
InitBoard(drv);
if (!SetBoardVars(drv, camcnt, pixel, flag816, xckdelay)) return 0; //sets data for transfer
//for second board
if (NUMBER_OF_BOARDS == 2){
	InitBoard(2);
	if (!SetBoardVars(2, camcnt, pixel, flag816, xckdelay)) return 0; //sets data for transfer
}

_PIXEL=pixel; // set globals
ADRDELAY=xckdelay;

// AboutS0(drv);

return 1; // no error
}

DllAccess UINT8 n2DLLInitBoard(UINT32 drv, UINT32 camcnt, UINT32 pixel, UINT32 flag816, UINT32 pclk, UINT32 xckdelay)		// init the driver -> true if found
{//returns TRUE if success									
 //is called automatically for 2 boards
 //if (!InitBoard(drv)) return 0; //must be called once before any other
 //if FIFO pclk=waits : read frequency; waits is set = 0 for max. FIFO read frequency
 // NO FIFO version: pclk not used.
	InitBoard(drv);
	if (!SetBoardVars(drv, camcnt, pixel, flag816, xckdelay)) return 0; //sets data for transfer

	_PIXEL = pixel; // set globals
	ADRDELAY = xckdelay;

	// AboutS0(drv);

	return 1; // no error
}

DllAccess UINT8 DLLReadByteS0(UINT32 drv,UINT8 *data,UINT32 PortOff)	// read byte from Port, PortOff 0..3= Regs of Board
{
	if (!ReadByteS0(drv,data,PortOff))  {return 0;}
	return 1;
}

DllAccess UINT8 DLLWriteByteS0(UINT32 drv,UINT8 DataByte, UINT32 PortOff) // writes DataByte to Port
{

	if (!WriteByteS0(drv, DataByte, PortOff))  { return 0; }
	return 1;
}
DllAccess UINT8 DLLReadLongS0(UINT32 drv, UINT32 *data, UINT32 PortOff)	// read byte from Port, PortOff 0..3= Regs of Board
{

	if (!ReadLongS0(drv, data, PortOff)) { return 0; }
	return 1;
}

DllAccess UINT8 DLLWriteLongS0(UINT32 drv, UINT32 DataL, UINT32 PortOff) // writes DataByte to Port
{
	if (!WriteLongS0(drv, DataL, PortOff)) { return 0; }
	return 1;
}

DllAccess UINT8 DLLReadLongDMA(UINT32 drv, UINT32 *data, UINT32 PortOff)	// read byte from Port, PortOff 0..3= Regs of Board
{

	if (!ReadLongDMA(drv, data, PortOff)) { return 0; }
	return 1;
}

DllAccess UINT8 DLLWriteLongDMA(UINT32 drv, UINT32 DataL, UINT32 PortOff) // writes DataByte to Port
{
	if (!WriteLongDMA(drv, DataL, PortOff)) { return 0; }
	return 1;
}

DllAccess UINT8 DLLReadLongIOPort(UINT32 drv, UINT32 *data,UINT32 PortOff) // writes DataByte to Port
{

if (!ReadLongIOPort(drv, data, PortOff))  { return 0; }
return 1;
}

DllAccess UINT8 DLLWriteLongIOPort(UINT32 drv,UINT32 DataL, UINT32 PortOff) // writes DataByte to Port
{

if (!WriteLongIOPort(drv, DataL, PortOff))   { return 0; }
return 1;
}




DllAccess void DLLAboutDrv(UINT32 drv)	// displays the version and board ID = test if board is there
{//is called automatically for 2 boards
AboutDrv(drv);
if (NUMBER_OF_BOARDS == 2) AboutDrv(2);
}

//	functions for managing controlbits in CtrlA register
DllAccess void DLLHighSlope(UINT32 drv)		//set input Trigger slope high
{
HighSlope(drv);

}

DllAccess void DLLLowSlope(UINT32 drv)		//set input Trigger slope low
{
LowSlope(drv);
}

DllAccess void DLLBothSlope(UINT32 drv)		//set input Trigger slope low
{
BothSlope(drv);
}

//following functions are not optimized for 2 cams
DllAccess void DLLOutTrigHigh(UINT32 drv)		//set output Trigger signal high
{
OutTrigHigh(drv);
}

DllAccess void DLLOutTrigLow(UINT32 drv)		//set output Trigger signal low
{
OutTrigLow(drv);
}

DllAccess void DLLOutTrigPulse(UINT32 drv,UINT32 PulseWidth)	// pulses high output Trigger signal
{
OutTrigPulse(drv,PulseWidth);
}

DllAccess void DLLWaitTrigger(UINT32 drv,UINT8 ExtTrigFlag, UINT8 *sk, UINT8 *ek)	// waits for trigger input or Key
{
BOOL bExtTrigFlag = FALSE;
BOOL SpaceKey = FALSE;
BOOL EscapeKey = FALSE;
if (ExtTrigFlag!=0) {bExtTrigFlag=TRUE;}
WaitTrigger(drv,bExtTrigFlag,&SpaceKey,&EscapeKey);


if (SpaceKey == TRUE) {*sk = 1;}
else *sk = 0;
if (EscapeKey == TRUE) {*ek = 1;}
else *ek = 0;
}					   


DllAccess void DLLOpenShutter(UINT32 drv)	// set IFC=high
{
OpenShutter(drv);
}

DllAccess void DLLCloseShutter(UINT32 drv)	// set IFC=low
{
CloseShutter(drv);
}

DllAccess void DLLVOn(UINT32 drv)			// set V_On signal low (V = V_Fak)
{
V_On(drv);
}

DllAccess void DLLVOff(UINT32 drv)			// set V_On signal high (V = 1)
{
V_Off(drv);
}

DllAccess UINT8 DLLReadKeyPort(UINT32 drv)   //before calling, mouse must be deactivated
{
return ReadKeyPort(drv);
}



DllAccess void DLLClrRead(UINT32 drvno, UINT32 fftlines, UINT32 zadr, UINT32 CCDClrCount) 
{
ClrRead(drvno, fftlines, zadr, CCDClrCount); 
}

DllAccess void DLLClrShCam(UINT32 drvno, UINT32 zadr)
{
ClrShCam(drvno, zadr);
}






// ****************   New functions for LabView includes FIFO version


DllAccess void DLLStartTimer(UINT32 drvno,UINT32 exptime)
{//exptime in microsec

StartFFTimer(drvno,exptime);//starts 28bit timer of PCI board with 1ns res

}
DllAccess void DLLSWTrig(UINT32 drvno)						//start a read to FIFO by software
{
SWTrig( drvno);
}
DllAccess void DLLStopFFTimer(UINT32 drvno)					// stop timer
{
StopFFTimer( drvno);
}
DllAccess UINT8 DLLFFValid(UINT32 drvno)						// TRUE if linecounter>0
{
if (FFValid( drvno)==TRUE) {return 1;}
else return 0;
}
DllAccess UINT8 DLLFlagXCKI(UINT32 drvno)						// TRUE if read to FIFO is active
{
if (FlagXCKI( drvno)==TRUE) {return 1;}
else return 0;
}
DllAccess void DLLRSFifo(UINT32 drvno)						// reset FIFO and linecounter
{
RSFifo( drvno);
}
DllAccess void DLLSetExtTrig(UINT32 drvno)					// read to FIFO is triggered by external input I of PCI board
{

SetExtFFTrig( drvno);

}
DllAccess void DLLSetIntTrig(UINT32 drvno)					// read to FIFO is triggered by Timer
{

SetIntFFTrig( drvno);// set hw register

}
DllAccess BYTE DLLReadFFCounter(UINT32 drvno)					// reads 4bit linecounter 
{
return ReadFFCounter( drvno);
}
DllAccess void DLLReadFifo(UINT32 drvno, pArrayT pdioden, INT32 fkt) //read camera
{
ReadFifo( drvno,  pdioden,  fkt);
}
DllAccess void DLLDisableFifo(UINT32 drvno) //switch fifo off
{
DisableFifo( drvno);
}
DllAccess void DLLEnableFifo(UINT32 drvno) //switch fifo off
{
EnableFifo( drvno);
}
DllAccess void DLLPickOneFifoscan(UINT32 drvno,pArrayT pdioden,UINT8* pabbr,UINT8* pspace,INT32 fkt)
{  
BOOL SpaceKey = FALSE;
BOOL EscapeKey = FALSE;
PickOneFifoscan(drvno,pdioden,&EscapeKey,&SpaceKey,fkt);

if (SpaceKey == TRUE) {*pspace = 1;}
else *pspace = 0;
if (EscapeKey == TRUE) {*pabbr = 1;}
else *pabbr = 0;
}

DllAccess UINT8 DLLFFOvl(UINT32 drvno)						// TRUE if linecounter>0
{
if (FFOvl(drvno)==TRUE) {return 1;}
else return 0;
}




DllAccess void DLLSetupVCLK(UINT32 drvno, UINT32 lines, UINT8 vfreq)
{ 

SetupVCLKReg( drvno,  lines,  vfreq);

}//DLLSetupVCLK





//*************  Software ring buffer for multi core
DllAccess void DLLStartRingReadThread(UINT32 drvno, UINT32 ringfifodepth, UINT32 threadp, __int16 releasems)	//starts 28bit timer and get thread
{
StartRingReadThread(drvno, ringfifodepth,threadp, releasems);
}
DllAccess void DLLStopRingReadThread(void)	//starts 28bit timer and get thread
{
StopRingReadThread();
}
DllAccess UINT32 DLLReadRingCounter(UINT32 drvno)
{	
return (ULONG) ReadRingCounter();
}
DllAccess void DLLReadRingLine( pArrayT pdioden, UINT32 lno) //read in ring buffer
{
ReadRingLine(pdioden,lno);
}
DllAccess UINT8 DLLReadRingBlock(void* pdioden, UINT32 start, UINT32 stop)
{//read ring buffer to user buffer relative to act ring pointer
//start,stop<0 : in the past, >0 wait until reached and copy afterwards
return ReadRingBlock(pdioden, start, stop);
}
DllAccess void DLLStartFetchRingBuf(void)
{
StartFetchRingBuf();
}
DllAccess UINT8 DLLFetchLastRingLine(pArrayT pdioden) //read last ring buffer line
{
FetchLastRingLine(pdioden);
return 0;
}
DllAccess UINT8 DLLRingValid(UINT32 drvno)						// TRUE if linecounter>0
{
if (RingValid()==TRUE) {return 1;}
else return 0;
}

DllAccess UINT8 DLLRingThreadIsOFF(void)
{//get thread state of ring read thread
if (RingThreadIsOFF()==TRUE) {return 1;}
else return 0;
}

DllAccess UINT8 DLLBlockTrig(UINT32 drv, UCHAR btrig_ch)
{//get trigger state ext input
	if (BlockTrig(drv, btrig_ch) == TRUE) { return 1; }
else return 0;
}


// for test purposes only: output of 2 strings 
void TestMsg(char testMsg1[20],char testMsg2[20])
{
  if (MessageBox( GetActiveWindow(), testMsg1, testMsg2, MB_OK|MB_ICONEXCLAMATION ) == IDOK ) {};
}

DllAccess UINT8 DLLSetS0Bit(ULONG bitnumber, CHAR Address, UINT32 drvno) {

	return SetS0Bit(bitnumber, Address, drvno);
}

DllAccess UINT8 DLLResetS0Bit(ULONG bitnumber, CHAR Address, UINT32 drvno) {

	return ResetS0Bit(bitnumber, Address, drvno);
}

//***** system timer sync for constant exposure
DllAccess UINT64 DLLInitSysTimer(void)
{
TPS = InitHRCounter(); // set global variable TPS and check if timer there
return TPS;
}

DllAccess UINT8 DLLWaitforTelapsed(LONGLONG musec)
{
BOOL Space=FALSE;
BOOL Abbruch = FALSE;
LONGLONG expttics = musec * TPS / 1000000;
LONGLONG loopcnt = 0;

//SetPriority(15);		//set priority threadp 1..31 / 15 = highestnormal

while ((expttics+START > ticksTimestamp()) && (! Abbruch))
	{// wait until time elapsed
	WaitTrigger(1,FALSE,&Space, &Abbruch); //check for ESC key - PS2 only
	loopcnt += 1;
	}

//ResetPriority();
START = ticksTimestamp(); //set global START for next loop

if (loopcnt<100) return 1;
return 0; // loop was too short - exposure time must be increased
}//DLLWaitforTelapsed


DllAccess UINT64 DLLTicksTimestamp(void)
{
	WDC_Err("entered tickstimestamp\n");
	return ticksTimestamp();}


DllAccess UINT32 DLLTickstous(UINT64 tks)
{return Tickstous(tks);}



//**********   complexer read functions


DllAccess void DLLSetupDMA(UINT32 drv, void*  pdioden, UINT32 nos, UINT32 nob)
{//setup DMA initiated by hardware DREQ
 //call this func once as it takes time to allocate the resources
 //but be aware: the buffer size and nos is set here and may not be changed later
 //if size changes: DLLClenupDMA and DLLSetupDMA must be called
 //read nos lines from FIFO, 
 //copy to just just one very big contigous block: pdioden

 //local declarations
 //	char string[20] = "";
 //	void *dummy = NULL;

	BOOL Abbruch = FALSE;
	BOOL Space = FALSE;
	BOOL ExTrig = FALSE;
	ULONG  lcnt = 0;
	PUSHORT pdest;
	BYTE	cnt = 0;
	int i = 0;
	ULONG dwdata;

	WDC_Err("entered DLLSetupDMA\n");

	if (!DBGNOCAM)
	{
		//Check if Camera there
		if (!FindCam(drv))
		{
			ErrorMsg("no Camera found");
			return;
		}
	}

	//stop all and clear FIFO
	StopFFTimer(drv);
	SetIntFFTrig(drv);
	RSFifo(drv);

	//pass mem pointer to DMA ISR via global pDMABigBuf before calling SetupPCIE_DMA!
	//pDMABigBuf = pdioden;
	pDMABigBufBase[drv] = pdioden;
	UserBufInScans = nos;  //one buffer for all

						   //ErrorMsg(" Camera found"); //without this message is a crash in the first call ...
						   //must before the functionSetupPCIE_DMA or after DLLDrvInit
						   //Sleep(1000);
	if (!SetupPCIE_DMA(drv, nos, nob))  //get also buffer address
	{
		ErrorMsg("Error in SetupPCIE_DMA");
		return;
	}

}//DLLSetupDMA

DllAccess void nDLLSetupDMA(UINT32 drv, UINT32 nos, UINT32 nob)
{//setup DMA initiated by hardware DREQ
	//call this func once as it takes time to allocate the resources
	//but be aware: the buffer size and nos is set here and may not be changed later
	//if size changes: DLLClenupDMA and DLLSetupDMA must be called
	//read nos lines from FIFO, 
	//copy to just  one very big contigous block: pDMABigBufBase

	//local declarations
//	char string[20] = "";
//	void *dummy = NULL;

	BOOL Abbruch = FALSE;
	BOOL Space = FALSE;
	BOOL ExTrig = FALSE;
	ULONG  lcnt = 0;
	PUSHORT pdest;
	BYTE	cnt = 0;
	int i = 0;
	ULONG dwdata;
	UINT64 memory_all_tmp = 0;
	UINT64 memory_free_tmp = 0;
	UINT64* memory_all = &memory_all_tmp;//needed to be initialized for errorfree working
	UINT64* memory_free = &memory_free_tmp;
	INT64 needed_mem;
	INT64 needed_mem_mb;
	INT64 memory_free_mb=0;
	Nob = nob;
	Nospb = nos;


	WDC_Err("entered nDLLSetupDMA with drv: %i nos: %i and nob: %i and camcnt: %i\n", drv, nos, nob, aCAMCNT[drv]);
	
	//checks if the dam routine was already called
	if (WDC_IntIsEnabled(hDev[drv]))
	{
		CleanupPCIE_DMA(drv);
	}
	
	/*if (!DBGNOCAM)
	{
		//Check if Camera there
		if (!FindCam(drv))
		{
			ErrorMsg("no Camera found");
			return;
		}
	}*/
	
	//stop all and clear FIFO
	StopFFTimer(drv);
	SetIntFFTrig(drv);
	RSFifo(drv);

	//pass mem pointer to DMA ISR via global pDMABigBuf before calling SetupPCIE_DMA!
	//pDMABigBuf = pdioden;

	FreeMemInfo(memory_all, memory_free);
	memory_free_mb = *memory_free / (1024 * 1024);
	needed_mem = (INT64)aCAMCNT[drv] * (INT64)nob * (INT64)nos * (INT64)_PIXEL * (INT64)sizeof(USHORT);
	needed_mem_mb = needed_mem / (1024 * 1024);

	//check if enough space is available in the physical ram
	if (*memory_free > (UINT64)needed_mem) {
		pDMABigBufBase[drv] = calloc(aCAMCNT[drv] * (nos) *nob * _PIXEL , sizeof(USHORT));   //!! +1 oder *2 weil sonst absturz im continuous mode
		// sometimes it makes one ISR more, so better to allocate nos+1 thaT IN THIS CASE THE ADDRESS pDMAIndex is valid
		WDC_Err("available memory:%lld MB\n \tmemory needed: %lld MB\n", memory_free_mb, needed_mem_mb);
	}else{
		ErrorMsg("Not enough physical RAM available!");
		WDC_Err("ERROR for buffer %d: available memory: %lld MB \n \tmemory needed: %lld MB\n", NUMBER_OF_BOARDS, memory_free_mb, needed_mem_mb);
	}
		
		//pDIODEN = (pArrayT)calloc(nob, nospb * _PIXEL * sizeof(ArrayT));

		
	//pDMABigBufBase[drv] = pdioden;
	UserBufInScans = nos;  //one buffer for all

	//ErrorMsg(" Camera found"); //without this message is a crash in the first call ...
	//must before the functionSetupPCIE_DMA or after DLLDrvInit
	//Sleep(1000);

	
	if (!SetupPCIE_DMA(drv, nos, nob))  //get also buffer address
	{
		ErrorMsg("Error in SetupPCIE_DMA");
		return;
	}
	
}//nDLLSetupDMA

DllAccess void DLLCleanupPCIE_DMA(UINT32 drv)
{//free resources
	CleanupPCIE_DMA(drv);
}

DllAccess void DLLCleanupDMA(UINT32 drv)
{//free resources
CleanupPCIE_DMA(drv);
}

DllAccess void DLLReturnFrame(UINT32 drv, UINT32 curr_nos, UINT32 curr_nob, UINT16 *pdioden, UINT32 length) {
	ULONGLONG addr=0;
	USHORT* pframe;

	addr = curr_nob * Nospb * _PIXEL + curr_nos * _PIXEL; 
	//complet buffer:	|nob0|nob1|...|curr_nob|...|Nob|
	// one nob:			|nos0|nos1|...|curr_nos|...|Nospb|

	/*
	if (curr_nob > 0)
	{
		addr -= 50 * _PIXEL;
	};
	*/

	pframe = pDMABigBufBase[drv] + addr;  //calc in words
	memcpy(pdioden, pframe, length * sizeof(USHORT));  // length in bytes

	/*
	WDC_Err("FRAME1: drv: %d, curr_nob: %d, _PIXEL: %d\n",drv,curr_nos,_PIXEL);

	WDC_Err("FRAME2: address Buff: 0x%x \n", pDMABigBufBase[drv]);
	WDC_Err("FRAME2: address pdio: 0x%x \n", pdioden);

	WDC_Err("FRAME3: pix42 of ReturnFrame: %d \n", *((USHORT*)pdioden + 420));
	WDC_Err("FRAME3: pix43 of ReturnFrame: %d \n", *((USHORT*)pdioden + 422));
	*/
}

DllAccess void DLLReadFFLoop(UINT32 drv, UINT32 exptus, UINT32 freq, UINT8 exttrig, UINT8 blocktrigger,UINT8 btrig_ch)
{//const burst loop with DMA initiated by hardware DREQ
	//read nos lines from FIFO
	//
	//local declarations

	ReadFFLoop(drv, exptus, exttrig, blocktrigger, btrig_ch);


}//DLLReadFFLoop


DllAccess void nDLLReadFFLoop(UINT32 board_sel, UINT32 exptus, UINT8 exttrig, UINT8 blocktrigger, UINT8 btrig_ch)
//cam_sel = 1 for only use first cam, cam_sel = 2 for sec. cam and cam_sel = 3 for both
{//const burst loop with DMA initiated by hardware DREQ
	//read nos lines from FIFO
	//
	//local declarations
	//is called automatically for 2 boards
	//old: ReadFFLoop(drv, exptus, freq, exttrig, blocktrigger, btrig_ch);
	//HANDLE cam_thread[2];
	//if (cam_sel == 1 || cam_sel == 3){
		//struct has to be volatile, if not readffloop is always called with drv=1
		
		params.board_sel = board_sel;
		params.exptus = exptus;
		params.exttrig = exttrig;
		params.blocktrigger = blocktrigger;
		params.btrig_ch = btrig_ch;

		//_beginthread(ReadFFLoopThread, 0, &params);//thread
		//thread wit prio 15
		_beginthreadex(0, 0, &ReadFFLoopThread, &params, 0, 0);//cam_thread[0] = (HANDLE)_beginthreadex(0, 0, &ReadFFLoopThread, &params, 0, 0);//threadex
//}
/*
	if (NUMBER_OF_BOARDS == 2 && (cam_sel == 2 || cam_sel == 3)){
		//struct has to be volatile, if not readffloop is always called with drv=1
		params2.drv = 2;
		params2.exptus = exptus;
		params2.exttrig = exttrig;
		params2.blocktrigger = blocktrigger;
		params2.btrig_ch = btrig_ch;

		//_beginthread(ReadFFLoopThread, 0, &params2);//thread
		_beginthreadex(0, 0, &ReadFFLoopThread, &params2, 0, 0);//cam_thread[1] = (HANDLE)_beginthreadex(0, 0, &ReadFFLoopThread , &params2, 0, 0); //threadex
	}
	*/
	//Sleep(100);//thread
	//wait until readprocess is finished
	//WaitForMultipleObjects(2, cam_thread, TRUE, INFINITE);//threadex

	//CloseHandle(cam_thread[0]);//threadex
	//CloseHandle(cam_thread[1]);//threadex
}//DLLReadFFLoop

DllAccess void DLLStopFFLoop(void) {
	escape_readffloop = TRUE;
}

DllAccess void DLLSetContFFLoop(UINT8 activate) {
	
	contffloop = activate;//0 or 1
}

//********  cooling functions

DllAccess void DLLActCooling(UINT32 drvno, UINT8 on) 
{
ActCooling(drvno,on);
}

DllAccess UINT8 DLLTempGood(UINT32 drvno, UINT32 ch)
{
return TempGood(drvno,ch);
}

DllAccess void DLLSetTemp(UINT32 drvno, UINT32 level) 
{SetTemp(drvno,level);}

DllAccess void DLLSetEC(UINT32 drvno, UINT64 ecin100ns)
{
	SetEC(drvno, ecin100ns);
}

DllAccess void DLLResetEC(UINT32 drvno)
{
	ResetEC(drvno);
}

DllAccess void DLLSetTORReg(UINT32 drvno, UINT8 fkt)
{SetTORReg(drvno,fkt);}

DllAccess void DLLSetupDELAY(UINT32 drvno, UINT32 delay)
{SetupDELAY(drvno, delay);}

DllAccess void DLLSetISPDA(UINT32 drvno, UINT8 set)
{if (set==0)  
{SetISPDA(drvno,FALSE);}
else SetISPDA(drvno,TRUE);}

DllAccess void DLLSetISFFT(UINT32 drvno, UINT8 set)
{if (set==0)  
{SetISFFT(drvno,FALSE);}
else SetISFFT(drvno,TRUE);}

DllAccess void DLLRsTOREG(UINT32 drvno)
{//reset TOREG
RsTOREG(drvno);
}

DllAccess void DLLSetupHAModule(UINT8 irsingle,UINT32 fftlines)
{//set to module for C8061 & C7041
SetupHAModule((irsingle!=0),fftlines);
}

DllAccess void DLLSetupVPB(UINT32 drvno, UINT32 range, UINT32 lines,UINT8 keep)
{	if (keep!=0) 
		{SetupVPB(drvno, range, lines, TRUE);}
	else
		SetupVPB(drvno, range, lines, FALSE);
}

DllAccess void DLLAboutS0(UINT32 drv)
{
	AboutS0(drv);

}//AboutS0

//optimized for 2 cams
DllAccess void DLLSetADGain(UINT32 drvno, UINT8 fkt, UINT8 g1, UINT8 g2, UINT8 g3, UINT8 g4, UINT8 g5, UINT8 g6, UINT8 g7, UINT8 g8)
{
	SetADGain(drvno, fkt, g1, g2, g3, g4, g5, g6, g7, g8);
}//SetADGain

DllAccess void DLLSendFLCAM(UINT32 drvno, UINT8 maddr, UINT8 adaddr, UINT16 data)
{
	SendFLCAM(drvno,  maddr,  adaddr,  data);
}

DllAccess void DLLSendFLCAM_DAC(UINT32 drvno, UINT8 ctrl, UINT8 addr, UINT16 data, UINT8 feature)
{
	SendFLCAM_DAC(drvno, ctrl, addr, data, feature);
}

DllAccess void DLLFreeMemInfo(UINT64 memory_all, UINT64 memory_free)
{
	FreeMemInfo(memory_all, memory_free);
}

DllAccess void DLLErrorMsg(char ErrMsg[20])
{
	ErrorMsg(ErrMsg);
}

DllAccess void DLLCalcTrms(UINT32 drvno, UINT32 nos, ULONG TRMSpix, UINT16 CAMpos, double *mwf, double *trms)
{
	CalcTrms(drvno, nos, TRMSpix, CAMpos, mwf, trms);
}

DllAccess void DLLStart2dViewer(UINT32 drvno, UINT16 cur_nob, UINT16 cam, UINT pixelAmount, UINT nos)
{
	if (Direct2dViewer == NULL)
	{
		Direct2dViewer = Direct2dViewer_new();
		Direct2dViewer_start2dViewer(
			Direct2dViewer,
			GetActiveWindow(),
			GetAddressOfPixel(drvno, 0, 0, cur_nob, cam),
			pixelAmount,
			nos);
	}
	return;
}

DllAccess void DLLShowNewBitmap(UINT32 drvno, UINT16 cur_nob, UINT16 cam, UINT pixelAmount, UINT nos)
{
	if (Direct2dViewer != NULL) {
		Direct2dViewer_showNewBitmap(
			Direct2dViewer,
			GetAddressOfPixel(drvno, 0, 0, cur_nob, cam),
			pixelAmount,
			nos);
	}
	return;
}
DllAccess void DLLDeinit2dViewer()
{
	if (Direct2dViewer != NULL)
	{
		SendMessage(Direct2dViewer_getWindowHandler(Direct2dViewer), WM_CLOSE, NULL, NULL);
		Direct2dViewer_delete(Direct2dViewer);
		Direct2dViewer = NULL;
	}
	return;
}
DllAccess void DLLSetGammaValue( UINT16 white, UINT16 black )
{
	if (Direct2dViewer != NULL) {
		Direct2dViewer_setGammaValue( Direct2dViewer, white, black );
	}
	return;
}
