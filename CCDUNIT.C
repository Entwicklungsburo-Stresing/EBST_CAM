
#ifndef _DLL
	#include "board.h"
#endif
BYTE Dispcnt = 0;
int yVal = 0;
volatile int testcnt = 0;
UINT choosen_board = 1;
BOOL both_boards = FALSE;
BOOL cont_mode = FALSE;

/*
void GetRmsVal(BYTE ch, ULONG nos)
			{
			double trms=0.0;
			double mwf = 0.0;
			double sumvar =0.0;
			unsigned int i=0;
			for ( i=0;i<nos;i++)
				{//get mean val
				mwf += TRMSVals[ch][i];
				}
			mwf /= nos;
			for ( i=0;i<nos;i++)
				{// get varianz
				trms = TRMSVals[ch][i];
				trms = trms - mwf;
				trms *= trms;
				sumvar += trms;
				}
			trms = sumvar / (nos+1);
			trms = sqrt (trms);
			TRMSval[ch]=trms;
	
			}//GetRmsVal

void CalcTrms()
{// online calc TRMS noise val of pix

		ULONG nos=NOS;  //number of samples
		ULONG pix=TRMSpix;// pixel for what the rms noise in time is calculated
		

		TRMSVals[0][m_lfdTrmsNr]=DisplData[0][pix]; //DIODEN[0][pix]; //keep act val
		TRMSVals[1][m_lfdTrmsNr]=DisplData[1][pix];//DIODEN[1][pix]; //keep act val
		m_lfdTrmsNr++;
		if (m_lfdTrmsNr % nos == 0)
			{
			GetRmsVal(0,nos);
			GetRmsVal(1,nos);
			m_lfdTrmsNr = 0;
			}

}//CalcTrms
*/

void Resort_to_DBs(UINT drvno, void* p1dim, void* p2dim, BYTE db1,BYTE db2)
{// repack array word [pixel][db]  to long [db][pixel]
	//used in CCDExamp
/*
ULONG i=0;
long dat=0;
pArrayT p1Dim = (pArrayT) p1dim;
pArrayT p2Dim = (pArrayT) p2dim;
for (i=0;i<	_PIXEL;i++)
	{
	dat = *(p1Dim+i);
	dat = dat >> 16;
	*(p2Dim+_PIXEL*(db2-1)+i) = dat & 0x0FFFF;
	*(p2Dim+_PIXEL*(db1-1)+i) = p2Dim[i]&0x0FFFF;
	}
*/
	
typedef WORD w2dim[_PIXEL][2];		// different packed source
typedef long l2dim[4][_PIXEL];	// destination array
ULONG i=0;

w2dim* ppack = (w2dim*) p2dim;
l2dim* pw2dim = (l2dim*) p2dim;

for (i=0;i<	_PIXEL;i++)
	{
	(*pw2dim)[db2-1][i] = (*ppack)[i][1]; //hi word
	(*pw2dim)[db1-1][i] = (*ppack)[i][2]; //lo word
	}

}// Resort_to_DBs



int YVal(unsigned long db, int pixeli)
	{ unsigned long val;
	ULONG xofs=0;
	if ((PixelOdd)&&(pixeli<(_PIXEL*CAMCNT))) { xofs +=1;}
	else xofs=0;
//	xofs=XStart;
	 val = DisplData[db-1][pixeli*XOFF+xofs] ;
	 val = val >> YSHIFT;
	 if (val>YLENGTH) val = YLENGTH;
	 return val; 
	};

	
void Display(unsigned long db,BOOL Plot)
// Plot=TRUE -> dense 2ms; Plot=FALSE -> Dots=1ms; 
{	long int i,y1,y2,val1, val2 ;
	int xlength = XLENGTH;
	unsigned long pencolor = 0x0000ff;   //0=sw, 00bbggrr

/*
	if (_PIXEL<=600) 
		{xlength =_PIXEL;
		XOFF = 1;}
	else
		{xlength = 600;
		/*XOFF = _PIXEL / 600;}
*/
	//!!!
	//XOFF = 1;

	Rectangle(hMSDC,LOX-1,LOY-1,LOX+xlength+1,LOY+YLENGTH+2);		// Clear
/*
	for (i=0;i<2;i++)//first pixels are not valid
			{
			//DisplData[db-1][i] = 0;
			DisplData[db][i] = 0;
			}
*/
	if (Plot)
	for	(i=0;i<xlength-1;i++)
	{	val1 = YVal(db,i);
		if (DISP2) val1 /= 2;
		val2 = YVal(db,i+1);
		if (DISP2) val2/= 2;
		y1 = LOY + YLENGTH - val1;
		y2 = LOY + YLENGTH - val2;
		MoveToEx(hMSDC, LOX+i,y1,NULL);
		LineTo(hMSDC,LOX+i+1,y2);
	}
	else
	for	(i=0;i<=xlength-1;i++)
	{	val1 = YVal(db,i);
		if (DISP2) val1 /= 2;
		y2 = LOY + YLENGTH - val1;
		//SetPixelV(adc,LOX+i,y2,pencolor);
		SetPixelV(hMSDC,LOX+i,y2,pencolor);
	};
	
	if (DISP2) //display 2 graphics with db1 and db2 on top of each other
		{		//array is: dioden[db][i] and dioden[db+1][i]
		if (Plot)
			for	(i=0;i<xlength-1;i++)
			{	y1 = LOY + YLENGTH/2 - YVal(db,i + _PIXEL)/2 ;//y1 = LOY + YLENGTH/2 - YVal(db+1,i)/2 ;
			y2 = LOY + YLENGTH/2 - YVal(db,i+1+_PIXEL)/2;//y2 = LOY + YLENGTH/2 - YVal(db+1,i+1)/2;
			MoveToEx(hMSDC, LOX+i,y1,NULL);
			LineTo(hMSDC,LOX+i+1,y2);
			}
		else
			for	(i=0;i<=xlength-1;i++)
			{
			y2 = LOY + YLENGTH/2 - YVal(db+1,i)/2  ;
			SetPixelV(hMSDC,LOX+i,y2,pencolor);
			};
		}
	};



void CopytoDispbuf(ULONG scan)
{	//display buffer is long
	//data array is word
	
	int i;
#ifdef _DLL
	UINT16 tempBuf[1200];
	DLLReturnFrame(choosen_board, scan, 0, &tempBuf);

	
#else
	
	PUSHORT tempBuf;
	tempBuf = pDMABigBufBase[choosen_board] + CAMCNT * scan * _PIXEL;
#endif
	for (i = 0; i < (_PIXEL*CAMCNT - 1); i++) {

		DisplData[0][i] = *(tempBuf + i);
	}

	if (both_boards){
#ifdef _DLL
		DLLReturnFrame(2, scan, 0, &tempBuf);
#else
		tempBuf = pDMABigBufBase[2] + scan * _PIXEL*CAMCNT;
#endif
		for (i = 0; i < (_PIXEL*CAMCNT - 1); i++) {
			DisplData[1][i] = *(tempBuf + i);//DIODENRingBuf[i + 0*FirstPageOffset + 0 * RAMPAGESIZE];//20: its a random number of the Ringbuffer (max 99)
		}
	}


	/* was GS
	for (i = -150; i < (_PIXEL-150); i++){
		//pDMABuf++;
		DisplData[0][i] = *(tempBuf  + i);//DIODENRingBuf[i + 0*FirstPageOffset + 0 * RAMPAGESIZE];//20: its a random number of the Ringbuffer (max 99)
	}
	*/
	//pDMABuf -= _PIXEL;
	//for (i = 0; i < _PIXEL; i++)
		//	DisplData[1][i] = DIODEN[1][i];//* (pDIODEN+_PIXEL+i);
		
		//GetNextScan = FALSE;//act vals in disp buffer
	

}







//****************************  FIFO functions   ********************
/*
void MeasureFifo(HDC aDC)   
//function reads every line from FIFO to main RAM
//be aware of a buffer overflow if looptime and pixel exceeds FIFO size
	{	int i = 0;	
		int j=0;

	BOOL Abbruch = FALSE;
	BOOL Space = FALSE;
	DWORD oldpriclass=0;
	DWORD oldprithread=0;
	DWORD priority=0;
	ULONG linesize = _PIXEL * sizeof(ArrayT);
	ULONG RingFifoDepth=256; //size of ring buffer

	// if thread is wanted ... - for highest speeds cams
	#if  (_USETHREAD)
	hPROCESS = GetCurrentProcess();
	oldpriclass = GetPriorityClass(hPROCESS);
	priority=REALTIME_PRIORITY_CLASS;
	if (! SetPriorityClass(hPROCESS,priority)) ErrorMsg(" No Class set ");
	hTHREAD = GetCurrentThread();
	oldprithread = GetThreadPriority(hTHREAD);
	if (! SetThreadPriority(hTHREAD,THREAD_PRIORITY_TIME_CRITICAL)) ErrorMsg(" No Thread set ");
	#endif

	//alloc ring buffer
	pRingFifo = (pArrayT) calloc(linesize,RingFifoDepth); //allooc buffer
	if (pRingFifo==0) {ErrorMsg("alloc RingFifo Buffer failed");
					abort(); }
	//alloc one more buffer for copy to display
	pCopyDispBuf = (pArrayT) calloc(linesize,1);
	if (pCopyDispBuf==0) {ErrorMsg("alloc pCopyDispBuf Buffer failed");
					abort(); }

//hCopyToDispBuf = CreateMutex(NULL,FALSE,NULL);

	ClrRead(choosen_board, 0, 0, 16);
	GetNextScan=FALSE;
	MaxLineCnt=0;

	//Clear FIFO and start timer
	RSFifo(choosen_board);
	if (EXTTRIGFLAG)
		{StopFFTimer(choosen_board);
		SetExtTrig(choosen_board);}
	else
		{SetIntTrig(choosen_board);
		StartFFTimer(choosen_board, ExpTime*1000);} // in micro sec


	do	{
		i=0;
//		ReadFifo(choosen_board,pDIODEN,0);//clear array for add
		do  {
			i += 1;

			do	{//wait for linecounter has at least one line
				}
			while ((! FFValid(choosen_board)) );//&& (!Abbruch));
			j=0;
			do //if there are more then1 line, get them all
				{//
				//OutTrigHigh(choosen_board);
				ReadFifo(choosen_board,pRingFifo+,FKT);//FKT);
				//OutTrigLow(choosen_board);
				j+=1;
				}
			while (ReadFFCounter(choosen_board)>=1);
			if (j>MaxLineCnt) MaxLineCnt=j;
 
			if (ShowTrms) CalcTrms();
     
			if (GetNextScan==TRUE)
				{
				WaitForSingleObject(hCopyToDispBuf, INFINITE);
				memcpy(pCopyDispBuf,pRingFifo+WrOffset*pixel , linesize);
				CopytoDispbuf();
				UpdateDispl=TRUE;
				GetNextScan=FALSE;
				ReleaseMutex(hCopyToDispBuf);
				}
			}
		while((i<ADDREP) && (Running));	
	} 
	while (Running);
	StopFFTimer(choosen_board);
	Running=FALSE;
	SetIntTrig(choosen_board);//stop if external trigger

	CloseHandle(hCopyToDispBuf) ;
#if (_USETHREAD)
	// we need to reset the Class - not really: return kills thread anyway
	if (! SetPriorityClass(hPROCESS, oldpriclass)) ErrorMsg(" No Class reset ");
	if (! SetThreadPriority(hTHREAD, oldprithread)) ErrorMsg(" No Class reset ");	
	#endif
};	 // MeasureFifo- readloop
*/

void UpdateTxT(void)
{
	char TrmsString[400];
	int j = 0;
	int i = 0;
	int xPos = 0;


	
	//B!j = sprintf_s(TrmsString, 260, " , linecounter max is %u , ", GetLastMaxLines());
	//B!j += sprintf_s(TrmsString + j, 260, " ISRTime: %u us", GetISRTime());

	
	xPos = GetCursorPosition();
	if (xPos < 0)
		xPos = 1;
	if (xPos > XLENGTH)
		xPos = XLENGTH;

	if ((Dispcnt % 10) == 0) //display only every 10th val or it's not readable
		yVal = DisplData[0][xPos];// YVal(1, xPos);
	Dispcnt += 1;
	j += sprintf_s(TrmsString + j, 260, " x: %i y: %i ", xPos, yVal);

	if (ShowTrms)
	{
		//j=sprintf(TrmsString,"                                                           ") ;//clear old display
		//TextOut(hMSDC,20,YLENGTH + 50,TrmsString,j);
		j += sprintf_s(TrmsString + j, 260, " Trms of Pixel %lu CH1 is %.1f ", TRMSpix, TRMSval[0]);
		if (_HWCH2)	j += sprintf_s(TrmsString + j, 260, " , CH2 is %.1f            ", TRMSval[1]);
		TextOut(hMSDC, 20, YLENGTH + 50, TrmsString, j);
	}

#if (_ERRTEST)
		if (*(pDMABuf + 1083) != 539)
		{
			ErrVal = *(pDMABuf + 1083);
			ErrCnt += 1;
		}
	j += sprintf_s(TrmsString + j, 260, " , err=%d , val=%d", ErrCnt, ErrVal);
#endif

	/*B!if (FFOvl(choosen_board) == TRUE) 	{
		j += sprintf_s(TrmsString + j, 260, " , overflow! "); }
	else j += sprintf_s(TrmsString + j, 260, "                      ");
	*/
	TextOut(hMSDC, 20, YLENGTH + 50, TrmsString, j);

	RedrawWindow(hMSWND, NULL, NULL, RDW_INVALIDATE);
}

int GetCursorPosition()
{
	POINT CurPos;

	GetCursorPos(&CurPos);
	ScreenToClient(hMSWND, &CurPos);

	int x = CurPos.x -LOX;
	return (int)x;// CurPos.x;
}

/*
void DisplayData()
{
	char header[260];
	int j=0;
	int i=0;

// display loop in normal priority
do
	{
//	if (UpdateDispl)
		{
		//wait for ring thread is running
		do {} while (RingThreadIsOFF()) ;

		//B! FetchLastRingLine(pDIODEN);
		TICKSDISP = ticksTimestamp();		

		CopytoDispbuf(Nob*Nospb);
		Display(1,PLOTFLAG);
		if (ShowTrms) CalcTrms();
		//OutTrigLow(1);
		TICKSDISP = ticksTimestamp()-TICKSDISP;//measure displaytime

//		UpdateDispl=FALSE;
		UpdateTxT();
		}

	Sleep(20); // depends how many frames per second you can see on a screen
	//GetNextScan = TRUE;
	}
while (Running);

j=sprintf_s(header,260," Measurement stopped !                                                                       ");
TextOut(hMSDC,100,LOY-17,header,j);
RedrawWindow(hMSWND,NULL,NULL,RDW_INVALIDATE);

//ActMouse(choosen_board);
}//DisplayData
*/


void Contimess(void *dummy)


	{// main read loop setup

	int i = 0;
	int j = 0;
	int k = 0;
	int n = 0;
	int erri = 0;
	int max = 0;
	BOOL Abbruch = FALSE;
	BOOL Space = FALSE;
	ULONG test;
	ULONG lwritten = 0;
	ULONG ldata = 0;
	char fn[3000];
	char header[260];
	volatile	ULONG S0Data = 0;
	//PUSHORT pdata = pDIODEN;
	ULONG cnt = 0;
	BOOL FFfull = FALSE;
	char TrmsString[2000];
	DWORD oldpriclass = 0;
	DWORD oldprithread = 0;
	DWORD priority = 0;
	ULONG pixel = 0;
	ULONG maxcnt = 0;
	ULONG errcnt = 0;
	ULONG val[1000];
	ULONG val1 = 0;
	ULONG gain = 0;


	contimess_run_once = TRUE;

	// if thread is wanted ...
/*
#if  (_USETHREAD)//
	hPROCESS = GetCurrentProcess();
	oldpriclass = GetPriorityClass(hPROCESS);
	// in WinNT GetAsyncKeyState -> WaitTrigger works only with HIGH_PRIORITY_CLASS
	//for that reason we use our own GetKey, which is part of board.c
	priority = REALTIME_PRIORITY_CLASS;  // priority = HIGH_PRIORITY_CLASS; };
	if (!SetPriorityClass(hPROCESS, priority)) ErrorMsg(" No Class set ");
	hTHREAD = GetCurrentThread();
	oldprithread = GetThreadPriority(hTHREAD);

	//	if (!SetThreadPriority(hTHREAD, THREAD_PRIORITY_TIME_CRITICAL)) ErrorMsg(" No Thread set ");
	if (!SetThreadPriority(hTHREAD, 15)) ErrorMsg(" No Thread set ");
#endif
*/	

	//stop all and clear FIFO
	//WDC_Err(choosen_board);
#ifdef _DLL
	DLLStopFFTimer(choosen_board);
	DLLSetIntTrig(choosen_board);
	DLLRSFifo(choosen_board);

	gain = 6;
	DLLSetADGain(choosen_board, 1, gain, gain, gain, gain, gain, gain, gain, gain); //set gain to values g1..g8 in Board.C
																				 //	SetADGain(choosen_board, 1, 0, 0, 0, 0, 0, 0, 0, 0); //set gain to values g1..g8 in Board.C
	if (both_boards) {
		DLLStopFFTimer(2);
		DLLSetIntTrig(2);
		DLLRSFifo(2);



		//setups
		//	SetupDELAY(choosen_board,DELAYini);	//init WRFIFO delay

		DLLRsTOREG(2); // reset TOREG
					//set TrigOut, default= XCK
		DLLSetTORReg(2, 0);

		gain = 6;
		DLLSetADGain(2, 1, gain, gain, gain, gain, gain, gain, gain, gain);
	}
#else


	StopFFTimer(choosen_board);
	SetIntFFTrig(choosen_board);
	RSFifo(choosen_board);
	//setups
	//	SetupDELAY(choosen_board,DELAYini);	//init WRFIFO delay

	//RsTOREG(choosen_board); // reset TOREG
	//set TrigOut, default= XCK
	//SetTORReg(choosen_board, 8);




	//SetTORReg(choosen_board, 0);  //XCK  
	//SetTORReg(choosen_board,1);	//outtrig
	//SetTORReg(choosen_board, 2);  //FFREAD  geht : 20 microsec
	//SetTORReg(choosen_board, 3);  // area read
	//SetTORReg(choosen_board, 1);// 0);  // 
	// ohne displ : 9 microsec

	//B!
	//SendFLCAM(choosen_board, 1, 0x0, 1);	//reset

	//Version 2 ch byte
	//-2	SendFLCAM(choosen_board,1, 0x28, 0x0000); //set to byte wise mode
	//-2	SendFLCAM(choosen_board, 1, 0x46, 0x8409); //set to 2 wire mode & 14bit & MSB first

	//		SendFLCAM(choosen_board,1, 0x26, 0x00b0); //set custom pattern D4=bit0 -> 10=1, 80=8, 800=80, f0=1111=15, fff=max
	//	SendFLCAM(choosen_board, 1, 0x25, 0x11); //set custom pattern with D0(=D12)+D1(=D13)=1 -> max=3fff=16383
	//		SendFLCAM(choosen_board,1, 0x25, 0x10); //single custom pattern
	//		SendFLCAM(choosen_board, 1, 0x42, 0x00); //clk align
	//  SendFLCAM(choosen_board,1, 0x25, 0x40); //ramp pattern

	//RSEC(choosen_board);

	SendFLCAM(DRV, 3, 1, 1088);//set Pixel in Camera Regs
	SendFLCAM(DRV, 3, 2, 3);//activate channel a and b
	SendFLCAM(DRV, 1, 0, 0x80);//ad setup
	SendFLCAM(DRV, 1, 2, 0x1);//Data = 5h, for test signal (defined in slide 7)
							//Data = 1h, for ADC data
	//SendFLCAM(DRV, 1, 3, 0xaa);//testpattern
	//SendFLCAM(DRV, 1, 4, 0xaa);//testpattern


	gain = 6;
	SetADGain(choosen_board, 1, gain, gain, gain, gain, gain, gain, gain, gain); //set gain to values g1..g8 in Board.C
																				 //	SetADGain(choosen_board, 1, 0, 0, 0, 0, 0, 0, 0, 0); //set gain to values g1..g8 in Board.C
	if (both_boards) {
		StopFFTimer(2);
		SetIntFFTrig(2);
		RSFifo(2);



		//setups
		//	SetupDELAY(choosen_board,DELAYini);	//init WRFIFO delay

		RsTOREG(2); // reset TOREG
					//set TrigOut, default= XCK
		SetTORReg(2, 0);

		gain = 6;
		SetADGain(2, 1, gain, gain, gain, gain, gain, gain, gain, gain);
	}

	SendFLCAM(DRV, 3, 1, 1088);
	SendFLCAM(DRV, 3, 2, 3);
	SendFLCAM(DRV, 3, 3, 1);




#endif


	
	UserBufInScans = Nospb;

	//!!GS
	//Nob = 1;
	//DMA_Setup
	

	// write header
	j=sprintf_s(header,260," Online Loop - Cancel with ESC or space- key  " );
	TextOut(hMSDC,100,LOY-17,header,j);
	RedrawWindow(hMSWND,NULL,NULL,RDW_INVALIDATE);

	//GS why delay?
	Sleep(100);




#ifdef _DLL
	if(both_boards)
		DLLReadFFLoop(choosen_board, ExpTime, EXTTRIGFLAG, 0, 0, 3);//if both cams are activated
	else
		DLLReadFFLoop(choosen_board, ExpTime, EXTTRIGFLAG, 0, 0, choosen_board);
#else
	IsrCounter = 0;
	struct ffloopparams params;
	if (both_boards)	params.board_sel = 3;
	else				params.board_sel = choosen_board;
	params.exptus = ExpTime;
	params.exttrig = EXTTRIGFLAG;
	params.blocktrigger = 0;
	params.btrig_ch = 0;
	//_beginthread(ReadFFLoopThread, 0, &params);//thread
	_beginthreadex(0, 0, &ReadFFLoopThread, &params, 0, 0);//cam_thread[0] = (HANDLE)_beginthreadex(0, 0, &ReadFFLoopThread, &params, 0, 0);//threadex


	DWORD64 IsrNumber = Nob*Nospb / (DMA_BUFSIZEINSCANS/DMA_HW_BUFPARTS);
	if (both_boards) IsrNumber *= 2;
	if (CAMCNT == 2) IsrNumber *= 2;
	while (IsrCounter < IsrNumber){
		j = sprintf_s(header, 260, " Online Loop - Cancel with ESC or space- key isr: %i of %i ",IsrCounter+1, IsrNumber);//+1 cheating
		TextOut(hMSDC, 100, LOY - 17, header, j);
		RedrawWindow(hMSWND, NULL, NULL, RDW_INVALIDATE);
	}
	//ReadFFLoop(choosen_board, ExpTime, FREQ, EXTTRIGFLAG, 0,  0);
	BOOL cancel = FALSE;
	if(cont_mode)
		while (!cancel) {

			CopytoDispbuf(0);
			Display(1, PLOTFLAG);

			UpdateTxT();

			if (GetAsyncKeyState(VK_ESCAPE))
				cancel = TRUE;
			if (GetAsyncKeyState(VK_SPACE))
				cancel = TRUE;
		}
	Sleep(100);//for the thread if there is just one isr 
	//start 2nd thread for getting data in highest std priority, ring=200 lines
#endif
	/*
	if (!HWINTR_EN)
	{
		StartRingReadThread(choosen_board, 200, _THREADPRI, -1);
	}
	else
		StartReadWithDma(choosen_board);
		*/
	//StartRingReadThread(choosen_board, 200, _THREADPRI, -1);
	//while (!RingThreadOn) {}; // wait until ReadThread is running
/*
	//start thread to display data
	if(DMAAlreadyStarted)
		_beginthread( DisplayData, 2000, &hMSDC ); 


	//start hardware timer - must be the last
	if (DMAAlreadyStarted)
		if (EXTTRIGFLAG)
			 {SetExtFFTrig(choosen_board); }
		else
			{SetIntFFTrig(choosen_board);
			StartFFTimer(choosen_board, ExpTime); // in micro sec
			}
*/
	}//Contimess
