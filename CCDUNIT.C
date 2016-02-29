
BYTE Dispcnt = 0;
int yVal = 0;
volatile int testcnt = 0;

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
	if ((PixelOdd)&&(pixeli<_PIXEL)) { xofs +=1;}
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
			{	y1 = LOY + YLENGTH/2 - YVal(db+1,i)/2 ;
			y2 = LOY + YLENGTH/2 - YVal(db+1,i+1)/2;
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



CopytoDispbuf()
{	//display buffer is long
	//data array is word
	int i;

	//while (!GetNextScan){ Sleep(5); }
	testcnt++;
	if (testcnt == 100){
 		testcnt = 0;
		DIODEN[0][0];
	}
	for (i = 0; i < _PIXEL; i++){
		//pDMAUserBuf++;
		DisplData[0][i] = *(pDMAUserBuf + i);//DIODENRingBuf[i + 0*FirstPageOffset + 0 * RAMPAGESIZE];//20: its a random number of the Ringbuffer (max 99)
	}
	//pDMAUserBuf -= _PIXEL;
	for (i = 0; i < _PIXEL; i++)
			DisplData[1][i] = DIODEN[1][i];//* (pDIODEN+_PIXEL+i);
		
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

	ClrRead(DRV, 0, 0, 16);
	GetNextScan=FALSE;
	MaxLineCnt=0;

	//Clear FIFO and start timer
	RSFifo(DRV);
	if (EXTTRIGFLAG)
		{StopFFTimer(DRV);
		SetExtTrig(DRV);}
	else
		{SetIntTrig(DRV);
		StartFFTimer(DRV, ExpTime*1000);} // in micro sec


	do	{
		i=0;
//		ReadFifo(DRV,pDIODEN,0);//clear array for add
		do  {
			i += 1;

			do	{//wait for linecounter has at least one line
				}
			while ((! FFValid(DRV)) );//&& (!Abbruch));
			j=0;
			do //if there are more then1 line, get them all
				{//
				//OutTrigHigh(DRV);
				ReadFifo(DRV,pRingFifo+,FKT);//FKT);
				//OutTrigLow(DRV);
				j+=1;
				}
			while (ReadFFCounter(DRV)>=1);
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
	StopFFTimer(DRV);
	Running=FALSE;
	SetIntTrig(DRV);//stop if external trigger

	CloseHandle(hCopyToDispBuf) ;
#if (_USETHREAD)
	// we need to reset the Class - not really: return kills thread anyway
	if (! SetPriorityClass(hPROCESS, oldpriclass)) ErrorMsg(" No Class reset ");
	if (! SetThreadPriority(hTHREAD, oldprithread)) ErrorMsg(" No Class reset ");	
	#endif
};	 // MeasureFifo- readloop
*/

int GetCursorPosition()
{
	POINT CurPos;

	GetCursorPos(&CurPos);
	ScreenToClient(hMSWND, &CurPos);

	int x = CurPos.x -LOX;
	return (int)x;// CurPos.x;
}


void UpdateTxT(void)
{
	char TrmsString[400];
	int j = 0;
	int i = 0;
	int xPos = 0;


	
	j = sprintf_s(TrmsString, 260, " , linecounter max is %u , ", GetLastMaxLines());
	j += sprintf_s(TrmsString + j, 260, " ISRTime: %u us", GetISRTime());

	
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
	j += sprintf_s(TrmsString + j, 260, " , err=%d , val=%d", ErrCnt, ErrVal);
#endif

	if (FFOvl(DRV) == TRUE) 	{ j += sprintf_s(TrmsString + j, 260, " , overflow! "); }
	else j += sprintf_s(TrmsString + j, 260, "                      ");
	TextOut(hMSDC, 20, YLENGTH + 50, TrmsString, j);

	RedrawWindow(hMSWND, NULL, NULL, RDW_INVALIDATE);
}

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

		CopytoDispbuf();
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

//ActMouse(DRV);
}//DisplayData



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
	PUSHORT pdata = pDIODEN;
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

	
	if (_ISPDA)	{ SetISPDA(DRV, TRUE); }
	else SetISPDA(DRV, FALSE);
	if (_ISFFT) { SetISFFT(DRV, TRUE); }
	else SetISFFT(DRV, FALSE);
	


	//SetTORReg(DRV, 0);  //XCK  
	SetTORReg(DRV,1);	//outtrig
	//SetTORReg(DRV, 2);  //FFREAD  geht : 20 microsec
	//SetTORReg(DRV, 3);  // area read
	//SetTORReg(DRV, 1);// 0);  // 
	// ohne displ : 9 microsec

	//!!!
	SendFLCAM(DRV, 1, 0x0, 1);	//reset

	//Version 2 ch byte
//-2	SendFLCAM(DRV,1, 0x28, 0x0000); //set to byte wise mode
//-2	SendFLCAM(DRV, 1, 0x46, 0x8409); //set to 2 wire mode & 14bit & MSB first

//		SendFLCAM(DRV,1, 0x26, 0x00b0); //set custom pattern D4=bit0 -> 10=1, 80=8, 800=80, f0=1111=15, fff=max
	//	SendFLCAM(DRV, 1, 0x25, 0x11); //set custom pattern with D0(=D12)+D1(=D13)=1 -> max=3fff=16383
//		SendFLCAM(DRV,1, 0x25, 0x10); //single custom pattern
//		SendFLCAM(DRV, 1, 0x42, 0x00); //clk align
	//  SendFLCAM(DRV,1, 0x25, 0x40); //ramp pattern

	//SetEC(DRV, 1000); //test EC in 10ns
	RSEC(DRV);

	gain = 6;
	SetADGain(DRV, 1, gain, gain, gain, gain, gain, gain, gain, gain); //set gain to values g1..g8 in Board.C
//	SetADGain(DRV, 1, 0, 0, 0, 0, 0, 0, 0, 0); //set gain to values g1..g8 in Board.C

	//stop all and clear FIFO
	StopFFTimer(DRV);
	SetIntFFTrig(DRV);
	RSFifo(DRV);

//startdma war hier mal
	if (!DMAAlreadyStarted){
		SetupPCIE_DMA(DRV);
		DMAAlreadyStarted = TRUE;
	}
	
	// write header
	j=sprintf_s(header,260," Online Loop - Cancel with ESC or space- key  " );
	TextOut(hMSDC,100,LOY-17,header,j);
	RedrawWindow(hMSWND,NULL,NULL,RDW_INVALIDATE);

//	SetupIR(DRV,0);


	// set slope for ext. trigger
	if (TrigMod==0)	HighSlope(DRV);
	if (TrigMod==1)	LowSlope(DRV);
	if (TrigMod==2)	BothSlope(DRV);

/*
	// set amplification if switchable
	if (HIAMP)	{V_On(DRV);}
	else {	V_Off(DRV);}
	
	if (_USESHUTTER) {OpenShutter(DRV);}
	//else CloseShutter(DRV);

	//setup for 16bit
	if (_AD16ADC)  {OpenShutter(DRV); // 16 bit needs openshutter
					CAL_AD(DRV,  ZADR); }


	if ((_FFTLINES>0)&&(!_HA_MODULE))
		SetupVCLKReg(DRV, _FFTLINES, (UCHAR) VFREQ);
*/


	Running=TRUE;
	UpdateDispl=FALSE;

	//StartPCIE_DMAWrite(DRV);

	//start 2nd thread for getting data in highest std priority, ring=200 lines
	if (HWINTR_EN)
		StartRingReadThread(DRV, 200, _THREADPRI, -1);
	else
		StartReadWithDma(DRV);
	//while (!RingThreadOn) {}; // wait until ReadThread is running

	//start thread to display data
	_beginthread( DisplayData, 2000, &hMSDC ); 


	//start hardware timer - must be the last
	if (EXTTRIGFLAG)
		 {SetExtFFTrig(DRV); }
	else
		{SetIntFFTrig(DRV);
		StartFFTimer(DRV, ExpTime); // in micro sec
		}

	}//Contimess







