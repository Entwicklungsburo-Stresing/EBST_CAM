#include "CCDUnit.h"

BYTE Dispcnt = 0;
int yVal = 0;
ULONG ERRCNT = 0;

int YVal(unsigned long db, int pixeli)
{
	unsigned long val;
	ULONG xofs = 0;
	if ((PixelOdd) && (pixeli < (_PIXEL*CAMCNT))) { xofs += 1; }
	else xofs = 0;
	//	xofs=XStart;
	val = DisplData[db - 1][pixeli*XOFF + xofs];
	val = val >> YSHIFT;
	if (val > YLENGTH) val = YLENGTH;
	return val;
};

void Display(unsigned long db, BOOL Plot)
// Plot=TRUE -> dense 2ms; Plot=FALSE -> Dots=1ms; 
{
	long int i, y1, y2, val1, val2;
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

	Rectangle(hMSDC, LOX - 1, LOY - 1, LOX + xlength + 1, LOY + YLENGTH + 2);		// Clear
/*
	for (i=0;i<2;i++)//first pixels are not valid
			{
			//DisplData[db-1][i] = 0;
			DisplData[db][i] = 0;
			}
*/
	if (Plot)
		for (i = 0; i < xlength - 1; i++)
		{
			val1 = YVal(db, i);
			if (DISP2) val1 /= 2;
			val2 = YVal(db, i + 1);
			if (DISP2) val2 /= 2;
			y1 = LOY + YLENGTH - val1;
			y2 = LOY + YLENGTH - val2;
			MoveToEx(hMSDC, LOX + i, y1, NULL);
			LineTo(hMSDC, LOX + i + 1, y2);
		}
	else
		for (i = 0; i <= xlength - 1; i++)
		{
			val1 = YVal(db, i);
			if (DISP2) val1 /= 2;
			y2 = LOY + YLENGTH - val1;
			//SetPixelV(adc,LOX+i,y2,pencolor);
			SetPixelV(hMSDC, LOX + i, y2, pencolor);
		};

	if (DISP2) //display 2 graphics with db1 and db2 on top of each other
	{		//array is: dioden[db][i] and dioden[db+1][i]
		if (Plot)
			for (i = 0; i < xlength - 1; i++)
			{
				y1 = LOY + YLENGTH / 2 - YVal(db, i + _PIXEL) / 2;//y1 = LOY + YLENGTH/2 - YVal(db+1,i)/2 ;
				y2 = LOY + YLENGTH / 2 - YVal(db, i + 1 + _PIXEL) / 2;//y2 = LOY + YLENGTH/2 - YVal(db+1,i+1)/2;
				MoveToEx(hMSDC, LOX + i, y1, NULL);
				LineTo(hMSDC, LOX + i + 1, y2);
			}
		else
			for (i = 0; i <= xlength - 1; i++)
			{
				y2 = LOY + YLENGTH / 2 - YVal(db + 1, i) / 2;
				SetPixelV(hMSDC, LOX + i, y2, pencolor);
			};
	}
	return;
};

/**
\brief Copy camera data from current nos and nob frame to display buffer.
\param scan 
\param pmemory_free how much is free
\return none
*/
void CopytoDispbuf()
{
	ReturnFrame( choosen_board, cur_nos, cur_nob, 0, DisplData[0], _PIXEL );
	if (CAMCNT>1) ReturnFrame( choosen_board, cur_nos, cur_nob, 1, DisplData[0]+_PIXEL, _PIXEL );
	if(both_boards)
	{
		ReturnFrame( 2, cur_nos, cur_nob, 0, DisplData[1], _PIXEL );
		if (CAMCNT>1) ReturnFrame( 2, cur_nos, cur_nob, 1, DisplData[1] + _PIXEL, _PIXEL );
	}
	return;
}

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

	if (DisplData[0][1088 + 1000] != 989) ERRCNT += 1;
	if (ShowTrms)
	{
		//j=sprintf(TrmsString,"                                                           ") ;//clear old display
		//TextOut(hMSDC,20,YLENGTH + 50,TrmsString,j);
		j += sprintf_s( TrmsString + j, 260, " Trms of Pixel %lu CH1 is %.1f ", TRMSpix, TRMSval_global[0] );
		if (CAMCNT > 1) j += sprintf_s( TrmsString + j, 260, ", Trms of Pixel %lu CH2 is %.1f ", TRMSpix, TRMSval_global[1] );
		j += sprintf_s(TrmsString + j, 260, " -- scan: %lu, err: %lu         ", DisplData[0][5],ERRCNT);
		TextOut(hMSDC, 20, YLENGTH + 50, TrmsString, j);
	}

	/*B!if (FFOvl(choosen_board) == TRUE) 	{
		j += sprintf_s(TrmsString + j, 260, " , overflow! "); }
	else j += sprintf_s(TrmsString + j, 260, "                      ");
	*/
	TextOut(hMSDC, 20, YLENGTH + 50, TrmsString, j);
	RedrawWindow(hMSWND, NULL, NULL, RDW_INVALIDATE);
	return;
}

int GetCursorPosition()
{
	POINT CurPos;
	GetCursorPos(&CurPos);
	ScreenToClient(hMSWND, &CurPos);
	int x = CurPos.x - LOX;
	return (int)x;// CurPos.x;
}

void initCamera()
{
	switch (CAMERA_SYSTEM)
	{
	case camera_system_3001:
		InitCamera3001(DRV, _PIXEL, TRIGGER_MODE, SENSOR_TYPE, 0 );
		break;
	case camera_system_3010:
		InitCamera3010(DRV, _PIXEL, TRIGGER_MODE, ADC_MODE, ADC_CUSTOM_PATTERN, LED_ON, GAIN_HIGH);
		break;
	case camera_system_3030:
		InitCamera3030(DRV, ADC_MODE, ADC_CUSTOM_PATTERN, GAIN);
		break;
	}
	if (both_boards) {
		switch (CAMERA_SYSTEM)
		{
		case camera_system_3001:
			InitCamera3001(2, _PIXEL, TRIGGER_MODE, SENSOR_TYPE, _IsArea );
			break;
		case camera_system_3010:
			InitCamera3010(2, _PIXEL, TRIGGER_MODE, ADC_MODE, ADC_CUSTOM_PATTERN, LED_ON, GAIN_HIGH);
			break;
		case camera_system_3030:
			InitCamera3030(2, ADC_MODE, ADC_CUSTOM_PATTERN, GAIN);
			break;
		}
	}
}

void initMeasurement()
{
#ifdef _DLL
	DLLStopFFTimer(choosen_board);
	DLLRSFifo(choosen_board);
	gain = 6;
	DLLSetADGain(choosen_board, 1, gain, gain, gain, gain, gain, gain, gain, gain); //set gain to values g1..g8 in Board.C
	if (both_boards)
	{
		DLLStopFFTimer(2);
		DLLRSFifo(2);
		//setups
		//SetupDELAY(choosen_board,DELAYini);	//init WRFIFO delay
		DLLRsTOREG(2); // reset TOREG
		//set TrigOut, default= XCK
		DLLSetTORReg(2, m_TOmodus);
		gain = 6;
		DLLSetADGain(2, 1, gain, gain, gain, gain, gain, gain, gain, gain);
	}
#else
	//set PDA and FFT
	SetSensorType( choosen_board, SENSOR_TYPE );

	if (_MSHUT) {
		CloseShutter(choosen_board);
		SetSEC(choosen_board, ExpTime * 100);
		SetTORReg(choosen_board, 14); //use SHUT
	}
	else { 
		ResetSEC(choosen_board);
		SetTORReg(choosen_board, m_TOmodus);
	}
	//set TrigOut, default= XCK
	StopSTimer(choosen_board);
	RSFifo(choosen_board);
	initCamera();
	if (both_boards) {
		StopSTimer(2);
		SetIntFFTrig(2);
		RSFifo(2);
		//set TrigOut, default= XCK
		SetTORReg(2, m_TOmodus);
	}
#endif
}

unsigned int __stdcall UpdateDisplayThread( void *parg )//threadex
{
	while (Running)
	{
		UpdateDisplay();
		/*if (GetAsyncKeyState( VK_ESCAPE ))
			break;
		if (GetAsyncKeyState( VK_SPACE ))
			break;*/
		Sleep( 200 );
	}

	return 1;//endthreadex is called automatically when this returns
}

// main read loop setup
void startMess(void *dummy)
{
	int j = 0;
	char header[260];
	contimess_run_once = TRUE;
	initMeasurement();
	// write header
	if(cont_mode)
		j = sprintf_s(header, 260, " Continuous mode - Cancel with ESC or space- key                   ");
	else
		j = sprintf_s( header, 260, " One Shot mode - Cancel with ESC or space- key                    " );
	TextOut(hMSDC, 100, LOY - 17, header, j);
	RedrawWindow(hMSWND, NULL, NULL, RDW_INVALIDATE);
#ifdef _DLL
	if (both_boards)
		DLLReadFFLoop(choosen_board, ExpTime, EXTTRIGFLAG, 0, 0, 3);//if both cams are activated
	else
		DLLReadFFLoop(choosen_board, ExpTime, EXTTRIGFLAG, 0, 0, choosen_board);
#else
	IsrCounter = 0;
	if (both_boards)	params.board_sel = 3;
	else				params.board_sel = choosen_board;
	// start read loop
	_beginthreadex(0, 0, &ReadFFLoopThread, &params, 0, 0);
	DWORD64 IsrNumber = Nob * (*Nospb) / (DMA_BUFSIZEINSCANS / DMA_HW_BUFPARTS);
	if (both_boards) IsrNumber *= 2;
	if (CAMCNT == 2) IsrNumber *= 2;
	while (IsrCounter < IsrNumber) {
		if (cont_mode)
			j = sprintf_s( header, 260, " Continuous mode - Cancel with ESC or space- key isr: %i of %i  ", IsrCounter + 1, IsrNumber );//+1 cheating );
		else
			j = sprintf_s( header, 260, " One Shot mode - Cancel with ESC or space- key isr: %i of %i  ", IsrCounter + 1, IsrNumber );//+1 cheating );
		TextOut(hMSDC, 100, LOY - 17, header, j);
		RedrawWindow(hMSWND, NULL, NULL, RDW_INVALIDATE);
		if (GetAsyncKeyState( VK_ESCAPE )) break;
	}
#endif
	return;
}

void UpdateDisplay()
{

	CopytoDispbuf();
	Display( 1, PLOTFLAG );
	UpdateTxT();
	DLLShowNewBitmap( DRV, cur_nob, 0, _PIXEL, *Nospb );

	return;
}