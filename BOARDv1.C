/*  Board.C				PCIe Version       V 1.00
	G. Stresing			5/2014

  all functions for reading driver data through DeviceIoControl

  on WINNT:		DMAPCIE.sys


	V1.000	based on PCI V2.30 

 Win C++version:	rename in board.cpp
					add #include "stdafx.h"
						#include "global.h"
  */

//#include "stdafx.h"		// use in C++ only
//#include "global.h"		// use in C++ only
#include "ccdctrl.h"
#include "board.h"
#include <limits.h>
#include <process.h>


#define _CNT255 TRUE   //TRUE if FIFO version has 8bit Counter / TRUE is default

// use LSCPCI1 on PCI Boards
#define	DRIVERNAME	"\\\\.\\LSCPCIE"
//#define	DRIVERNAME	"\\\\.\\PCIe Demo Driver Manager 2"


// globals within board
// don't change values here - are set within functions SetBoardVars...

// handle array for our drivers
HANDLE ahCCDDRV[5] = {INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE};		

ULONG aWAITS[5] = {0,0,0,0,0} ;	// waitstates
ULONG aFLAG816[5] = {1,1,1,1,1};  //AD-Flag
ULONG aPIXEL[5] = {0,0,0,0,0};	// pixel
ULONG aXCKDelay[5] = {1000,1000,1000,1000,1000};	// sensor specific delay
BOOL aINIT[5] = {FALSE,FALSE,FALSE,FALSE,FALSE};


//globals for Ring buffer
ULONG RingFifoDepth = 0;
ULONG RingWRCnt = 0;
ULONG RingRDCnt = 0;
volatile BOOL RingThreadOn=FALSE;
volatile BOOL RingThreadOFF=FALSE;
ULONG Ringdrvno = 1;
volatile ULONG MaxLineCounter=0;
volatile BOOL FetchActLine=FALSE;
volatile BOOL DispBufValid=FALSE;

pArrayT pRingBuf = NULL;
pArrayT pCopyDispBuf = NULL;

ULONG DELAY=0x100;
ULONG VFREQ=Vfreqini;
//defined as global for ReadRingThread
ULONG RRT_FftLines=0;		
BOOL RRT_ExtTrigFlag=FALSE; 
ULONG ERR=0;

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
HANDLE hPROCESS =0;
HANDLE hTHREAD =0;
__int16 RELEASETHREADms = 0;

//general switch to suppress ErrorMsg windows , global in BOARD
BOOL _SHOW_MSG = TRUE;


// ***********     functions    **********************

void ErrMsgBoxOn(void)
{//general switch to suppress error mesage box
_SHOW_MSG = TRUE;
}

void ErrMsgBoxOff(void)
{//general switch to suppress error mesage box
_SHOW_MSG = FALSE;
}

void ErrorMsg(char ErrMsg[40])
{if (_SHOW_MSG)
	{if (MessageBox( GetActiveWindow(), ErrMsg,"ERROR",MB_OK|MB_ICONEXCLAMATION ) == IDOK ) {};}
};

void ValMsg(ULONG val)
{ char AString[40];
	if (_SHOW_MSG)
	{
		sprintf_s(AString,40,"%s%u","val= ",val);	
		if (MessageBox( GetActiveWindow(), AString,"ERROR",MB_OK|MB_ICONEXCLAMATION ) == IDOK ) {};}
};



BOOL CCDDrvInit(UINT drvno)
{// returns true if driver was found
	ULONG MAXDMABUFLENGTH = 0x07fff; //val look in registry driver parameters
									 //depends on os, how big a buffer can be
	BOOL fResult=FALSE;
	char AString[80]="";
	HANDLE hccddrv = INVALID_HANDLE_VALUE ;

	if ((drvno < 1) || (drvno>4)) return FALSE;
/*
	if ((ULONG) _PIXEL > (ULONG)(MAXDMABUFLENGTH/4))  
		{
		ErrorMsg("DMA Buffer length > 0x7fff/4 -> need special driver!");
		return FALSE;
		}
*/	
	sprintf_s(AString,80,"%s%u",DRIVERNAME,drvno);
	hccddrv = CreateFile(AString, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
						 NULL, OPEN_EXISTING,
						FILE_ATTRIBUTE_NORMAL /*FILE_FLAG_DELETE_ON_CLOSE*/, NULL);

	if (hccddrv == (HANDLE) INVALID_HANDLE_VALUE)
		{return FALSE;}	// false if LSCPCIn not there 

	//save handle in global array
	ahCCDDRV[drvno]=hccddrv;
	return TRUE;	  // no Error, driver found
}; //CCDDrvInit


void CCDDrvExit(UINT drvno)
{
	if (ahCCDDRV[drvno]!=INVALID_HANDLE_VALUE)
		CloseHandle(ahCCDDRV[drvno]);	   // close driver
	ahCCDDRV[drvno] = INVALID_HANDLE_VALUE;
	aINIT[drvno] = FALSE;
};


BOOL InitBoard(UINT drvno)
	{		// inits PCI Board and gets all needed addresses
			// and gets Errorcode if any
	ULONG Errorcode = Error_notinitiated;
	BOOL fResult = FALSE;
	DWORD   ReturnedLength;
	ULONG	ctrlcode = 1; // Init Board


	fResult = DeviceIoControl(ahCCDDRV[drvno],IOCTL_SetFct,  // read error code
							&ctrlcode,        // Buffer to driver.
                            sizeof(ctrlcode),
							&Errorcode,sizeof(Errorcode),&ReturnedLength,NULL);
	if (! fResult)
		{ErrorMsg("InitBoard failed");};
	
	// these error messages are for inspection of the driver only
	// they can be omitted because here should be no error
	if (Errorcode==NoError)
			{return TRUE; }  // everything went fine

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
	return FALSE;
	};  // InitBoard

char CntBoards(void)
{//get how many PCI boards are there
int i=0;
int foundBoards=0;

for (i=1;i<5;i++) // check for max 4
	if (CCDDrvInit(i)) //call once per cam
		{
		foundBoards += 1;
		}
return foundBoards;
}//CntBoards



void SetupDMA(void)
{
	BOOL fResult = FALSE;
	DWORD   ReturnedLength;
	ULONG	ctrlcode = 1; // Init Board
	ULONG   Errorcode = 0;

	ctrlcode = 0x01;
	fResult = DeviceIoControl(ahCCDDRV[1], IOCTL_INITIATOR_RESET,  // assert reset
		&ctrlcode,        // Buffer to driver.
		sizeof(ctrlcode),
		&Errorcode, sizeof(Errorcode), &ReturnedLength, NULL);
	if (!fResult)  { ErrorMsg("startDMA failed"); };

	ctrlcode = 0x01;
	fResult = DeviceIoControl(ahCCDDRV[1], IOCTL_CLEAR_INITIATOR_RESET,  // deassert reset
		&ctrlcode,        // Buffer to driver.
		sizeof(ctrlcode),
		&Errorcode, sizeof(Errorcode), &ReturnedLength, NULL);
	if (!fResult)  { ErrorMsg("startDMA failed"); };

	ctrlcode = pDIODEN;//set data array address
	fResult = DeviceIoControl(ahCCDDRV[1], IOCTL_SET_DMA_WRITE_ADDR,  
		&ctrlcode,        // Buffer to driver.
		sizeof(ctrlcode),
		&Errorcode, sizeof(Errorcode), &ReturnedLength, NULL);
	if (!fResult)  { ErrorMsg("SetupAddress failed"); };

	/*
	ctrlcode = _PIXEL;
	//calculate TLP size and TLP counts depending on _PIXEL
	fResult = DeviceIoControl(ahCCDDRV[1], IOCTL_SETUP_DMA_PCIE_PAYLOADVALS,  // TLP size
		&ctrlcode,        // Buffer to driver.
		sizeof(ctrlcode),
		&Errorcode, sizeof(Errorcode), &ReturnedLength, NULL);
	if (!fResult)  { ErrorMsg("SetupDMA failed"); };
	*/
	
	ctrlcode = 0x20;  //=32DWORDs = 128Bytes
	fResult = DeviceIoControl(ahCCDDRV[1], IOCTL_SET_DMA_WRITE_TLPSIZE,  // TLP size
		&ctrlcode,        // Buffer to driver.
		sizeof(ctrlcode),
		&Errorcode, sizeof(Errorcode), &ReturnedLength, NULL);
	if (!fResult)  { ErrorMsg("SetupDMA failed"); };

	ctrlcode = 0x011;  //0x10=1024 -> 0x11= 1088 pixel _PIXEL auf 1200
						//0x11= 17*128B = 2176B = 1088 WORDs = 544 DWORDs
	fResult = DeviceIoControl(ahCCDDRV[1], IOCTL_SET_DMA_WRITE_TLPCOUNT,  // TLP count
		&ctrlcode,        // Buffer to driver.
		sizeof(ctrlcode),
		&Errorcode, sizeof(Errorcode), &ReturnedLength, NULL);
	if (!fResult)  { ErrorMsg("SetupDMA failed"); };
	
	}

void StartWriteDMA(void)
{
	BOOL fResult = FALSE;
	DWORD   ReturnedLength;
	ULONG	ctrlcode = 1; 
	ULONG   Errorcode = 0;

	ctrlcode = 0x01; //code wird ins DCSR2 geschrieben - 0x01=start write
	fResult = DeviceIoControl(ahCCDDRV[1], IOCTL_DMA_START,  // read error code
		&ctrlcode,        // Buffer to driver.
		sizeof(ctrlcode),
		&Errorcode, sizeof(Errorcode), &ReturnedLength, NULL);
	if (!fResult)  {ErrorMsg("startDMA failed");};
}

ULONG GetWriteStat(void)
{
	BOOL fResult = FALSE;
	DWORD   ReturnedLength;
	ULONG	ctrlcode = 0; 
	ULONG   data = 0;

	ctrlcode = 0x01; //is DCR2 as *4
	fResult = DeviceIoControl(ahCCDDRV[1], IOCTL_GET_DMA_REGISTER,  // read error code
		&ctrlcode,        // Buffer to driver.
		sizeof(ctrlcode),
		&data, sizeof(data), &ReturnedLength, NULL);
	if (!fResult)  { ErrorMsg("GetWriteStat failed"); };

	return data;
}

void Cleanup(void)
{
	BOOL fResult = FALSE;
	DWORD   ReturnedLength;
	ULONG	ctrlcode = 0;
	ULONG   data = 0;

	ctrlcode = 0x01; //is DCR2 as *4
	fResult = DeviceIoControl(ahCCDDRV[1], IOCTL_CLEANUP,  // read error code
		&ctrlcode,        // Buffer to driver.
		sizeof(ctrlcode),
		&data, sizeof(data), &ReturnedLength, NULL);
	if (!fResult)  { ErrorMsg("Cleanup failed"); };

	return data;
}

void DisableINTR(void)
{
	BOOL fResult = FALSE;
	DWORD   ReturnedLength;
	ULONG	ctrlcode = 0;
	ULONG   Errorcode = 0;

	ctrlcode = 0x0;
	fResult = DeviceIoControl(ahCCDDRV[1], IOCTL_DISABLE_INTERRUPTS,  // read error code
		&ctrlcode,        // Buffer to driver.
		sizeof(ctrlcode),
		&Errorcode, sizeof(Errorcode), &ReturnedLength, NULL);
	if (!fResult)  { ErrorMsg("disable INTR failed"); };
}

void EnableINTR(void)
{
	BOOL fResult = FALSE;
	DWORD   ReturnedLength;
	ULONG	ctrlcode = 0;
	ULONG   Errorcode = 0;

	ctrlcode = 0x0;
	fResult = DeviceIoControl(ahCCDDRV[1], IOCTL_ENABLE_INTERRUPTS,  // read error code
		&ctrlcode,        // Buffer to driver.
		sizeof(ctrlcode),
		&Errorcode, sizeof(Errorcode), &ReturnedLength, NULL);
	if (!fResult)  { ErrorMsg("enable INTR failed"); };
}



BOOL ActMouse(UINT drvno)
	{		// inits PCI Board and gets all needed addresses
			// and gets Errorcode if any
#ifndef _DEBUG
	ULONG Errorcode = Error_notinitiated;
	BOOL fResult = FALSE;
	DWORD   ReturnedLength;
	ULONG	ctrlcode = 3; // Activate mouse


	fResult = DeviceIoControl(ahCCDDRV[drvno],IOCTL_SetFct,  // read error code
							&ctrlcode,        // Buffer to driver.
                            sizeof(ctrlcode),
							&Errorcode,sizeof(Errorcode),&ReturnedLength,NULL);
	if (! fResult)
		ErrorMsg("ActMouse failed");

	if (Errorcode!=NoError)
		{ErrorMsg(" Mouse Error ");
	return FALSE;}

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


	fResult = DeviceIoControl(ahCCDDRV[drvno],IOCTL_SetFct,  // read error code
							&ctrlcode,        // Buffer to driver.
                            sizeof(ctrlcode),
							&Errorcode,sizeof(Errorcode),&ReturnedLength,NULL);
	if (! fResult)
		ErrorMsg("DeactMouse failed");

	if (Errorcode!=NoError)
		{ErrorMsg(" Mouse Error ");
	return FALSE;}

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


BOOL SetBoardVars(UINT drvno, BOOL sym, BOOL burst,ULONG pixel, ULONG waits,ULONG flag816,ULONG pportadr,
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
	USHORT buswidth = 0x840; //default 8 bit


	if (ahCCDDRV[drvno] == INVALID_HANDLE_VALUE)
		{	return FALSE;		}

	//set startval for CTRLA Reg  +slope, IFC=h, VON=1 
		
	WriteByteS0(drvno,0x03, 0x04);  //write CTRLA reg in S0
	if (_COOLER) ActCooling(drvno,FALSE); //deactivate cooler

	
  /*sym - flag activates divider for symmetrical ND pulse
    max 1F * 66ns + 100ns  waits
    max 1F waits, no F_DP ND has 2.2 mu - with F_DP ND has 4.2 mu
    min 0waits, no F_DP ND has 100ns=10MHz - with F_DP has 170ns=5.8MHz
    SYM=false activates DMAModeRegs waitstategenerator
    0waits=66ns , max F waits =530ns
    double ND pulse on every 12bit read
    Bit7 = F_NDSYM ON, Bit6 = F_DP(Double pulse) ON, Bit5 = DIS_ND OFF*/
	
	if (sym) //compatible mode up to 10 MHz cams
		{
		data = 0x080; //sym pulse 
        if (waits>0x01F)  {data |= 0x1F; }
				else {data |=  (UCHAR)waits;}
		if (flag816==1)  {data |= 0x040; } //for 16Bit set F_DP
			   else {data &= 0x0BF; } //else clear F_DP
		WriteByteS0(drvno,data, 0x05);  //write CTRLB reg in S0
        //deact DMAMode asym waitstate generator

//		ReadLongIOPort(drvno,&reg,0x080);  // DMAMode Reg
//        reg &= 0x0ffffffC3; //clear waitstates
//        WriteLongIOPort(drvno,reg, 0x080);  // DMAMode Reg
		}
	   else   // highest speed cams 15MHz
	   {
        data=0;
		if (waits > 0x0F) { data = 0x0F;}  //max f waits
					else {data =  (UCHAR) waits;}
        data = data << 2;
  //      ReadLongIOPort(drvno,&reg,0x080);  // DMAMode Reg
  //      reg = reg & 0x0ffffffC3; //clear waitstates
  //      reg = reg | data;
  //      WriteLongIOPort(drvno,reg, 0x080);  // DMAMode Reg
        // deact SYM generator
		data = 0;
		if (flag816==1)  {data |= 0x040; } //for 16Bit set F_DP
        WriteByteS0(drvno,data, 0x05);  //write CTRLB reg in S0
	   };

/*
	if (burst)
		{//30 MHz cams can use dma burst mode
		ReadLongIOPort(drvno,&reg,0x080);  // DMAMode Reg
        reg = reg | 0x100;				//set burst bit
		reg = reg | 0x080;				//en bterm for highest speed
        WriteLongIOPort(drvno,reg, 0x080);  // DMAMode Reg

		// set burst flag for 33MHz ND-freq
		ReadByteS0(drvno,&data,0x06);
		data = data | 0x08;
		WriteByteS0(drvno,data, 0x06);  //write CTRLC reg in S0
		}
	  else
	  { // clear EN_BURST Flag
		ReadByteS0(drvno,&data,0x06);
		data = data & 0xf7;
		WriteByteS0(drvno,data, 0x06); 
	  };
*/

	  //set global vars if driver is there

	aWAITS[drvno] = waits;
	aFLAG816[drvno] = flag816;

	aPIXEL[drvno] = pixel;
	if (_HA_IR) 	aPIXEL[drvno] =  4 * pixel; //!! 2*
	if (_HA_IRSingleCH) 	aPIXEL[drvno] =  4 * pixel;
	aXCKDelay[drvno] = xckdelay;
	aINIT[drvno] = TRUE;


	//new for Fifo
#if (_FIFO)
	if (flag816 == 1) {	reg = 2*aPIXEL[drvno];}//16 bit
		else {reg = aPIXEL[drvno];};

	if(_FFPCI133==TRUE) 
		reg = aPIXEL[drvno];

	if (_OPTSTATE)//decrement pixel if shutterstate input with OPTO1 is impacted
		if (_FFPCI133) {		reg -= 1;}
		else reg -= 2;

    if (pclk>6) pclk=6;
	reg |= pclk << 16;
	if (xckdelay>7) xckdelay=7;
	if (xckdelay<1) xckdelay=1;
	reg |=  xckdelay << 19;	
	WriteLongS0(drvno,reg, 0x10); //set pixelreg	

#endif
	//new for new boards check for S0 length if DELAY reg is there

	WriteLongS0(drvno,DELAY, 0x34);	//set WRFFDELAY

	return TRUE; //no error
	};  // SetBoardVars




// ***************************  camera read stuff  **********
//*********************************************************

void Resort(UINT drvno, void* ptarget, void* psource)
//		example for resort array for 1 PCI board with 2 slots
//      array type long 8bit vals in +0 port1 and +1 in port2
//		resort for display -> port1 longs in DB1 and port2 longs in DB2 
{
		ULONG i=0;
		ULONG barraylength =0;
		ULONG larraylength =0;

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
	ArrayT* plsarray ;

	plsarray = (ArrayT *)psource;
	pltarray = (ArrayT *)ptarget;

//	if (_IR2) pltarray += 4*_PIXEL * sizeof(ArrayT);

	
#if (_HA_IRSingleCH) // one channel for HA G92xx with 256 pixel
	larraylength = _PIXEL;

	if (_FIFO)
		{
		if (_FFPCI133) //16 bit 1 channel cam
			{
			pltarray[0] = plsarray[1]& 0x8080;//show shutterstate in first pixel 
			for (i=1;i<larraylength;i++)
				pltarray[i] = plsarray[4*i+2];   // 0 in flanke, 1,2 signal, 3 null
				//pltarray[i] = plsarray[4*i+1] - plsarray[4*i+3] + 1000;  // 0 in flanke, 1,2 signal, 3 null
				//pltarray[i] = (plsarray[4*i+1] + plsarray[4*i+2])/2 - plsarray[4*i+3] + 1000;
			}
		else // 12bit IR cams 1 channel
			{
			for (i=0;i<larraylength;i++)
				pltarray[i] = plsarray[4*i+3];   // CH od 
			if(_HWCH2) // 2cams parallel -> append 2nd array
				for (i=0;i<larraylength;i++)
					pltarray[i+_PIXEL] = plsarray[_PIXEL*4+ 4*i+3];   
				//pltarray[i] = plsarray[4*i+2];  // CH ev
			}//else
		}//if (_FIFO)
	else // no FIFO
		for (i=0;i<larraylength;i++)
			{ 
			pltarray[i] = plsarray[4*i+2];   // CH od
			//pltarray[i] = plsarray[4*i+1];  // CH ev
			}
#else	// two channel sensors with 512 pixel

	larraylength = _PIXEL/2 ;

	if (_FIFO)
		for (i=0;i<larraylength;i++)
			{ //test:600 fifo
			pltarray[2*i+1] = plsarray[4*i+2]; //!!must change direction up/dn or modulation
			pltarray[2*i] = plsarray[4*i+3];

			if(_HWCH2) // 2cams parallel -> append 2nd array
				for (i=0;i<larraylength;i++)
					{
					pltarray[2*i+1+_PIXEL] = plsarray[_PIXEL*4+ 4*i+2];  
					pltarray[2*i+_PIXEL] = plsarray[_PIXEL*4+ 4*i+3];  
					}
			} 
	else
		for (i=0;i<larraylength;i++)	
			{ //test:600nofifo
			pltarray[2*i+1] = plsarray[4*i+1]; //!!must change direction up/dn or modulation
			pltarray[2*i] = plsarray[4*i+2];
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




//  call of the read function for no FIFO version, with FIFO use CallIORead
BOOL GETCCD(UINT drvno, void* pdioden, ULONG fftlines, long fkt, ULONG zadr)	
{	//starts data transfer and gets data in buffer dioden
	//drvno: driver number 1..4; 1 for LSCISA1
	//pdioden: pointer to destination array of ULONGs
	//fftlines: vertical lines of FFT sensors: vclks=2 * no of lines
	//fkt:  -1=clrread; 0=init dest array with 0; 1=standard read; 2=add to data
	//zadr: cam address on bus -> for series2000 addressed mode
	//returns true; false on error

  	BOOL fResult=FALSE;
	DWORD   ReturnedLength = 0;
	pArrayT pReadArray = NULL;
	pArrayT pCorArray = NULL;
	pArrayT	pDiodenBase = NULL;
	pArrayT	pDiodenBase2 = NULL;
	ULONG arraylength=0;
	sCCDFkts CCDfkt;
	ULONG i = 0;
	BOOL addalloc=FALSE;
	BOOL coralloc=FALSE;
	BYTE chl = 1;
	BYTE chh = 1;
		char pstring[22]="";

	if (! aINIT[drvno]) return FALSE;	// return with error if no init

	//set function recs
	CCDfkt.NoOfLines = 0; // NoOfLines = block in FIFO DMA on demand
	CCDfkt.ADFlag =10000; //delay before DMA Start (driver 2.x)	
	CCDfkt.Adrwaits = aXCKDelay[drvno]; // pass wait between XCK and ND
	if (_TI)
		{
		CCDfkt.Waits = 0;  // 6000 only valid for  TI sensors, time of vclks in 0 = 2mu
		CCDfkt.Vclks = fftlines * 4;
		}
	  else
		{// vclks for no FIFO 
		CCDfkt.Waits = VFREQ;  // 6000 only valid for  FFT sensors- no FIFO, time of vclks in ns = 6000
		CCDfkt.Vclks = fftlines * 2;
		}
	CCDfkt.Fkt	= fkt;
	CCDfkt.Zadr = zadr;

	pReadArray = (pArrayT)pdioden;
//	pReadArray = pReadArray + (db-1) * pixel;
	pDiodenBase = pReadArray;
	arraylength = aPIXEL[drvno] * sizeof(ArrayT); //length in bytes
	
	if (fkt==0) // set array to 0
		{
		for (i=0;i<	_PIXEL;i++)
			*(pReadArray++) = 0;
		return TRUE;
		}

	if  (fkt>2)
		return FALSE;  // function not implemented

	#if (_IR2)
		//alloc array for correction 4 x pixel
		//for IR - need 2. array
		pDiodenBase2 = (pArrayT)pdioden + aPIXEL[drvno];
		pCorArray = (pArrayT) calloc(aPIXEL[drvno]*4,sizeof(ArrayT));
		if (pCorArray==0) 		
			{ErrorMsg("alloc Cor Buffer failed");
			return FALSE; }
		coralloc=TRUE;
		pReadArray = pCorArray;
		arraylength *= 4;
		CCDfkt.Fkt = 1;		//make standard read
	#else


	if (_HA_IR)
			{//alloc local array because it is 2*PIXEL -> has to be resortet later
			pReadArray = (pArrayT) calloc(2*aPIXEL[drvno],sizeof(ArrayT)); 
			if (pReadArray==0) 		
				{ErrorMsg("alloc IR Buffer failed");
				return FALSE; }
			addalloc=TRUE;
			CCDfkt.Fkt = 1;		//make standard read and add later
			}

	else
		if (fkt!=1) //read in our local array ladioden - add and clrread
			{
			//alloc local array dioden, so we don't overwrite our DIODEN
			pReadArray = (pArrayT) calloc(aPIXEL[drvno],sizeof(ArrayT)); 
			if (pReadArray==0) 		
				{ErrorMsg("alloc ADD/CLR Buffer failed");
				return FALSE; }
			CCDfkt.Fkt = 1;		//make standard read and clr later
			addalloc=TRUE;
			}
	
	#endif
	
		
	//read camera data
	fResult = DeviceIoControl(ahCCDDRV[drvno],IOCTL_GetCCD,  
		 					&CCDfkt, sizeof(CCDfkt),
							pReadArray,arraylength,
							&ReturnedLength,NULL);

	if ((! fResult) || (ReturnedLength!=arraylength))
		{ErrorMsg("Read DMA Buffer failed");
		if (addalloc) free(pReadArray);
		if (coralloc) free(pCorArray);
		return FALSE; }

	//clrread and add: fkt=-1 and 2 could not be implemented with dma
	//so we do it here after reading
	if (fkt==-1)
	{ // return , nothing else to do
		if (addalloc) free(pReadArray);
		if (coralloc) free(pCorArray);
		return TRUE;
		}
 
	if (_RESORT) Resort(drvno,pReadArray,pReadArray);

	if ((_HA_IR) && (fkt!=2)) // copy back
			{
			pDiodenBase2=pReadArray;
			for (i=0;i<	_PIXEL;i++)
				* (pDiodenBase++) = * (pDiodenBase2++); 
			}

	if (fkt==2) // we must now add our data to DIODEN for online add
			{
			pDiodenBase2=pReadArray;
			for (i=0;i<	_PIXEL;i++)
				* (pDiodenBase++) += * (pDiodenBase2++); 
			}

	if (addalloc) free(pReadArray);
	if (coralloc) free(pCorArray);
	
#if _IS_C4350
//special for Ha Detector head C4350
	V_On(drvno);  //starts new read sequence to Fifo
	V_Off(drvno);
//special end
#endif

	return TRUE; // no Error, all went fine
	};  // GETCCD





BOOL CallIORead(UINT drvno, void* pdioden)
{	//here only the standard read fkt=1 is implemented
	//FIFO version -> = ReadFifo with resort & for double line cams

  	BOOL fResult=FALSE;
	DWORD   ReturnedLength = 0;
	sCCDFkts CCDfkt;
	BOOL b12alloc=FALSE;
	DWORD arraylength = aPIXEL[drvno] * sizeof(ArrayT); //length in bytes
	PUCHAR pRArray = (UCHAR*) pdioden;
	ULONG pixel = aPIXEL[drvno];
	BYTE a=1;
	BYTE b=0;

	//set function recs - here all have to be 0 - as we read the Fifo
	CCDfkt.NoOfLines = 0; // NoOfLines = block in FIFO DMA on demand
	CCDfkt.ADFlag =0;	//dsdelay = delay before DMA start
	CCDfkt.Adrwaits = 0; // adrwaits -> 3mu
	CCDfkt.Waits = 0;  // Waits between vclks 6mu
	CCDfkt.Vclks = 0;   
	CCDfkt.Fkt	= 1;
	CCDfkt.Zadr = 0;

	if (aFLAG816[drvno] ==2) // 8 bit allways byte array of words
			{
			arraylength =aPIXEL[drvno];
			//alloc local array dioden, so we don't overwrite our DIODEN
			pRArray = (BYTE*) calloc(arraylength,sizeof(BYTE)); 
			if (pRArray==0) 		
				{ErrorMsg("alloc 12bit Buffer failed");
				return FALSE; }
			b12alloc=TRUE;
			}


	if (aFLAG816[drvno] ==1) // 12 bit allways byte array of words
			{
			arraylength = 2*aPIXEL[drvno]; //2bytes per word
			if (_HWCH2) arraylength *= 2; //double if 2 cams 
			//alloc local array dioden, so we don't overwrite our DIODEN
			//2.200 pRArray = (BYTE*) calloc(aPIXEL[drvno],4*sizeof(BYTE)); 
			pRArray = (BYTE*) calloc(arraylength,sizeof(BYTE)); 
			if (pRArray==0) 		
				{ErrorMsg("alloc 12bit Buffer failed");
				return FALSE; }
			b12alloc=TRUE;
			}

	//read camera data
	fResult = DeviceIoControl(ahCCDDRV[drvno],IOCTL_GetCCD,  
							&CCDfkt, sizeof(CCDfkt),
							pRArray,arraylength,
							&ReturnedLength,NULL);
	if ((! fResult) || (ReturnedLength!=arraylength))
		return FALSE;

//	ValMsg(arraylength);

	if (aFLAG816[drvno] ==2) 
	{	// resort 8 bit array -> BYTE sort to data array 8,16 or 32
		ULONG i=0;
		BYTE* ptarray;
		BYTE* psarray ;
		psarray = (UCHAR*) pRArray;  //(BYTE*) calloc(PIXEL*4, sizeof(BYTE));
		ptarray = (UCHAR*) pdioden;    

		if (sizeof(ArrayT)==1) {return FALSE;}; //8 bit no sort

		if (sizeof(ArrayT)==4) //target 32 bit
		{//BYTE resort
		for (i=0;i<aPIXEL[drvno];i++) //_PIXEL
			{//standard resort for FIFO 1 channel
			 ptarray[i*4] =  psarray[i]; // target1 lo byte
			 ptarray[i*4+1] = 0;
			 ptarray[i*4+2] =  0;
			 ptarray[i*4+3] =  0;
			}		
		}; //32bit

		if (sizeof(ArrayT)==2) //target 16 bit
		{
		for (i=0;i<aPIXEL[drvno];i++)
			{
			 ptarray[i*2] =  psarray[i];	// lo byte
			 ptarray[i*2+1] =  0;
			}
		}; //16 bit
	}// if AD=8bit


	if (aFLAG816[drvno] ==1)
	{	// resort 12/16 bit array -> BYTE sort of array data
		ULONG i=0;
		ULONG pixel=aPIXEL[drvno];
		BYTE* ptarray;
		BYTE* psarray ;
		psarray = (UCHAR*) pRArray;  //(BYTE*) calloc(PIXEL*4, sizeof(BYTE));
		ptarray = (UCHAR*) pdioden;    

		if (sizeof(ArrayT)==1) {return FALSE;}; //8 bit not for 12bit

		if (sizeof(ArrayT)==4) //target 32 bit
		{//BYTE resort
		if ( _HWCH2) //2 cams
		{
			for (i=0;i<pixel;i++) //_PIXEL
				{
				//resort for 16bit 2 channel 2word->1long cha/chb  
					//2.227 hi/lo changed -> //!!not tested 
					if (_HILO) {a=2; b=0;} else {a=0; b=2;};
					ptarray[i*4] =   psarray[4*i+b];   // target1 lo 
					ptarray[i*4+1] = psarray[4*i+a]; // target1 hi  
	
					ptarray[i*4+2] = psarray[4*i+b+1]; // target2 lo	
					ptarray[i*4+3] = psarray[4*i+a+1]; //	target2 hi  

		//			ptarray[i*8] =   //psarray[8*i+4]; // war +4 target1 lo byte
		//			ptarray[i*8+1] = 0;//psarray[8*i+2]; //psarray[8*i-2];   // war +2 target2 hi byte
		//			ptarray[i*8-4] = 0;//psarray[8*i];//psarray[8*i+4]; // target1 lo byte war -4
		//			ptarray[i*8-3] = 0;//psarray[8*i-2];//psarray[8*i+2];   // t1 target1 hi byte				
				}
		}
		else
			for (i=0;i<pixel;i++) //_PIXEL
				{//standard resort for FIFO 1 channel word->long
			//2.227 hi/lo changed -> for FFT and PDA
				if (_HILO) {a=0; b=1;}
				ptarray[i*4] =  psarray[2*i+a]; // target1 lo byte
				ptarray[i*4+1] = psarray[2*i+b];    // b target1 hi byte
				ptarray[i*4+2] =  0;
				ptarray[i*4+3] =  0;
				}
		}; //32 bit

		//resort 12 bit array takes 4ns / pixel on a 3GHz PC
		if (sizeof(ArrayT)==2) //target 16 bit array
		{
		if ( _HWCH2) //2 cams
			{for (i=0;i<pixel-1;i++)
				{//here the 2cam 12bit is appended
				//append 2nd cam first
				if (_HILO) {a=2; b=0;} else {a=0; b=2;};
				ptarray[2*pixel+i*2+1] = psarray[i*4+a+1];	// hi byte cam2 
				ptarray[2*pixel+i*2] = psarray[i*4+b+1];	// lo byte cam2

				ptarray[i*2+1] = psarray[i*4+a];	// hi byte cam1
				ptarray[i*2] =  psarray[i*4+b];	// lo byte cam1

				} //for
			}
		else // only one camera
			{for (i=0;i<pixel;i++)
				{//change lo-hi
				if (_HILO) {a=0; b=1;}
				ptarray[i*2] =  psarray[i*2+a];	// lo byte
				ptarray[i*2+1] = psarray[i*2+b];	// hi byte
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

	ULONG length=0;
	ULONG i = 0;
	BOOL addalloc=FALSE;

	if (! aINIT[drvno]) return FALSE;	// return with error if no init

	pReadArray = (pArrayT) pdioden;
//	pReadArray = pReadArray + (db-1) * pixel;
	pDiodenBase = pReadArray;

	if (fkt==0) // set array to 0
		{
		for (i=0;i<	_PIXEL;i++)
			*(pReadArray++) = 0;
		return TRUE;
		}

	if (fkt==5) // set array to val
		{
		for (i=0;i<	_PIXEL;i++)
			*(pReadArray++) =  (ArrayT) i*50;
		return TRUE;
		}

	if  (fkt>2)
		return FALSE;  // function not implemented

	//if ((_IR) && (!addalloc))
	if (_HA_IR)
			{
			//alloc local array because it is 2*PIXEL -> has to be resortet later
			pReadArray = (pArrayT) calloc(2*aPIXEL[drvno],sizeof(ArrayT)); 
			if (pReadArray==0) 		
				{ErrorMsg("alloc IR Buffer failed");
				return FALSE; }
			addalloc=TRUE;
			}

	else
		if (fkt!=1) //read in our local array ladioden - add and clrread
			{
			//alloc local array dioden, so we don't overwrite our DIODEN
			pReadArray = (pArrayT) calloc(aPIXEL[drvno],sizeof(ArrayT)); 
			if (pReadArray==0) 		
				{ErrorMsg("alloc ADD/CLR Buffer failed");
				return FALSE; }
			addalloc=TRUE;
			}


	//call the read
	if (! CallIORead(drvno,pReadArray))
		{ErrorMsg("Read DMA Buffer - FIFO failed");
		if (addalloc) free(pReadArray);
		return FALSE; }


	if (fkt==-1)
	{ // return , nothing else to do
		if (addalloc) free(pReadArray);
		return TRUE;
		}

	if (_RESORT) Resort(drvno,pReadArray,pReadArray);  //pixel resort
	
	if ((_HA_IR) && (fkt!=2)) // copy back
			{
			pDiodenBase2=pReadArray;
			length=_PIXEL;
			if(_HWCH2) length=_PIXEL*2;
			for (i=0;i<	length;i++)
				* (pDiodenBase++) = * (pDiodenBase2++); 
			}


	//clrread and add fkt=-1 and 2 could not be implemented with dma
	//so we do it here after reading
	if (fkt==2) // we must now add our data to DIODEN for online add
			{
			pDiodenBase2=pReadArray;
			for (i=0;i<_PIXEL;i++)
				* (pDiodenBase++) += * (pDiodenBase2++); 
			}

	if (addalloc) free(pReadArray);

	return TRUE; // no Error, all went fine
	};  // ReadFifo






// *********************** PCI board registers



BOOL ReadLongIOPort(UINT drvno,ULONG *DWData,ULONG PortOff)
	{// reads long of PCIruntime register LCR
	// PortOff: Reg Offset from BaseAdress - in bytes
	// on return -> TRUE if success
	BOOL fResult = FALSE;
	DWORD   ReturnedLength;
	ULONG	PortOffset;

	PortOffset = PortOff;
	fResult = DeviceIoControl(ahCCDDRV[drvno],IOCTL_ReadLongIORunReg,  
							&PortOffset,        
                            sizeof(PortOffset),
							DWData,sizeof(ULONG),&ReturnedLength,NULL);
	if (! fResult)
		{ErrorMsg("Read IORunReg failed"); return FALSE;};
	return TRUE;
	};  // ReadLongIOPort


BOOL ReadLongS0(UINT drvno,ULONG *DWData, ULONG PortOff)
	{// reads long on space0 area
	// PortOff: Offset from BaseAdress - in Bytes !
	// return -> TRUE if success
	BOOL fResult = FALSE;
	DWORD   ReturnedLength;
	ULONG	PortOffset;

	//space0 starts at S0-Offset=0x80 in BAR0
	PortOffset = (PortOff ) ;
	fResult = DeviceIoControl(ahCCDDRV[drvno],IOCTL_ReadLongS0,  // read one byte
							&PortOffset,        // offset in bytes
                            sizeof(PortOffset),
							DWData,sizeof(ULONG),&ReturnedLength,NULL);
	if (! fResult)
		{ErrorMsg("Read long in space0 failed");  return FALSE;};
	return TRUE;
	};  // ReadLongS0

BOOL ReadLongDMA(UINT drvno,ULONG *DWData, ULONG PortOff)
	{// reads long on DMA area
	// PortOff: Offset from BaseAdress - in Bytes !
	// return -> TRUE if success
	BOOL fResult = FALSE;
	DWORD   ReturnedLength;
	ULONG	PortOffset;

	PortOffset = PortOff ;
	fResult = DeviceIoControl(ahCCDDRV[drvno], IOCTL_ReadLongDMAReg,  // read one byte
							&PortOffset,        // offset in bytes
                            sizeof(PortOffset),
							DWData,sizeof(ULONG),&ReturnedLength,NULL);
	if (! fResult)
		{ErrorMsg("Read long in DMA failed");  return FALSE;};
	return TRUE;
	};  // ReadLongDMA
	
BOOL ReadByteS0(UINT drvno,UCHAR *data, ULONG PortOff)
	{// reads byte in space0 area
	// PortOff: Offset from BaseAdress - in Bytes !
	// returns TRUE if success
	BOOL fResult = FALSE;
	DWORD   ReturnedLength;
	ULONG	PortOffset;

	//space0 starts at S0-Offset in BAR0
	PortOffset = PortOff ;
	fResult = DeviceIoControl(ahCCDDRV[drvno],IOCTL_ReadByteS0,  // read one byte
							&PortOffset,        // Buffer to driver.
                            sizeof(PortOffset),
							data,sizeof(UCHAR),&ReturnedLength,NULL);
	if (! fResult)
		{ ErrorMsg("Read byte in space0 failed");
		return FALSE;};
	return TRUE;
	};  // ReadByteS0


BOOL WriteLongIOPort(UINT drvno,ULONG DWData, ULONG PortOff)
	{	// writes long to PCIruntime register
		// PortOff: Reg Offset from BaseAdress - in bytes
		// returns TRUE if success
	BOOL fResult = FALSE;
	sDLDATA WriteData;
	ULONG	DataLength;
	DWORD   ReturnedLength;

	WriteData.POff	= PortOff;
	WriteData.Data	= DWData;
	DataLength		= 8; 
	fResult = DeviceIoControl(ahCCDDRV[drvno],IOCTL_WriteLongIORunReg, 
							&WriteData,      
                            DataLength,
							NULL,0,&ReturnedLength,NULL);
	if (! fResult)
		{ErrorMsg("WriteLongIOPort failed");
		return FALSE;}
	return TRUE;
	};  // WriteLongIOPort


BOOL WriteLongS0(UINT drvno,ULONG DWData, ULONG PortOff)
	{	// writes long to space0 register
		// PortOff: Reg Offset from BaseAdress - in bytes
		// returns TRUE if success
	BOOL fResult = FALSE;
	sDLDATA WriteData;
	ULONG	DataLength;
	DWORD   ReturnedLength;

	//space0 starts at S0-Offset in BAR0
	WriteData.POff= PortOff ; // offset in bytes
	WriteData.Data = DWData;
	DataLength = 8; 
	fResult = DeviceIoControl(ahCCDDRV[drvno],IOCTL_WriteLongS0,  // port offset in longs
							&WriteData,      
                            DataLength,
							NULL,0,&ReturnedLength,NULL);
	if (! fResult)
		{ErrorMsg("WriteLongS0 failed");
			return FALSE;}
	return TRUE;
	};  // WriteLongS0

BOOL WriteLongDMA(UINT drvno,ULONG DWData, ULONG PortOff)
	{	// writes long to space0 register
		// PortOff: Reg Offset from BaseAdress - in bytes
		// returns TRUE if success
	BOOL fResult = FALSE;
	sDLDATA WriteData;
	ULONG	DataLength;
	DWORD   ReturnedLength;

	WriteData.POff= PortOff ; // offset in bytes
	WriteData.Data = DWData;
	DataLength = 8; 
	fResult = DeviceIoControl(ahCCDDRV[drvno], IOCTL_WriteLongDMAReg,  // port offset in longs
							&WriteData,      
                            DataLength,
							NULL,0,&ReturnedLength,NULL);
	if (! fResult)
		{ErrorMsg("WriteLongDMA failed");
			return FALSE;}
	return TRUE;
	};  // WriteLongDMA

BOOL WriteByteS0(UINT drvno,BYTE DWData, ULONG PortOff)
	{	// writes byte to space0 register
		// PortOff: Reg Offset from BaseAdress - in bytes
		// returns TRUE if success
	BOOL fResult = FALSE;
	sDLDATA WriteData;
	ULONG	DataLength;
	DWORD   ReturnedLength;

	
	//space0 starts at S0-Offset in BAR0
	WriteData.POff= PortOff ;
	WriteData.Data = DWData;
	DataLength = sizeof(WriteData); 
	fResult = DeviceIoControl(ahCCDDRV[drvno],IOCTL_WriteByteS0,  // 
							&WriteData,      
                            DataLength,
							NULL,0,&ReturnedLength,NULL);
	if (! fResult)
		{ErrorMsg("WriteByteS0 failed");
			return FALSE;}
	return TRUE;
	};  // WriteByteS0

BOOL WriteBytePort0(UINT drvno,BYTE DWData, ULONG PortOff)
	{	// writes byte to subd with nd clks if VON=hi / adr if VON=lo data
		// PortOff: adr data
		// returns TRUE if success
	BOOL fResult = FALSE;
	sDLDATA WriteData;
	ULONG	DataLength;
	DWORD   ReturnedLength;

	WriteData.POff= PortOff &0xff;
	WriteData.Data = DWData;
	DataLength = sizeof(WriteData); 
	fResult = DeviceIoControl(ahCCDDRV[drvno],IOCTL_WriteBytePort0,  // 
							&WriteData,      
                            DataLength,
							NULL,0,&ReturnedLength,NULL);
	if (! fResult)
		{ErrorMsg("WriteBytePort0 failed");
			return FALSE;}
	return TRUE;
	};  // WriteBytePort0

void AboutDrv(UINT drvno)
{	USHORT version = 0;
	ULONG S0Data = 0;
	UCHAR udata1,udata2,udata3,udata4 =0;
	BOOL fResult = FALSE;
	ULONG PortNumber = 0;		// must be 0
	DWORD   ReturnedLength=0;  // Number of bytes returned
	char pstring[80]="";
	char wstring[16]="";
	char astring[3]="";
	HWND hWnd = GetActiveWindow();
	HDC aDC = GetDC(hWnd); 

	fResult = DeviceIoControl(ahCCDDRV[drvno],IOCTL_GetVersion,
					&PortNumber,        // Buffer to driver.
                    4,&version,sizeof(version),&ReturnedLength,NULL);
	if (fResult)
		{ // read driver version via DevIoCtl
		sprintf_s(wstring,17,"Driver LSCPCI%d",drvno);
		sprintf_s(pstring,81," version: 0x%x", version);
		if (MessageBox( hWnd, pstring, wstring,MB_OK|MB_ICONEXCLAMATION ) == IDOK ) {};
		}
	else
		{ErrorMsg("About DeviceIo failed");
		return;
		};

	// read ISA Id from S0Base+7
	ReadLongS0(drvno,&S0Data,4); // Board ID =5053
	S0Data = S0Data>>16;

	//or
	//S0Data = (UCHAR)ReadByteS0(8); // ID=53
	sprintf_s(pstring,80,"     ID = 0x%x", S0Data);
	if (MessageBox( hWnd, pstring," Board ID=53 ",MB_OK|MB_ICONEXCLAMATION ) == IDOK ) {};

	ReadLongIOPort(drvno,&S0Data,0); //read LCR0 for check length 0xffffffco
	S0Data 	=  ~S0Data; //length is inverted

	sprintf_s(pstring,80,"     length = 0x%x", S0Data);
	if (MessageBox( hWnd, pstring," PCI space0 length=",MB_OK|MB_ICONEXCLAMATION ) == IDOK ) {};

	if (S0Data>=0x1F)
		{//if WE -> has space 0x20
		ReadByteS0(drvno,&udata1,0x1C);
		ReadByteS0(drvno,&udata2,0x1D);
		ReadByteS0(drvno,&udata3,0x1E);
		ReadByteS0(drvno,&udata4,0x1F);
		sprintf_s(pstring,80," ven ID = %c%c%c%c", udata1,udata2,udata3,udata4);
		if (MessageBox( hWnd, pstring," Board vendor=EBST ",MB_OK|MB_ICONEXCLAMATION ) == IDOK ) {};
		}

	if (S0Data>=0x3F)
		{//if 9056 -> has space 0x40
		ReadLongS0(drvno,&S0Data,0x3C);
		sprintf_s(pstring,80," board version = 0x%x", S0Data);
		if (MessageBox( hWnd, pstring," Board version ",MB_OK|MB_ICONEXCLAMATION ) == IDOK ) {};
		}

	ReleaseDC(hWnd,aDC);
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
{// set bit D7
	BYTE IRQCNTHH;

	HighSlope(drvno);
	ReadByteS0(drvno, &IRQCNTHH, 0x3B);
	IRQCNTHH |= 0x80;
	WriteByteS0(drvno, IRQCNTHH, 0x3B);
}; //BothSlope

void NotBothSlope(UINT drvno)
{// set bit D7
	BYTE IRQCNTHH;
	ReadByteS0(drvno, &IRQCNTHH, 0x3B);
	IRQCNTHH &= 0x7F;
	WriteByteS0(drvno, IRQCNTHH, 0x3B);
}; //NotBothSlope

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Ausgabe eines High-Signals an Pin 17                                      */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void OutTrigLow(UINT drvno)
	{
	BYTE CtrlA;
	ReadByteS0(drvno,&CtrlA,4);
	CtrlA &= 0xf7;
	WriteByteS0(drvno,CtrlA,4);
	};						//OutTrigLow

/*---------------------------------------------------------------------------*/


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Ausgabe eines Low-Signals an Pin 17                                       */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void OutTrigHigh(UINT drvno)
	{
	BYTE CtrlA;
	ReadByteS0(drvno,&CtrlA,4);
	CtrlA |= 0x08;
	WriteByteS0(drvno,CtrlA,4);
	}; //OutTrigHigh


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Ausgabe eines PulseWidth breiten Rechteckpulses an Pin 17                 */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void OutTrigPulse (UINT drvno,ULONG PulseWidth)
  {
	OutTrigHigh(drvno);
	Sleep (PulseWidth);
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

void WaitTrigger(UINT drvno,BOOL ExtTrigFlag,BOOL *SpaceKey, BOOL *AbrKey)
	// returns if Trigger or Key
	{ 
	BOOL FirstLo = FALSE;
	BOOL HiEdge = FALSE;
	BOOL Abbr = FALSE;
	BOOL Space = FALSE;
	UCHAR ReturnKey =0;
	BYTE ReadTrigPin = 0;

	do 
		{
		if (ExtTrigFlag)
			{ 
			ReadByteS0(drvno,&ReadTrigPin,4);
			ReadTrigPin &= 0x040;
			if (ReadTrigPin == 0) FirstLo = TRUE; //first look for lo
			if (FirstLo) {if (ReadTrigPin > 0) HiEdge = TRUE;}; // then look for hi
			}
			else HiEdge = TRUE;

#if _PS2KEYBOARD  //with PS2 keyboard
		ReturnKey = ReadKeyPort(drvno);
		if (ReturnKey==_ScanCode_Cancel) Abbr = TRUE;
		if (ReturnKey==_ScanCode_End) Space = TRUE;
#else	//other keyboard -> do not use highest priority thread
		// or use Sleep to serve the interrupt
		if (GetAsyncKeyState(VK_ESCAPE))  Abbr = TRUE;
		if (GetAsyncKeyState(VK_SPACE))  Space = TRUE;
#endif
		}
	while ((!HiEdge) && (! Abbr));
	if (Abbr) *AbrKey = TRUE;	//stops immediately
	if (Space) *SpaceKey = TRUE;	//stops after next trigger
  };// WaitTrigger


//******************** the triginput has an optional FF to detect short pulses
// the FF is edge triggered and must be reset via RSTrigShort after each pulse to arm it again
// it is enabled once by EnTrigShort()
//********************

void WaitTriggerShort(UINT drvno,BOOL ExtTrigFlag,BOOL *SpaceKey, BOOL *AbrKey)
	// returns if Trigger or Key
	{ 
	BOOL FirstLo = FALSE;
	BOOL HiEdge = FALSE;
	BOOL Abbr = FALSE;
	BOOL Space = FALSE;
	UCHAR ReturnKey =0;
	BYTE ReadTrigPin = 0;

	do 
		{
		if (ExtTrigFlag)
			{ 
			ReadByteS0(drvno,&ReadTrigPin,4);
			ReadTrigPin &= 0x040;
			if (ReadTrigPin > 0) HiEdge = TRUE; 
			}
			else HiEdge = TRUE;

#if _PS2KEYBOARD  //with PS2 keyboard
		ReturnKey = ReadKeyPort(drvno);
		if (ReturnKey==_ScanCode_Cancel) Abbr = TRUE;
		if (ReturnKey==_ScanCode_End) Space = TRUE;
#else	//other keyboard -> do not use highest priority thread
		// or use Sleep to serve the interrupt
		if (GetAsyncKeyState(VK_ESCAPE))  Abbr = TRUE;
		if (GetAsyncKeyState(VK_SPACE))  Space = TRUE;
#endif
		}
	while ((!HiEdge) && (! Abbr));
	if (Abbr) *AbrKey = TRUE;	//stops immediately
	if (Space) *SpaceKey = TRUE;	//stops after next trigger
	RSTrigShort(drvno);//!
  };// WaitTrigger^Short


void EnTrigShort(UINT drvno)
	{//use the short trig pulse FF for ext TrigIn
	UCHAR CtrlA;
	ReadByteS0(drvno,&CtrlA,4);
	CtrlA |= 0x080;	// set trigger path to FF
	CtrlA |= 0x010; // enable FF
	WriteByteS0(drvno,CtrlA ,4);
	}; //EnTrigShort

void RSTrigShort(UINT drvno)
	{//reset the short trig pulse FF 
	UCHAR CtrlA;
	ReadByteS0(drvno,&CtrlA,4);
	CtrlA &= 0x0EF; // write CLR to FF
	WriteByteS0(drvno,CtrlA ,4);
	CtrlA |= 0x010; // arm FF again
	WriteByteS0(drvno,CtrlA ,4);
	}; //RSTrigShort

void DisTrigShort(UINT drvno)
	{//use the direct input for ext TrigIn
	UCHAR CtrlA;
	ReadByteS0(drvno,&CtrlA,4);
	CtrlA &= 0x07F;	// set trigger path to FF
	CtrlA &= 0x0EF; // clr  FF
	WriteByteS0(drvno,CtrlA ,4);
	}; //SetTrigShort


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

  void CloseShutter(UINT drvno)   // IFC = low
	{
	UCHAR CtrlA;
	ReadByteS0(drvno,&CtrlA,4);
	CtrlA &= 0x0fd;	/* $FD = 1111 1101 */
	WriteByteS0(drvno,CtrlA ,4);
	}; //CloseShutter

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

  void OpenShutter(UINT drvno)   // IFC = high
	{
	UCHAR CtrlA;
	ReadByteS0(drvno,&CtrlA,4);
	CtrlA |= 0x02 ;	
	WriteByteS0(drvno,CtrlA,4);
	}; //OpenShutter


   BOOL GetShutterState(UINT drvno)
	{
	UCHAR CtrlA;
	ReadByteS0(drvno,&CtrlA,4);
	CtrlA &= 0x02 ;	
	if (CtrlA==0) return FALSE;
	return TRUE;
	}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* set V_ON low (V = V_Fak)                                               */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

  void V_On(UINT drvno)
	{
	UCHAR CtrlA;
	ReadByteS0(drvno,&CtrlA,4);
	CtrlA |= 0x01;
	WriteByteS0(drvno,CtrlA ,4);
	}; //V_On

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* set V_ON high (V = 1)                                                  */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

  void V_Off(UINT drvno)
	{
	UCHAR CtrlA;
	ReadByteS0(drvno,&CtrlA,4);
	CtrlA &= 0xfe;	// $FE = 1111 1110 
	WriteByteS0(drvno,CtrlA,4);
	}; //V_Off


// optional Opto Couplers
  void SetOpto(UINT drvno,BYTE ch)
	{//sets signal=low
	BYTE ctrlc;
	ReadByteS0(drvno,&ctrlc,6); 
	if (ch==2) {ctrlc |= 0x04;}
	else ctrlc |= 0x02;
	WriteByteS0(drvno,ctrlc,6);
	}; //SetOpto


  void RsetOpto(UINT drvno,BYTE ch)
  { //sets signal=high
	BYTE ctrlc;
	ReadByteS0(drvno,&ctrlc,6); 
	if (ch==2) {ctrlc &= 0xfb;}
	else ctrlc &= 0xfd;
	WriteByteS0(drvno,ctrlc,6);
	}; //RsetOpto


  BOOL GetOpto(UINT drvno,BYTE ch)
	{//no input or low -> high / high input -> low 
	BYTE ctrlc;
	ReadByteS0(drvno,&ctrlc,6); 
	if (ch==2) {ctrlc &= 0x04;}
	else ctrlc &= 0x02;
	if (ctrlc>0) return TRUE;
	return FALSE;
	}; //GetOpto

void SetDAT(UINT drvno,ULONG datin100ns)
	{//delay after trigger HW register
	datin100ns |= 0x80000000; // enable delay
	WriteLongS0(drvno,datin100ns,0x20);
	}; //SetDAT

void RSDAT(UINT drvno)
	{//delay after trigger HW register
	WriteLongS0(drvno,0,0x20);
	}; //RSDAT


void SetTORReg(UINT drvno, BYTE fkt)
{BYTE val=0; //defaut XCK= high during read
BYTE val2=0;
if (fkt==1) val = 0x80; // set to REG -> OutTrig
if (fkt==2) val = 0x10; // set to FFRead
if (fkt==3) val = 0x02; // set to IRArea read (VON)
if (fkt==4) val = 0x40; //set to EC
if (fkt==5) val = 0x08; //set to DAT

ReadByteS0(drvno,&val2,0x2B);
val2 &= 0x03; //dont disturb lower bits
val |= val2;
WriteByteS0(drvno,val,0x2B);
}//SetTORReg

void SetISPDA(UINT32 drvno, BOOL set)
{//set bit if PDA sensor - used for EC and IFC
BYTE val=0;
ReadByteS0(drvno,&val,0x2B);
if (set!=0) { val |= 0x02;}
else val &= 0xfd;
WriteByteS0(drvno,val,0x2B);
}//SetISPDA

void SetISFFT(UINT32 drvno, BOOL set)
{//set bit if FFT sensor - used for vclks and IFC
BYTE val=0;
ReadByteS0(drvno,&val,0x2B);
if (set!=0) { val |= 0x01;}
else val &= 0xfe;
WriteByteS0(drvno,val,0x2B);
}//SetISFFT

void RsTOREG(UINT32 drvno)
{// reset TOREG
WriteByteS0(drvno,0,0x2B);
}




#if (_FIFO)
void ClrRead(UINT drvno, UINT fftlines, UINT zadr, UINT ccdclrcount) 
	  //normal clear for Kamera is a complete read out
	  //most cams needs up to 10 complete reads for resetting the sensor
	  //depends how much it was overexposured
	{
	UINT i;
	SetIntFFTrig(drvno);
//	RSFifo(drvno);
	for (i=0;i<ccdclrcount;i++) 
			{
			SWTrig(drvno);
			while (FlagXCKI(drvno))
			{}//wait until its ready
			}
	StopFFTimer(drvno);
	RSFifo(drvno);
	}; //ClrRead

#else
  void ClrRead(UINT drvno, UINT fftlines, UINT zadr, UINT ccdclrcount) 
	  //normal clear for Kamera is a complete read out
	  //most cams needs up to 10 complete reads for resetting the sensor
	  //depends how much it was overexposured
	{
	pArrayT dummy=NULL;
	UINT i;
    for (i=0;i<ccdclrcount;i++) 
				{
				//OutTrigHigh(drvno);
				GETCCD(drvno,dummy,fftlines,-1,zadr); 
				//OutTrigLow(drvno);
				};
	}; //ClrRead
#endif




void ClrShCam(UINT drvno, UINT zadr) //clear for Shutter cameras
	{
	pArrayT dummy = NULL;
	CloseShutter(drvno);              //IFC=low
	Sleep(5);
	GETCCD(drvno,dummy,0,-1,zadr);
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

	fResult = DeviceIoControl(ahCCDDRV[drvno],IOCTL_ReadKey,  // read one byte
							&PortOffset,        // Buffer to driver.
                            sizeof(PortOffset),
							&Data,sizeof(Data),&ReturnedLength,NULL);
	if (! fResult)
		{ErrorMsg("Read Key Ioctl failed");
		exit(0); };
	return Data;
	};  // ReadKeyPort


void CAL_AD(UINT drvno, UINT zadr)
//for ADC16061
{// calibrate 16 bit A/D converter
	//takes 272800 ND cycles

	//BOOL _OLDCAL = FALSE;

	if (_OLDCAL)
	// old version
	{
	pArrayT pReadArray = (pArrayT) calloc(aPIXEL[drvno]*4,sizeof(ArrayT)); 
	zadr = zadr|0x80;
	GETCCD(drvno,pReadArray,0,1,zadr);
	if (_FIFO) SWTrig(drvno);
	free(pReadArray);
	if (_FIFO) StopFFTimer(drvno);
	}
	else
	//new cal with send AA to D4
	{
	CloseShutter(drvno);// IFC=lo
	Sleep(1);
	// old version ADC16061 with D4
	//SendCommand(drvno,0xD4,0xAA);
	//new version for ADS850 with F6
	SendCommand(drvno,0xF6,0xAA);	
	Sleep(1);
	SendCommand(drvno,0xD4,0x00); //reset CAL
	OpenShutter(drvno);// IFC=hi
	}
Sleep(6);//wait 6ms 
}//CAL16Bit

void SetOvsmpl(UINT drvno, UINT zadr)
//for ADC16061
{// set Oversample / reset with cal16bit
	 // by calling GETCCD with adr 0x10 set
	pArrayT pReadArray = (pArrayT) calloc(aPIXEL[drvno]*4,sizeof(ArrayT)); 
	zadr = zadr|0x10;
	GETCCD(drvno,pReadArray,0,1,zadr);
	free(pReadArray);
}//SetOvsmpl


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
ULONG reg=0;
ReadLongIOPort(drvno,&reg,0); //read LCR0 for check length 0xffffffco
reg	=  ~reg; //length is inverted
if (reg>0x34) 
	WriteLongS0(drvno,0,0x24);	// EC=0
}


void SendCommand(UINT drvno, BYTE adr, BYTE data)
//for programming of seriell port for AD98xx
{//before calling IFC has to be set to low
//and EC must be set 0 with 	RSEC(drvno);
	BYTE regorg=0;
	BYTE regh=0;
	BYTE regl=0;
	BYTE regno=0;
//	Sleep(1);
	WriteByteS0(drvno,adr,0); // write address to bus

	if (_PRGRMVON)
		{//clk with VON
		regno=4;
		ReadByteS0(drvno,&regorg,regno);// get reg data
		regh = regorg | 0x01; // VON
		regl = regorg & 0xFE;
		}
	else
		{//clk with ND
		regno=5;
		ReadByteS0(drvno,&regorg,regno);// get reg data
		regh =	regorg & 0xDF;			//ND_DIS clk is inverted
		regl =	regorg | 0x20; 
		}

	WriteByteS0(drvno,regh,regno); // // ND/VON hi  setup

	WriteByteS0(drvno,regl,regno); // // ND/VON lo 
	WriteByteS0(drvno,regh,regno); // // ND/VON hi

	WriteByteS0(drvno,data,0);	// write data to bus

	WriteByteS0(drvno,regl,regno); // // ND/VON lo 
	WriteByteS0(drvno,regh,regno); // // ND/VON hi


	WriteByteS0(drvno,regorg,regno); // ND/VON restore
	WriteByteS0(drvno,0,0);	// write 0 to bus

}//SendCommand 




void SetHiamp(UINT drvno, BOOL hiamp)
{
	if (_TI)
		{
		CloseShutter(drvno);// IFC=lo
		RSEC(drvno);
		Sleep(1);
		if  (hiamp)
		{ SendCommand( drvno, 0x99, 1); } //1
		else 
		{ SendCommand( drvno, 0x99, 1); } //0
		Sleep(1);
		OpenShutter(drvno);// IFC=hi
		return;
		}
//	if (_HA_IR) CloseShutter(drvno);// IR uses #11 or #14
	if (hiamp) {V_On(drvno);}	//standard use #11 VON
	else {V_Off(drvno);}
}//SetHiamp


void SetAD(UINT drvno, BYTE adadr, BYTE addata)
//for AD98xx
{	BYTE reg=0;
	CloseShutter(drvno);// IFC=lo
	RSEC(drvno);
	Sleep(1);	
	reg = 0;

#define _OLDSEND FALSE // TRUE for old send mechanism PAL 94.1

	#if (!_OLDSEND)
	reg = adadr<<4;
	if ((adadr&0x80)==0x80) //neg
		reg |=0x01; //D8
	#else
	reg = adadr;
	if ((adadr&0x80)==0x80) //neg
		reg |=0x80; //D8
	#endif
	
	SendCommand( drvno, 0xA1, reg); //send adr
 	SendCommand( drvno, 0xB2, addata); //send data
 	SendCommand( drvno, 0xC3, 1); //load SH
 	SendCommand( drvno, 0xC3, 2); // SH send to AD
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
	SetAD(drvno,sign_adr,ofs);
}//SetADOff


void SetADAmpRed(UINT drvno, BYTE amp)
//for AD98xx
{ //0=1x .. 64=6x
	if (amp>=64) amp=0x3F;
	SetAD(drvno,2,amp);
}//SetADAmpRed


void SetAD16Default(UINT drvno,UINT res)
{// for AD98xx
	UCHAR db;
	// SHA mode
	//db = 0xc8 ;  //1100 1000; 
	
	// cds mode Vin=4 0xD8
	// cds Vin=2 0x58
	// cds 
	db = 0xD8;  // cds mode
//	db = 0xC8;  // sha mode
//	if (res==8 )	db |= 01;  //set to 8 bit mode

	SetAD(drvno,0,db);  
	SetADAmpRed(drvno,0x0);
	SetADOff(drvno, 0,TRUE);
	SetDA(drvno, 0, 2);
}//SetAD16Default

void SetDA(UINT drvno, BYTE gain, BYTE ch)
//AD98xx setup
{ // ch=1->A  ,  CH=2->B
	BYTE lb = 0;
	BYTE hb = 0x40; // buffered
	BYTE g=0;

	if (ch==2) hb |= 0x80;  //chb=2

 
	g = gain>>4;
	hb |= g;
	g= gain<<4;
	lb |= g;

	CloseShutter(drvno);// IFC=lo
	RSEC(drvno);
	Sleep(1);	
	SendCommand( drvno, 0xA1, hb); //send hi byte
 	SendCommand( drvno, 0xB2, lb); //send lo byte
 	SendCommand( drvno, 0xC3, 1); //load SH
 	SendCommand( drvno, 0xC3, 4); // SH send to DA
	Sleep(1);
	OpenShutter(drvno);		// IFC=hi
	Sleep(1);
}//SetDA

void SetAndantFSYNC(UINT drvno, ULONG microsec)
// for Andanta FPA320x256
{	/*	
	set the high time for FSYNC
	if FSYNC goes low, the integration starts -> FSYNC shorter then VON
	FSYNC in micro sec, implements integrate while read
	0 = integrate then read: FSYNC = VON = FRAME
	here a register in the S6 FPGA is used, 
	so value must be transferred via SendCommand
	*/
	ULONG data=0;

	if (_PRGRMVON) 
		{ErrorMsg("PRGRMVON must be FALSE!");
		return;
		}

	CloseShutter(drvno);// IFC=lo
	RSEC(drvno);
	Sleep(1);	

	data = microsec;
	data &= 0xFF; 
	SendCommand( drvno, 0x90, (BYTE)data); //send lo word
	data = microsec;
	data = data >> 8; 
 	SendCommand( drvno, 0x91, (BYTE)data); //send hi word

	Sleep(1);
	OpenShutter(drvno);		// IFC=hi
	Sleep(1);
}//SetAndantFSYNC

void SendIRAndant(UINT drvno, BYTE first, BYTE secnd, BYTE third, BYTE fourth)
//for Andanta FPA320x256
{	/*	
	first ST WIN ITR GC PW1 PW0 I2 I1
	  	  1   0	  0  1   1   0   1  0
	secnd I0 AP2 AP1 AP0 BW1 BW0 IMRO NDR0
		  0   1   0   0   0   0   0    0
	third TS7 TS6 TS5 TS4 TS3 TS2 TS1 TS0
	       0   0   0    0   0  0   0   0
	fourth RO2 RO1 RO0 OM1 OM0 RE RST OEN
			0   0   0   0   0  0   0   0
	*/

	if (_PRGRMVON) 
		{ErrorMsg("PRGRMVON must be FALSE!");
		return;
		}

	CloseShutter(drvno);// IFC=lo
	RSEC(drvno);
	Sleep(1);	

	SendCommand( drvno, 0xB2, first); //send 
 	SendCommand( drvno, 0xC3, secnd); //send 
	SendCommand( drvno, 0xD4, third); //send 
 	SendCommand( drvno, 0xE5, fourth); //send 
 	SendCommand( drvno, 0xF6, 1); //load SH
 	SendCommand( drvno, 0xF6, 2); // SH send to AD
	Sleep(1);
	OpenShutter(drvno);		// IFC=hi
	Sleep(1);


}//SendIRAndant


void SendAndantROI(UINT drvno, UINT pixanz, UINT lineanz,UINT startpix,UINT startline)
{// send range of interest for OSC9806

BYTE first, secnd,third,fourth;

first = 0xC0;
first |= (startpix / 2) >> 2  ;
secnd = (startpix / 2) << 6 ;
secnd |= (startline/2) >> 1;
third = (startline/2) << 7 ;
third |= (pixanz/2) >> 1;
fourth = (pixanz/2) << 7;
fourth |= (lineanz/2) -1;

SendIRAndant(drvno, first, secnd, third, fourth);
}//SendAndantROI



void SetupIR(UINT drvno, BYTE fkt)
//for Andanta FPA320x256
{	/*	fkt=0 reset RST
		fkt=1 set to default
		fkt=2 set high gain GC
		fkt=3 set lo gain GC

	first ST WIN ITR GC PW1 PW0 I2 I1
	  	  1   0	  0  1   1   0   1  0
	secnd I0 AP2 AP1 AP0 BW1 BW0 IMRO NDR0
		  0   1   0   0   0   0   0    0
	third TS7 TS6 TS5 TS4 TS3 TS2 TS1 TS0
	       0   0   0    0   0  0   0   0
	fourth RO2 RO1 RO0 OM1 OM0 RE RST OEN
			0   0   0   0   0  0   0   0
	*/
	BYTE first=0x9A;
	BYTE secnd=0x40;
	BYTE third=0x0;
	BYTE fourth=0x0;
	ULONG keepareg =0;

	// setupir works only if not int while read 
	// if  int while read is used, rewrite register afterwards
	SetAndantFSYNC(drvno,0);

	if (fkt==0) fourth |= 0x02;//reset
	if (fkt==1) first=0x9A;//default init
	if (fkt==2) first &= 0xEF;// high gain  F7
	if (fkt==3) first |= 0x10;// lo gain   F8

	if (fkt==10) first &= 0xDF;//test with F5
//	if (fkt==10) fourth |= 0x18;//test with F5

	SendIRAndant(drvno, first, secnd, third, fourth);

}//SetupIR



BOOL CheckFFTrig(UINT drvno) //ext trigger in FF for short pulses
{	// CtrlA register Bit 6 reads trigger FF 
	// if CtrlA bit 4 was set FF is activated, write 0 to bit4 clears&disables FF
BYTE ReadCtrlA =0;
BYTE data =0;
ReadByteS0(drvno,&ReadCtrlA,4);
ReadCtrlA |= 0x080; // set to edge triggered
ReadCtrlA |= 0x010; //activate TrigFF
WriteByteS0(drvno,ReadCtrlA,4);
if ((ReadCtrlA&0x40)==0x040) 
	{// D6 high, recognize pulse
	data = ReadCtrlA&0x0EF;
	WriteByteS0(drvno,data,4);//clears Trigger FF	
	data = ReadCtrlA|0x010;
	WriteByteS0(drvno,data,4);//activate again
	return TRUE;
	}
return FALSE;
}//CheckFFTrig


// ExpTime is passed as global var here
// function is not used in WCCD
#ifndef ExpTime
ULONG ExpTime; //in micro sec - needed only in DLL, defined in DLL.h
#endif


#if (! _FIFO)
//***** Ring NOFifo fkts **************************************
//starts an own thread for writing to a ring buffer of size FifoDepth
//allocates the buffer here
//read is done by ReadRingLine if tread>twrite
// or by ReadLastRing if tread<twrite
//only used in DLL

void __cdecl ReadRingThread(void *dummy)
{// max priority
#define	FKT 1  //1 is the standard read
#define ZADR 1 //only not addressable cameras implemented here
#define DRV 1  // only 1 board implemented here




UINT64 start = 0;
UINT64 delayticks =0;
LARGE_INTEGER PERFORMANCECOUNTERVAL = {0,0};

UINT i=0;
//get global saved vals
BOOL extrig = RRT_ExtTrigFlag;
ULONG fftlines = RRT_FftLines;

//alloc Fifo
ULONG pixel = aPIXEL[Ringdrvno];
ULONG linesize = pixel * sizeof(ArrayT);
UINT linestofetch=0;
BOOL Space=FALSE;
BOOL Abbruch=FALSE;

delayticks = ustoTicks(ExpTime*1000);

pRingBuf = (pArrayT) calloc(linesize,RingFifoDepth); //allooc buffer
if (pRingBuf==0) {ErrorMsg("alloc RingFifo Buffer failed");
					abort(); }
//one more buffer for copy to global display buffer
pCopyDispBuf = (pArrayT) calloc(linesize,1);
if (pCopyDispBuf==0) {ErrorMsg("alloc pCopyDispBuf Buffer failed");
					abort(); }

// max priority
SetPriority(READTHREADPriority);

RingWRCnt=0;
DispBufValid=FALSE;

// Timer on loop
while (RingThreadOn==TRUE) // || FFValid(FFdrvno))
	{
			//we use waittrigger even in internal mode for checking keys
			WaitTrigger(DRV,extrig,&Space, &Abbruch);
			if (Abbruch==TRUE)RingThreadOn=FALSE;

			//wait for counter has reached start + needed delay
			do {QueryPerformanceCounter(&PERFORMANCECOUNTERVAL);}
			while ((start+ delayticks) >= PERFORMANCECOUNTERVAL.QuadPart); 
			//set start for next loop
			if (! extrig) 
				{
				QueryPerformanceCounter(&PERFORMANCECOUNTERVAL);
				start = PERFORMANCECOUNTERVAL.QuadPart;
				}
			 
			//this part is standard read and full bining if FFT
			OutTrigHigh(DRV);
			GETCCD(DRV,pRingBuf+RingWRCnt*pixel,fftlines,1,ZADR);//standard read
			OutTrigLow(DRV);		
			//end standard read

			if (FetchActLine==TRUE)// if display needs data, copy it here
				{
				memcpy(pCopyDispBuf,pRingBuf+RingWRCnt*pixel  , linesize);
				FetchActLine=FALSE;
				DispBufValid=TRUE;
				}
			RingWRCnt +=1;
			if (RingWRCnt>=RingFifoDepth) RingWRCnt=0;//wrap counter

//	if (RELEASETHREADms>=0) Sleep(RELEASETHREADms); // <0 don' release		
}//while (RingThreadOn==TRUE) 

free(pRingBuf);
free(pCopyDispBuf);
ResetPriority();
_endthread();
}//ReadRingThread

#else
//***************  Fifo only Functions   ***************


//***** Ring Fifo fkts **************************************
//starts an own thread for writing to a ring buffer of size FifoDepth
//allocates the buffer here
//read is done by ReadRing if tread>twrite
// or by ReadLastRing if tread<twrite

void __cdecl ReadRingThread(void *dummy)

{// max priority
UINT i=0;
int k=0;
BOOL Space=FALSE;
BOOL Abbruch=FALSE;
//alloc Fifo
ULONG pixel = aPIXEL[Ringdrvno];
ULONG linesize = pixel * sizeof(ArrayT);
UINT linestofetch=0;
if (_HWCH2) linesize *=2;
MaxLineCounter =0;

//one more buffer for copy to display
pCopyDispBuf = (pArrayT) calloc(linesize,1);
if (pCopyDispBuf==0) {ErrorMsg("alloc pCopyDispBuf Buffer failed");
					abort(); }

//SetThreadIdealProcessor(GetCurrentThread(),3);

SetPriority(READTHREADPriority);
#if (_TESTCAM)
ERR=0;
#endif

DispBufValid=FALSE;
RingThreadOFF=FALSE;
// Timer on loop
RingThreadOn = TRUE;// before loop
while (RingThreadOn==TRUE) // || FFValid(FFdrvno))
	{
	if (FFValid(Ringdrvno))
		{
		linestofetch = ReadFFCounter(Ringdrvno);
		//keep and show how many lines were written to fifo
		if (MaxLineCounter<linestofetch) MaxLineCounter=linestofetch;

		for (i=1;i<=linestofetch;i++)
			{//read all whats there
			ReadFifo(Ringdrvno, pRingBuf+RingWRCnt*pixel, 1);	


#if (_TESTRUP) //set outtrig if pixel 1 and 256 found
			if (*(pRingBuf+RingWRCnt*pixel+4) == 1) OutTrigHigh(Ringdrvno);
			if (*(pRingBuf+RingWRCnt*pixel+4) == Roilines-2) OutTrigLow(Ringdrvno);
#endif

#if (_TESTANDANTA)
			if (RingWRCnt==_AndantaLine) //test for andanta
#else
			if (FetchActLine==TRUE)// if display needs data, copy it here
#endif
				{
				memcpy(pCopyDispBuf,pRingBuf+RingWRCnt*pixel , linesize);
				//*(pCopyDispBuf)=RingWRCnt;
				FetchActLine=FALSE;
				DispBufValid=TRUE;
				//OutTrigHigh(1);
				//OutTrigLow(1);

#if (_TESTCAM) //test data for integrity	
			for (k=100;k<4500;k++)
				{if (*(pRingBuf+RingWRCnt*pixel+k)>= 1500) //wert etwa bei 1500
						{//FetchActLine=TRUE;
						ERR+=1;}}
#else
			//FetchActLine=FALSE;// dont display if no error
#endif

				}
			RingWRCnt +=1;
			if (RingWRCnt>RingFifoDepth-1) RingWRCnt=0;//wrap counter
			}
		}
/*	else //no valid line, use time to check ESC key
		{
		//we use waittrigger just for checking keys
		WaitTrigger(Ringdrvno,FALSE,&Space, &Abbruch);
		if (Abbruch==TRUE)RingThreadOn=FALSE;
		}*/
//	if (RELEASETHREADms>=0) Sleep(RELEASETHREADms); // <0 don' release		
	}
free(pRingBuf);
free(pCopyDispBuf);
ResetPriority();
RingThreadOFF=TRUE;
_endthread();
}//ReadRingThread

#endif

void StartRingReadThread(UINT drvno, ULONG ringfifodepth, ULONG threadp, __int16 releasems)
{	ULONG pixel = aPIXEL[drvno];
	ULONG linesize = pixel * sizeof(ArrayT);
	//set globals in BOARD
	Ringdrvno = drvno;
	RingRDCnt = 0;
	RingWRCnt = 0;
#if (_TESTRUP)
	Roilines = ROILINES;
#endif	
	READTHREADPriority = threadp; // pass vals as globals
	RELEASETHREADms = releasems;

	if (_HWCH2) linesize *=2;
	pRingBuf = (pArrayT) calloc(linesize,ringfifodepth); //allooc buffer
	if (pRingBuf==0) {ErrorMsg("alloc RingFifo Buffer failed, abort function");
					return; }

	RingFifoDepth=ringfifodepth;
	_beginthread(ReadRingThread,0,NULL); // starts get loop in an own thread
}

void StopRingReadThread(void)
{
	RingThreadOn = FALSE;// global variable ends thread and frees mem
}//StopRingFFTimer


//  call of the read function if write is faster then read
// ReadRingFifoThread is writing fast to the ring buffer
// if global flag FetchActLine is set, the thread copies last line to pCopyDispBuf

void StartFetchRingBuf(void)
{
FetchActLine=TRUE; //set global flag
}

BOOL RingThreadIsOFF(void)
{//return state of thread
return RingThreadOFF;
}

// FetchLastRingLine gets latest valid displ buf
void FetchLastRingLine(void* pdioden)	
{	//reads displ buf data to user buffer dioden
ULONG pixel = aPIXEL[Ringdrvno];
ULONG linesize = pixel * sizeof(ArrayT);
if (_HWCH2) linesize *=2;

//!! could hang here
//wait until thread is ready
//while ((DispBufValid==FALSE)&&(RingThreadOn))
//	{Sleep(5);}; 

if (DispBufValid==TRUE) 
	{
	memcpy(pdioden,pCopyDispBuf , linesize);
	DispBufValid=FALSE;
#if (_TESTCAM)
	FetchActLine=TRUE; //get next line if do not display only the err
#else
	FetchActLine=TRUE; //get next line
#endif
	}
};  // FetchLastRingLine


ULONG GetLastMaxLines (void)	
{	//returns the max no. of lines which accumulated
return MaxLineCounter;
};  // GetLastMaxLines


//  call of the read function if write is slower then read
void ReadRingLine(void* pdioden, UINT32 lno)	
{	//reads fifo data to user buffer dioden
ULONG pixel = aPIXEL[Ringdrvno];
ULONG linesize = pixel * sizeof(ArrayT);

memcpy(pdioden,pRingBuf+lno*pixel , linesize);
};  // ReadRingLine

ULONG ReadRingCounter()
{ULONG diff=0;
diff = RingWRCnt-RingRDCnt;
return  diff;
}//ReadRingCounter

BOOL RingValid()
{	
if ((RingWRCnt-RingRDCnt)==0) {return FALSE;}
else return TRUE;
}//RingValid


void SetExtSWTrig(BOOL ext)
{//set the global flag - used in ringreadthread
if (ext) RRT_ExtTrigFlag=TRUE;
else RRT_ExtTrigFlag=FALSE;
}//SetExtSWTrig



//*************** Hardware Fifo fkts ******************
void StartFFTimer(UINT drvno,ULONG exptime)
{//exptime in microsec
	ULONG data=0;
	ReadLongS0(drvno,&data,8); //reset	
	data &= 0xF0000000;
	data = exptime & 0x0FFFFFFF;;
	data |= 0x40000000;
	WriteLongS0(drvno,data,8); //set
}


void SWTrig(UINT drvno)
{//start 1 write cycle to FIFO by software
	UCHAR reg=0;
	ReadByteS0(drvno,&reg,11);  //enable timer
	reg |= 0x40;  
	WriteByteS0(drvno,reg,11);	
	ReadByteS0(drvno,&reg,0x12); 	
	reg |= 0x40;
	WriteByteS0(drvno,reg,0x12); //set Trigger
	reg &= 0xBF;
	WriteByteS0(drvno,reg,0x12); //reset
}


void StopFFTimer(UINT drvno)
{
	BYTE data=0;
	ReadByteS0(drvno,&data,11);
	data &= 0xBF;
	WriteByteS0(drvno,data,11);	
}


BOOL FFValid(UINT drvno)
{	// not empty & XCK = low -> true
	BYTE data=0;
	ReadByteS0(drvno,&data,0x13);
	data &= 0x80;
	if (data>0) return TRUE; 

	return FALSE;
}	


BOOL FFFull(UINT drvno)
{	// Fifo is full
	BYTE data=0;
	ReadByteS0(drvno,&data,0x13);
	data &= 0x20;
	if (data>0) return TRUE; //empty

	return FALSE;
}

BOOL FFOvl(UINT drvno)
{	// had Fifo overflow
	BYTE data=0;
	ReadByteS0(drvno,&data,0x13);
	data &= 0x08; //0x20; if not saved
	if (data>0) return TRUE; //empty

	return FALSE;
}

BOOL FlagXCKI(UINT drvno)
{	// XCKI write to FIFO is active 
	BYTE data=0;
	ReadByteS0(drvno,&data,0x13);
	data &= 0x10;
	if (data>0) return TRUE; //is running

	return FALSE;
}

void RSFifo(UINT drvno)
{	//reset FIFO and FFcounter
	BYTE data=0;
	ReadByteS0(drvno,&data,0x12);
	data |= 0x80;
	WriteByteS0(drvno,data,0x12);
	data &= 0x7F;
	WriteByteS0(drvno,data,0x12);
  }


void DisableFifo(UINT drvno)
{	//reset FIFO and FFcounter
	BYTE data=0;
	ReadByteS0(drvno,&data,0x12);
	data |= 0x80;
	WriteByteS0(drvno,data,0x12);
//	data &= 0x7F;
//	WriteByteS0(drvno,data,0x12);
  }


void EnableFifo(UINT drvno)
{	//reset FIFO and FFcounter
	BYTE data=0;
	ReadByteS0(drvno,&data,0x12);
//	data |= 0x80;
//	WriteByteS0(drvno,data,0x12);
	data &= 0x7F;
	WriteByteS0(drvno,data,0x12);
  }

void SetExtFFTrig(UINT drvno)  // set external Trigger
{
	BYTE data=0;
	ReadByteS0(drvno,&data,11);
	data |= 0x80;
	WriteByteS0(drvno,data,11);	

}//SetExtFFTrig


void SetIntFFTrig(UINT drvno) // set internal Trigger
{
	BYTE data=0;
	ReadByteS0(drvno,&data,11);
	data &= 0x7F;
	WriteByteS0(drvno,data,11);	
}//SetIntFFTrig


BYTE ReadFFCounter(UINT drvno)
{   //count number of lines in FIFO 
	//max. 16 || capacity of FIFO /(pixel*sizeof(ArrayT)) (7205=8k)
	//new: if _CNT255 ff counts up to 255
BYTE data=0;
ReadByteS0(drvno,&data,0x14);
if (_CNT255) {}
else data &= 0x0f;
return data;
}


void SetupVCLKReg(UINT drvno, ULONG lines, UCHAR vfreq)
{
WriteLongS0(drvno, lines*2, 0x18);// write no of vclks=2*lines
WriteByteS0(drvno, vfreq, 0x1b);// write v freq
VFREQ=vfreq;//keep freq global
}//SetupVCLKReg

void SetupVCLKrt(ULONG vfreq)
{
VFREQ=vfreq;// VFREQ is global in BOARD
}//Setup VFREQ from setup

void SetupDELAY(UINT drvno,ULONG delay)
{
ULONG reg=0;
DELAY=delay;// DELAY is global in BOARD

//new for new boards check for S0 length if DELAY reg is there
ReadLongIOPort(drvno,&reg,0); //read LCR0 for check length 0xffffffco
reg	=  ~reg; //length is inverted
if (reg>0x34) WriteLongS0(drvno,DELAY, 0x34);	//set WRFFDELAY

}//SetupDELAY


void PickOneFifoscan(UINT drvno,pArrayT pdioden,BOOL* pabbr,BOOL* pspace,ULONG fkt)  
{	//get one scan off free running fifo timer
	//don't enable Fifo during a read
	//so wait for a complete read and enable afterwards

	do {//here used for Keypressed
		WaitTrigger(drvno,FALSE,pspace, pabbr);//test abbruch
		}
	while ((!FlagXCKI(drvno)) && (*pabbr==FALSE));//sync to active read

	//don't reset Fifo during a read
	do {//here used for Keypressed
		WaitTrigger(drvno,FALSE,pspace, pabbr);//test abbruch
		}
	while ((FlagXCKI(drvno)) && (*pabbr==FALSE));//wait for not active read

 	RSFifo(drvno);

	do {//here used for Keypressed and 1 line valid
		WaitTrigger(drvno,FALSE,pspace, pabbr);//test abbruch
		}
	while ((! FFValid(drvno)) && (*pabbr==FALSE));//wait for 1 line in Fifo

	//copy data from fifo to buffer pDIODEN
	ReadFifo(drvno,pdioden,fkt);


//	DisableFifo(drvno);
}//PickOneFifoscan



//********************  thread priority stuff

BOOL ThreadToPriClass(ULONG threadp,DWORD *priclass,DWORD *prilevel)
{ //converts threadp value (1..31) to process priority class and thread priority level 

DWORD propriclass[31] = {IDLE_PRIORITY_CLASS,IDLE_PRIORITY_CLASS,IDLE_PRIORITY_CLASS,IDLE_PRIORITY_CLASS,IDLE_PRIORITY_CLASS,IDLE_PRIORITY_CLASS,
						 NORMAL_PRIORITY_CLASS,NORMAL_PRIORITY_CLASS,NORMAL_PRIORITY_CLASS,NORMAL_PRIORITY_CLASS,NORMAL_PRIORITY_CLASS,
						 HIGH_PRIORITY_CLASS,HIGH_PRIORITY_CLASS,HIGH_PRIORITY_CLASS,HIGH_PRIORITY_CLASS,
						 REALTIME_PRIORITY_CLASS,REALTIME_PRIORITY_CLASS,REALTIME_PRIORITY_CLASS,REALTIME_PRIORITY_CLASS,REALTIME_PRIORITY_CLASS,
						 REALTIME_PRIORITY_CLASS,REALTIME_PRIORITY_CLASS,REALTIME_PRIORITY_CLASS,REALTIME_PRIORITY_CLASS,REALTIME_PRIORITY_CLASS,
						 REALTIME_PRIORITY_CLASS,REALTIME_PRIORITY_CLASS,REALTIME_PRIORITY_CLASS,REALTIME_PRIORITY_CLASS,REALTIME_PRIORITY_CLASS,
						 REALTIME_PRIORITY_CLASS};

DWORD threprilevel[31] = {THREAD_PRIORITY_IDLE,THREAD_PRIORITY_LOWEST,THREAD_PRIORITY_BELOW_NORMAL,THREAD_PRIORITY_NORMAL,THREAD_PRIORITY_ABOVE_NORMAL,
						THREAD_PRIORITY_HIGHEST,THREAD_PRIORITY_LOWEST,THREAD_PRIORITY_BELOW_NORMAL,THREAD_PRIORITY_NORMAL,THREAD_PRIORITY_ABOVE_NORMAL,
						THREAD_PRIORITY_HIGHEST,THREAD_PRIORITY_BELOW_NORMAL,THREAD_PRIORITY_NORMAL,THREAD_PRIORITY_ABOVE_NORMAL,THREAD_PRIORITY_HIGHEST,
						THREAD_PRIORITY_IDLE,THREAD_PRIORITY_IDLE,THREAD_PRIORITY_IDLE,THREAD_PRIORITY_IDLE,THREAD_PRIORITY_IDLE,THREAD_PRIORITY_IDLE,
						THREAD_PRIORITY_LOWEST,THREAD_PRIORITY_BELOW_NORMAL,THREAD_PRIORITY_NORMAL,THREAD_PRIORITY_ABOVE_NORMAL,THREAD_PRIORITY_HIGHEST,
						THREAD_PRIORITY_HIGHEST,THREAD_PRIORITY_HIGHEST,THREAD_PRIORITY_HIGHEST,THREAD_PRIORITY_HIGHEST,THREAD_PRIORITY_TIME_CRITICAL};
// range check
if ((0 < threadp) && ( threadp < 32)) {
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
if (! ThreadToPriClass(threadp,&NEWPRICLASS,&NEWPRILEVEL))
	{ 	ErrorMsg(" threadp off range ");
		return FALSE; 
	}

hPROCESS = GetCurrentProcess();
OLDPRICLASS = GetPriorityClass(hPROCESS);

if (! SetPriorityClass(hPROCESS,NEWPRICLASS))
	{	ErrorMsg(" No Class set ");
		return FALSE;
	}

hTHREAD = GetCurrentThread();
OLDTHREADLEVEL = GetThreadPriority(hTHREAD);

if (! SetThreadPriority(hTHREAD,NEWPRILEVEL)) 
	{	ErrorMsg(" No Thread set ");
		return FALSE;	
	}
return TRUE;
}//SetPriority



BOOL ResetPriority()
{
// reset the class Priority and stop thread
if (! SetPriorityClass(hPROCESS, OLDPRICLASS)) 
	{ErrorMsg(" No Class reset ");	
	return FALSE;}
if (! SetThreadPriority(hTHREAD, OLDTHREADLEVEL)) 
	{ErrorMsg(" No Thread reset ");
	return FALSE; }

return TRUE;
}


//***********************  System Timer in Ticks
UINT64 LargeToInt(LARGE_INTEGER li)
{ //converts Large to Int64
UINT64 res = 0;
res = li.HighPart;
res = res << 32 ;
res = res + li.LowPart;
return res;
} //LargeToInt



UINT64 InitHRCounter()
{//returns TPS ticks per sec
// init high resolution counter 
BOOL ifcounter;
UINT64 tps=0;
LARGE_INTEGER freq ;
freq.LowPart  = 0;
freq.HighPart = 2;

//tps:: ticks per second = freq
ifcounter = QueryPerformanceFrequency(&freq);	
tps = LargeToInt(freq); //ticks per second

if (tps==0) ErrorMsg(" System Timer Error ");

return tps;

} // InitHRCounter

UINT64 ticksTimestamp()
{
LARGE_INTEGER PERFORMANCECOUNTERVAL = {0,0};

QueryPerformanceCounter(&PERFORMANCECOUNTERVAL);
return PERFORMANCECOUNTERVAL.QuadPart;

}//ticksTimestamp


//calc delay in ticks from us
UINT64 ustoTicks(ULONG us)
{// init high resolution counter 
 // and calcs DELAYTICKS from m_belPars.m_belDelayMsec
BOOL ifcounter;
UINT64 delaytks=0;
UINT64 tps=0; //ticks per second
LARGE_INTEGER freq ;
freq.LowPart  = 0;
freq.HighPart = 0;

//get tps: ticks per second
ifcounter = QueryPerformanceFrequency(&freq);	
tps = LargeToInt(freq); //ticks per second

if (tps==0) return FALSE; // no counter available

delaytks = us ;
delaytks = delaytks * tps;
delaytks = delaytks / 1000000;  
return delaytks;
} // ustoTicks



UINT32 Tickstous(UINT64 tks)
{// init high resolution counter 
 // and returns ms
BOOL ifcounter;
UINT64 delay=0;
UINT64 tps=0; //ticks per second
LARGE_INTEGER freq ;
freq.LowPart  = 0;
freq.HighPart = 0;

//get tps: ticks per second
ifcounter = QueryPerformanceFrequency(&freq);	
tps = LargeToInt(freq); //ticks per second

if (tps==0) return 0; // no counter available

delay = tks * 1000000;
delay = delay / tps;
return (UINT32) delay;
} // Tickstous


// ************************  COOLER Functions  *********************

void ActCooling(UINT drvno, BOOL on)
{//activates cooling with IFC signal
	if (on) {	OpenShutter(drvno) ;}
	else CloseShutter(drvno) ;
}


BOOL TempGood(UINT drvno,UINT ch)
{//reads EOI Signal = D4 CTRLC
BYTE CtrlC=0;
ReadByteS0(drvno,&CtrlC,6);

if (ch==1)
	{	
	if ((CtrlC&0x10)==0x10)
		{return FALSE;}
	else return TRUE;
	}
if (ch==2)
	{	
	if ((CtrlC&0x20)==0x20)
		{return FALSE;}
	else return TRUE;
	}
if (ch==3)
	{	
	if ((CtrlC&0x40)==0x40)
		{return FALSE;}
	else return TRUE;
	}
if (ch==4)
	{	
	if ((CtrlC&0x80)==0x80)
		{return FALSE;}
	else return TRUE;
	}

return FALSE;
}//TempGood


void SetTemp(UINT drvno, ULONG level)
{// set temperature controler (8 levels)
CloseShutter(drvno);// IFC=lo
Sleep(1);	

if (level>=8) level =0;
SendCommand(drvno,0xA1,(BYTE)level);

Sleep(1);
OpenShutter(drvno);		// IFC=hi
Sleep(1);

if (level==0) ActCooling(drvno,FALSE);
}

void RSArray(UINT drvno)
{//resets lines of area array and stop generator
WriteLongS0(drvno,0,0x2C);
}

void SetArray(UINT drvno, ULONG lines)
{//sets lines of area array and enable generator
ULONG data=lines;
RSArray(drvno);
if (data>10000) data = 10000;
if (data != 0) data |= 0x8000; //set enable/reset bit 15 if !=0
WriteLongS0(drvno,data,0x2C);
}

void SetROILines(ULONG lines)
{//set global var roi=range of interest
#if (_TESTRUP)
Roilines = lines; 
#endif
}

