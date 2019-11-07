/*   **********************************************
	DLL for CCD Camera driver of
	for linking to labview

  Entwicklungsbuero Stresing
  Germany

 Version V1.0  6/2016

  this DLL translates DLL calls from Labview or others
  to the unit Board.c 
  the drivers must have been installed before calling ! 


  for using the PCI Board, copy PCIB\board.c and .h to actual folder
	and make a rebuild all


	V1.0:
	all declarations use stdcall (WinAPI)


	*/

#include <windows.h>

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
#include "board.c"


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


DllAccess UINT8 DLLCCDDrvInit(UINT32 drv)		// init the driver -> true if found
{			//must be called once before any other
if (CCDDrvInit(drv)) {
	WDC_Err("finished DRVInit back in DLL\n");
	return 1;}
return 0;
}

DllAccess void DLLCCDDrvExit(UINT32 drv)		// closes the driver
{
CCDDrvExit(drv);
}


DllAccess UINT8 DLLInitBoard(UINT32 drv,UINT32 pixel,UINT32 flag816,UINT32 pclk, UINT32 xckdelay)		// init the driver -> true if found
{//returns TRUE if success									

//if (!InitBoard(drv)) return 0; //must be called once before any other
//if FIFO pclk=waits : read frequency; waits is set = 0 for max. FIFO read frequency
// NO FIFO version: pclk not used.

if (!SetBoardVars(drv, pixel,flag816, pclk, xckdelay)) return 0; //sets data for transfer
_PIXEL=pixel; // set globals
ADRDELAY=xckdelay;

// AboutS0(drv);

return 1; // no error
}

DllAccess UINT8 DLLReadByteS0(UINT32 drv,UINT8 *data,UINT32 PortOff)	// read byte from Port, PortOff 0..3= Regs of Board
{
if (ReadByteS0(drv,data,PortOff))  {return 1;}
return 0;
}

DllAccess UINT8 DLLWriteByteS0(UINT32 drv,UINT8 DataByte, UINT32 PortOff) // writes DataByte to Port
{
if (WriteByteS0(drv,DataByte, PortOff))   {return 1;}
return 0;
}
DllAccess UINT8 DLLReadLongS0(UINT32 drv,UINT32 *data,UINT32 PortOff)	// read byte from Port, PortOff 0..3= Regs of Board
{
if (ReadLongS0(drv,data,PortOff))  {return 1;}
return 0;
}

DllAccess UINT8 DLLWriteLongS0(UINT32 drv,UINT32 DataL, UINT32 PortOff) // writes DataByte to Port
{
if (WriteLongS0(drv,DataL, PortOff))   {return 1;}
return 0;
}

DllAccess UINT8 DLLReadLongIOPort(UINT32 drv, UINT32 *data,UINT32 PortOff) // writes DataByte to Port
{
if (ReadLongIOPort(drv, data,PortOff))  {return 1;}
return 0;
}

DllAccess UINT8 DLLWriteLongIOPort(UINT32 drv,UINT32 DataL, UINT32 PortOff) // writes DataByte to Port
{
if (WriteLongIOPort(drv,DataL, PortOff))   {return 1;}
return 0;
}




DllAccess void DLLAboutDrv(UINT32 drv)	// displays the version and board ID = test if board is there
{
AboutDrv(drv);
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


//***** system timer sync for constant exposure
DllAccess UINT64 DLLInitSysTimer(void)
{
TPS = InitHRCounter(); // set global variable TPS and check if timer there
return TPS;
}

DllAccess UINT8 DLLWaitforTelapsed(UINT32 musec)
{
BOOL Space=FALSE;
BOOL Abbruch = FALSE;
__int64 expttics = musec * TPS / 1000000;
__int64 loopcnt = 0;

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

DllAccess void DLLSetupDMA(UINT32 drv, void*  pdioden, ULONG nos)
{//setup DMA initiated by hardware DREQ
	//call this func once as it takes time to allocate the resources
	//but be aware: the buffer size and nos is set here and may not be changed later
	//if size changes: DLLClenupDMA and DLLSetupDMA must be called
	//read nos lines from FIFO, no INTR
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
	pDMABigBuf = pdioden;
	pDMABigBufBase = pdioden;
	UserBufInScans = nos;  //one buffer for all


	//ErrorMsg(" Camera found"); //without this message is a crash in the first call ...
	//must before the functionSetupPCIE_DMA or after DLLDrvInit
	//Sleep(1000);
	if (!SetupPCIE_DMA(drv, nos))  //get also buffer address
	{
		ErrorMsg("Error in SetupPCIE_DMA");
		return;
	}
	
	

}//DLLSetupDMA

DllAccess void DLLCleanupDMA(UINT32 drv)
{//free resources
CleanupPCIE_DMA(drv);
}


DllAccess void DLLReadFFLoop(UINT32 drv, UINT32 exptus, UINT32 freq, UINT8 exttrig, UINT8 blocktrigger,UINT8 btrig_ch)
{//const burst loop with DMA initiated by hardware DREQ
	//read nos lines from FIFO
	//
	//local declarations

	char string[20] = "";
	void *dummy = NULL;

	BOOL Abbruch = FALSE;
	BOOL Space = FALSE;
	BOOL ExTrig = FALSE;
	ULONG  lcnt = 0;
	PUSHORT pdest;
	BYTE	cnt = 0;
	int i = 0;
	ULONG dwdata = 0;
	ULONG nos = 0;
	ReadLongS0(drv, &dwdata, 0x44); //NOS is in reg R1
	nos = dwdata;

	// ErrorMsg("jump to DLLReadFFLoop");

	WDC_Err("entered DLLReadFFLoop\n");



	if (!DBGNOCAM)
	{
		//Check if Camera there
		if (!FindCam(DRV))
		{
			ErrorMsg("no Camera found");
			return;
		}
	}



	// only one of exposure time or frequency is permitted to be unequal zero
	if ((exptus != 0) && (freq != 0)) {
		ErrorMsg(" expt + freq ");
		return;
	}

	//calculate exposure time [us] if frequency is given 
	if (freq != 0) {
		if (freq <= 1000000) {
			exptus = 1000000 / freq;// in us
		}
		else {
			ErrorMsg(" freq too high ");
			return;
		}
	}




	//reset the internal block counter and ScanIndex before START
	//set to hw stop of timer hwstop=TRUE
	RS_DMABlockCounter(drv, TRUE);

	//reset intr copy buf function
	ISRCounter = 0;
	SubBufCounter = 0;

	//ErrorMsg("in DLLReadFFLoop - start timer");

	if (exttrig != 0) {	ExTrig = TRUE;
						SetExtFFTrig(drv);}
	else {	ExTrig = FALSE;
			SetIntFFTrig(drv);

	}

//if Block tigger
if (blocktrigger != 0)
{
	BOOL StartbyTrig = FALSE;
	while (!StartbyTrig){ // check for kill ?
		//	Sleep(1);
		if (GetAsyncKeyState(VK_ESCAPE))
		{ //stop if ESC was pressed
			StopFFTimer(drv);
			//SetIntFFTrig(drv);//disable ext input
			SetDMAReset();
			return;
		}
		if (GetAsyncKeyState(VK_SPACE))
		{ //start if Space was pressed
			StartbyTrig = TRUE;
		}
		if (BlockTrig(drv, btrig_ch) )			StartbyTrig = TRUE;
	}//while
}


//start Timer !!!
StartFFTimer(drv, exptus);

WDC_Err("before finished oneshot\n");


while (IsTimerOn(drv)){ // check for kill ?
//	Sleep(1);
	if ( GetAsyncKeyState(VK_ESCAPE))
	{ //stop if ESC was pressed
		StopFFTimer(drv);
		//SetIntFFTrig(drv);//disable ext input
		SetDMAReset();
	}
	//if the Buffersize is not a multiple of the DMA Buffer the rest has to be taken  
	GetLastBufPart();

}

StopFFTimer(drv);

//GetLastBufPart();

WDC_Err("after finished oneshot\n");


//ErrorMsg("stopped");
//stop timer
//SetIntFFTrig(drv);


#if (DMA_CONTIGBUF)
//test with memset if data is transferred
//memset(pDMASubBuf, 20, (nos-1)*_PIXEL*sizeof(USHORT)+100);
//copy data from DMABuf to our data buf
//memcpy(pDMABigBuf, pDMASubBuf, nos*_PIXEL*sizeof(USHORT));
#endif


}//DLLReadFFLoop


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


DllAccess void DLLSetADGain(UINT32 drvno, UINT8 fkt, UINT8 g1, UINT8 g2, UINT8 g3, UINT8 g4, UINT8 g5, UINT8 g6, UINT8 g7, UINT8 g8)
{
	SetADGain(drvno, fkt, g1, g2, g3, g4, g5, g6, g7, g8);

}//SetADGain


DllAccess void DLLSendFLCAM(UINT32 drvno, UINT8 maddr, UINT8 adaddr, UINT16 data)
{
	SendFLCAM(drvno,  maddr,  adaddr,  data);
}


