/* simple window App for calling CCDlsc Driver via Jungo driver
*/

/*
This file is part of CCDExamp.

CCDExamp is free software : you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

CCDExamp is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar.If not, see < http://www.gnu.org/licenses/>.

Copyright 2020 Entwicklungsbuero G. Stresing (http://www.stresing.de/)
*/
#include "CCDExamp.h"


int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow )
{
	MSG      msg;

	if (!InitApplication( hInstance ))
		return FALSE;

	//make init here, that CCDExamp can be used to read the act regs...
	if (!SetBoardVars( choosen_board, aCAMCNT[choosen_board], _PIXEL ))
	{
		ErrorMsg( "Error in SetBoardVars" );
		return FALSE;
	}

	//show allocate buffer dialog before entering main application
	if(_ISFFT) DialogBox( hInstance, MAKEINTRESOURCE( IDD_SETFULLBIN ), hMSWND, (DLGPROC)FullBinning );
	else DialogBox( hInstance, MAKEINTRESOURCE( IDD_ALLOCBBUF ), hMSWND, (DLGPROC)AllocateBuf );
	if (!InitInstance( hInstance, nCmdShow ))
		return FALSE;
	//setup exposure
	DialogBox( hInstance, MAKEINTRESOURCE( IDD_EXPTIME ), hMSWND, (DLGPROC)SetupMeasure );

	while (GetMessage( &msg, NULL, 0, 0 ))
	{
		TranslateMessage( &msg );			// exit is the only message
		DispatchMessage( &msg );
	}
	return(msg.wParam);
}	// END WinMain

BOOL InitApplication( HINSTANCE hInstance )
{
	WNDCLASS wc;

	// Register the main application window class.
	//............................................
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = (WNDPROC)WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon( hInstance, lpszAppName );
	wc.hCursor = LoadCursor( NULL, IDC_ARROW );
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = lpszAppName;
	wc.lpszClassName = lpszAppName;

	if (!RegisterClass( &wc ))
		return(FALSE);

#ifdef _DLL
	NUMBER_OF_BOARDS = DLLCCDDrvInit( DRV );

	if (NUMBER_OF_BOARDS < 1)
	{
		DLLErrorMsg( " Can't open CCD driver " );
		return (FALSE);
	};

	if (!DLLInitBoard( DRV, _PIXEL, FLAG816, 0, aXCKDelay ))//pclk is not used
	{
		DLLErrorMsg( " Can't set Boardvars in intiboard " );
		return (FALSE);
	};
#else
	if (!CCDDrvInit())
	{
		ErrorMsg( " Can't open CCD driver " );
		return (FALSE);
	};

	if (!InitBoard( 1 ))
	{
		ErrorMsg( " Can't open first board " );
		return (FALSE);
	};
	if (number_of_boards >= 2)
		if (!InitBoard( 2 ))
		{
			ErrorMsg( " Can't open second board " );
			return (FALSE);
		}
#endif
	return TRUE;
}

BOOL InitInstance( HINSTANCE hInstance, int nCmdShow )
{
	HWND     hWnd;

	// Save the application-instance handle. 
	hInst = hInstance;

	//CreateWindowA(lpClassName, lpWindowName, dwStyle, x, y, \
	//	nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam)\
	// Create the main application window.
	//....................................
	hWnd = CreateWindow( lpszAppName,
		lpszTitle,
		WS_OVERLAPPEDWINDOW,
		(GetSystemMetrics( SM_CXSCREEN ) - (XLENGTH / XOFF + 40)) / 2, (GetSystemMetrics( SM_CYSCREEN ) - (YLENGTH + 220)) / 2,
		XLENGTH / XOFF + 40, YLENGTH + 220,//640/380,  
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (!hWnd) return(FALSE);

	//RSInterface(choosen_board);
	//	if (! InitBoard(choosen_board)) //Error message in InitBoard
	//		return (FALSE); 

	//set global handle for our window
	// must be outside the thread
	hMSWND = hWnd;
	hMSDC = GetDC( hMSWND );

	ShowWindow( hWnd, nCmdShow );
	UpdateWindow( hWnd );
	//	AboutDrv(choosen_board);		//shows driver version and Board ID

	// init high resolution counter 	
//	TPS = InitHRCounter();
//	if (TPS==0) return (FALSE);
	if (_ISFFT)
	{//set full binning as standard mode

//reset auto start in case of setting before
		ResetS0Bit( 0, 0x5, choosen_board ); // S0Addr_CTRLB = 0x5,
		ResetS0Bit( 1, 0x5, choosen_board ); // S0Addr_CTRLB = 0x5,
		ResetS0Bit( 2, 0x5, choosen_board ); // S0Addr_CTRLB = 0x5,
		//Triger stuff
		ResetS0Bit( 4, 0x5, choosen_board ); // S0Addr_CTRLB = 0x5,
		ResetS0Bit( 5, 0x5, choosen_board ); // S0Addr_CTRLB = 0x5,
		//Reset partial binning
		WriteLongS0( choosen_board, 0, 0x2C ); // S0Addr_ARREG = 0x2C,
		//vclks
		SetupVCLKReg( choosen_board, _FFTLINES, Vfreqini );
	}

	CloseShutter( choosen_board ); //set cooling  off

/*
	if (_AD16cds)  {//resets EC reg!
					InitCDS_AD(choosen_board, m_SHA,m_Amp,m_Ofs,m_TIgain);
					OpenShutter(choosen_board);	//IFC must be hi or EC would not work
					}
*/
	return TRUE;
}

BOOL RegisterWin95( CONST WNDCLASS* lpwc )
{
	WNDCLASSEX wcex;

	wcex.style = lpwc->style;
	wcex.lpfnWndProc = lpwc->lpfnWndProc;
	wcex.cbClsExtra = lpwc->cbClsExtra;
	wcex.cbWndExtra = lpwc->cbWndExtra;
	wcex.hInstance = lpwc->hInstance;
	wcex.hIcon = lpwc->hIcon;
	wcex.hCursor = lpwc->hCursor;
	wcex.hbrBackground = lpwc->hbrBackground;
	wcex.lpszMenuName = lpwc->lpszMenuName;
	wcex.lpszClassName = lpwc->lpszClassName;

	// Added elements for Windows 95.
	//...............................
	wcex.cbSize = sizeof( WNDCLASSEX );
	wcex.hIconSm = LoadImage( wcex.hInstance, lpwc->lpszClassName,
		IMAGE_ICON, 16, 16,
		LR_DEFAULTCOLOR );

	return RegisterClassEx( &wcex );
}

void AboutTiming( HWND hWnd )
{
	int j = 0;
	char fn[260];
	ULONG TDispus = 0;

#ifdef _DLL
	TDispus = DLLTickstous( TICKSDISP ); // in us
#else
	TDispus = Tickstous( TICKSDISP ); // in us
#endif
	j = sprintf_s( fn, 260, "Timing  \n" );
	//j+=sprintf(fn+j,"treadpix:\t\t%04d ns \n",TReadPix);
	j += sprintf_s( fn + j, 260, "tdisplay:\t\t%06d µs \n", TDispus );
	j += sprintf_s( fn + j, 260, "exp. time:\t\t%04d ms ", ExpTime );
	MessageBox( hWnd, fn, "time", MB_OK );
}

void AboutKeys( HWND hWnd )
{
	int j = 0;
#define s_size 1000
	char fn[s_size];
	j = sprintf_s( fn, s_size, "F- Keys  \n" );
	j += sprintf_s( fn + j, s_size, "F1 : timing info \n" );
	j += sprintf_s( fn + j, s_size, "F2 : activate cooling \n" );
	j += sprintf_s( fn + j, s_size, "F3 : deactivate cooling \n" );
	j += sprintf_s( fn + j, s_size, "F4 : check cool good? \n" );
	//j+=sprintf_s(fn+j,s_size,"F5 : send IR Setup \n");
	j += sprintf_s( fn + j, s_size, "F6 : start measure \n" );
	j += sprintf_s( fn + j, s_size, "F7 : high amp on \n" );
	j += sprintf_s( fn + j, s_size, "F8 : high amp off \n" );
	j += sprintf_s( fn + j, s_size, "arrow up : change X-scale span\n" );
	j += sprintf_s( fn + j, s_size, "arrow dn : change X-scale span\n" );
	j += sprintf_s( fn + j, s_size, "arrow up : change X-scale offset\n" );
	j += sprintf_s( fn + j, s_size, "arrow dn : change X-scale offset\n" );
	j += sprintf_s( fn + j, s_size, "shift + arrow up : change Y-scale span\n" );
	j += sprintf_s( fn + j, s_size, "shift + arrow dn : change Y-scale span\n" );
	j += sprintf_s( fn + j, s_size, "\n" );
	MessageBox( hWnd, fn, "time", MB_OK );
}

void AboutCFS( HWND hWnd )
{
	int i, j = 0;
	char fn[600];
	ULONG S0Data = 0;
	ULONG length = 0;
	ULONG BData;
	ULONG actpayload;

#ifndef _DLL
	if (!ReadLongIOPort( choosen_board, &S0Data, 0 ))
	{
		ErrorMsg( " BOARD not found! " );
		return;
	}
	/*
	j=sprintf(fn,"CFS - registers   \n");
	j+=sprintf(fn+j,"PCIE number of BARs = 0x%x\n",S0Data);

	ReadLongIOPort(choosen_board,&S0Data,1);
	j+=sprintf(fn+j,"BAR0 address = 0x%x\n",S0Data);
	ReadLongIOPort(choosen_board,&S0Data,2);
	j+=sprintf(fn+j,"BAR0 length 2 = 0x%x\n",S0Data);
	ReadLongIOPort(choosen_board,&S0Data,3);
	j+=sprintf(fn+j,"BAR1 address 3 = 0x%x\n",S0Data);
	ReadByteS0(choosen_board,&S0Data,8); //!B basties test
	j+=sprintf(fn+j,"BAR1 length 4 = 0x%x\n",S0Data);
	*/
	/* dev sn not implemented
	j=0;
	ReadLongIOPort(choosen_board,&BData,0x104);
	j+=sprintf(fn+j,"i=0x%x : 0x%.8x\n",0x104,BData);
	ReadLongIOPort(choosen_board,&BData,0x108);
	j+=sprintf(fn+j,"i=0x%x : 0x%.8x\n",0x108,BData);

	MessageBox(hWnd,fn,"Dev SN",MB_OK);
	*/
	j = 0;
	for (i = 0x0; i < 0x40; i = i + 4)
	{
		ReadLongIOPort( choosen_board, &BData, i );
		j += sprintf( fn + j, "i=0x%x : 0x%.8x\n", i, BData );
		//i+=3;
	}

	MessageBox( hWnd, fn, "Conf Space Header", MB_OK );

	for (j = 0, i = 0x40; i < 0x6c; i = i + 4)
	{
		ReadLongIOPort( choosen_board, &BData, i );
		j += sprintf( fn + j, "i=0x%x : 0x%.8x\n", i, BData );
	}
	MessageBox( hWnd, fn, "Conf Space ext. capabilities", MB_OK );
	//the following code have to be printed in binary code , not in hex

	j = 0;
	j += sprintf( fn + j, "PAY_LOAD values : 0 = 128 bytes, 1 = 256 bytes, 2 = 512 bytes\n" );
	ReadLongIOPort( choosen_board, &BData, 0x5C );//0x4c		
	j += sprintf( fn + j, "PAY_LOAD Supported : 0x%x\n", BData & 0x7 );

	//		WriteLongIOPort(choosen_board,0x2840,0x60);  not working  !! destroys PC? !!
	ReadLongIOPort( choosen_board, &BData, 0x60 );
	actpayload = (BData >> 5) & 0x7;
	j += sprintf( fn + j, "PAY_LOAD : 0x%x\n", actpayload );
	ReadLongIOPort( choosen_board, &BData, 0x60 );
	j += sprintf( fn + j, "MAX_READ_REQUEST_SIZE : 0x%x\n\n", (BData >> 12) & 0x7 );

	BData = aPIXEL[choosen_board];
	j += sprintf( fn + j, "pixel: %d \n", BData );

	switch (actpayload)
	{
	case 0: BData = 0x20;		break;
	case 1: BData = 0x40;		break;
	case 2: BData = 0x80;		break;
	case 3: BData = 0x100;		break;
	}

	j += sprintf( fn + j, "TLP_SIZE is: %d DWORDs = %d BYTEs\n", BData, BData * 4 );

	BData = (_PIXEL - 1) / (BData * 2) + 1;
	j += sprintf( fn + j, "number of TLPs should be: %d\n", BData );
	ReadLongDMA( choosen_board, &BData, 16 );
	j += sprintf( fn + j, "number of TLPs is: %d \n", BData );

	MessageBox( hWnd, fn, "DMA transfer payloads", MB_OK );

#else
	if (!DLLReadLongIOPort( choosen_board, &S0Data, 0 ))
	{
		DLLErrorMsg( " BOARD not found! " );
		return;
	}
	/*
	j=sprintf(fn,"CFS - registers   \n");
	j+=sprintf(fn+j,"PCIE number of BARs = 0x%x\n",S0Data);

	ReadLongIOPort(choosen_board,&S0Data,1);
	j+=sprintf(fn+j,"BAR0 address = 0x%x\n",S0Data);
	ReadLongIOPort(choosen_board,&S0Data,2);
	j+=sprintf(fn+j,"BAR0 length 2 = 0x%x\n",S0Data);
	ReadLongIOPort(choosen_board,&S0Data,3);
	j+=sprintf(fn+j,"BAR1 address 3 = 0x%x\n",S0Data);
	ReadByteS0(choosen_board,&S0Data,8); //!B basties test
	j+=sprintf(fn+j,"BAR1 length 4 = 0x%x\n",S0Data);
	*/
	/* dev sn not implemented
	j=0;
	ReadLongIOPort(choosen_board,&BData,0x104);
	j+=sprintf(fn+j,"i=0x%x : 0x%.8x\n",0x104,BData);
	ReadLongIOPort(choosen_board,&BData,0x108);
	j+=sprintf(fn+j,"i=0x%x : 0x%.8x\n",0x108,BData);

	MessageBox(hWnd,fn,"Dev SN",MB_OK);
	*/
	j = 0;
	for (i = 0x0; i < 0x40; i = i + 4)
	{
		DLLReadLongIOPort( choosen_board, &BData, i );
		j += sprintf( fn + j, "i=0x%x : 0x%.8x\n", i, BData );
		//i+=3;
	}

	MessageBox( hWnd, fn, "Conf Space Header", MB_OK );

	for (j = 0, i = 0x40; i < 0x6c; i = i + 4)
	{
		DLLReadLongIOPort( choosen_board, &BData, i );
		j += sprintf( fn + j, "i=0x%x : 0x%.8x\n", i, BData );
	}
	MessageBox( hWnd, fn, "Conf Space ext. capabilities", MB_OK );
	//the following code have to be printed in binary code , not in hex

	j = 0;
	j += sprintf( fn + j, "PAY_LOAD values : 0 = 128 bytes, 1 = 256 bytes, 2 = 512 bytes\n" );
	DLLReadLongIOPort( choosen_board, &BData, 0x5C );//0x4c		
	j += sprintf( fn + j, "PAY_LOAD Supported : 0x%x\n", BData & 0x7 );

	//		WriteLongIOPort(choosen_board,0x2840,0x60);  not working  !! destroys PC? !!
	DLLReadLongIOPort( choosen_board, &BData, 0x60 );
	actpayload = (BData >> 5) & 0x7;
	j += sprintf( fn + j, "PAY_LOAD : 0x%x\n", actpayload );
	DLLReadLongIOPort( choosen_board, &BData, 0x60 );
	j += sprintf( fn + j, "MAX_READ_REQUEST_SIZE : 0x%x\n\n", (BData >> 12) & 0x7 );

	BData = aPIXEL[choosen_board];
	j += sprintf( fn + j, "pixel: %d \n", BData );

	switch (actpayload)
	{
	case 0: BData = 0x20;		break;
	case 1: BData = 0x40;		break;
	case 2: BData = 0x80;		break;
	case 3: BData = 0x100;		break;
	}

	j += sprintf( fn + j, "TLP_SIZE is: %d DWORDs = %d BYTEs\n", BData, BData * 4 );

	BData = (_PIXEL - 1) / (BData * 2) + 1;
	j += sprintf( fn + j, "number of TLPs should be: %d\n", BData );
	DLLReadLongDMA( choosen_board, &BData, 16 );
	j += sprintf( fn + j, "number of TLPs is: %d \n", BData );

	MessageBox( hWnd, fn, "DMA transfer payloads", MB_OK );
#endif


}//AboutCFS

/*
void AboutS0(void)
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
		"EC \t",
		"TOR \t",
		"ARREG \t",
		"GIOREG \t",
		"DELAY EC/REG ",
		"IRQREG \t",
		"PCI board version",
		"R10 \t",
		"R11 \t",
		"R12 \t",
		"R13 DMABUFSIZE",
		"R14 SCANSPERINTR",
		"R15 SCANINDEX",
		"R16 \t",
		"R17 \t",
		"R18 \t",
		"R19 \t",
		"R1a \t",
		"R1b \t",
		"R1c \t",
		"R1d \t",
		"R1e \t ",
		"R1f \t"
	}; //Look-Up-Table for the S0 Registers
	//for debug
	//CleanupPCIE_DMA(choosen_board);
	if (!ReadLongIOPort(choosen_board, &S0Data, 0))
	{
		ErrorMsg(" BOARD not found! ");
		return;
	}
	hWnd = GetActiveWindow();
	j = sprintf(fn, "S0- registers   \n");
	//Hier werden alle 6 Adressen der BARs in Hex abgefragt
	//FIXME was soll dass denn? Falsche Schleife!
	/*for (i = 80; i<40; i = i + 4){
		ReadLongIOPort(choosen_board, &S0Data, i);
		if (S0Data != 0) numberOfBars++;
	}
	j += sprintf(fn + j, "PCIE number of BARs = %x\n", numberOfBars);
	for (i = 0; i <= 31; i++)
	{
		ReadLongS0(choosen_board, &S0Data, i * 4);
		j += sprintf(fn + j, "%s \t: 0x%I32x\n", LUTS0Reg[i], S0Data);
	}
	MessageBox(hWnd, fn, "S0 regs", MB_OK);
	}//AboutS0
	*/

void AboutDMA( HWND hWnd )
{
	int i, j = 0;
	int numberOfBars = 0;
	char fn[460];
	ULONG S0Data = 0;
	ULONG length = 0;
	char LUTDMAReg[18][20] = {
		"DCSR         ",
		"DDMACR  ",
		"WDMATLPA",
		"WDMATLPS",
		"WDMATLPC",
		"WDMATLPP",
		"RDMATLPP",
		"RDMATLPA",
		"RDMATLPS",
		"RDMATLPC",
		"WDMAPERF",
		"RDMAPERF",
		"RDMASTAT",
		"NRDCOMP",
		"RCOMPDSIZW",
		"DLWSTAT",
		"DLTRSSTAT",
		"DMISCCONT"
	}; //Look-Up-Table for the DMA Registers

	/*	if (! ReadLongIOPort(choosen_board,&S0Data,0))
	{
	ErrorMsg(" BOARD not found! ");
	return;
	}
	j=sprintf(fn,"DMA- registers   \n");

	//Hier werden alle 6 Adressen der BARs in Hex abgefragt
	for (i=16;i<40;i=i+4){
	ReadLongIOPort(choosen_board,&S0Data,i);
	if(S0Data != 0) numberOfBars++;
	}

	j+=sprintf(fn+j,"PCIE number of BARs = %x\n",numberOfBars);

	*/
	j = 0;
	for (i = 0; i <= 17; i++)
	{
#ifndef _DLL
		ReadLongDMA( choosen_board, &S0Data, i * 4 );
#else
		DLLReadLongDMA( choosen_board, &S0Data, i * 4 );
#endif

		j += sprintf( fn + j, "%s \t : 0x%x\n", LUTDMAReg[i], S0Data );
	}

	MessageBox( hWnd, fn, "DMA regs", MB_OK );
	/*
	j = 0;
	for (i=0;i<=0xffffffff;i++)
	{
	WriteLongDMA(choosen_board,i,0x14);
	ReadLongDMA(choosen_board,&S0Data,0x14);
	if(i != S0Data){
	j+=sprintf(fn+j,"Error: Readvalue is different to the writevalue : 0x%x\n",S0Data);
	break;
	}
	j+=sprintf(fn+j,"%s \t : 0x%x\n",LUTDMAReg[i/4],S0Data);
	}

	MessageBox(hWnd,fn,"DMA regs",MB_OK);
	*/
}//AboutDMA

void AboutPCI( HWND hWnd )
{
	int i, j = 0;
#define s_size 1000
	char fn[s_size];
	ULONG S0Data = 0;
	ULONG length = 0;

	j = sprintf_s( fn, s_size, "PCI - registers   \n" );

	//00-0f
	for (i = 0; i < 0x30; i++)
	{

#ifndef _DLL
		ReadLongIOPort( choosen_board, &S0Data, i * 4 );
#else
		DLLReadLongIOPort( choosen_board, &S0Data, i * 4 );
#endif
		j += sprintf_s( fn + j, s_size, "0x%x = 0x%x\n", i * 4, S0Data );
	}

	MessageBox( hWnd, fn, "PCI regs", MB_OK );
}//AboutPCI

LRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	int dummy;
	static HWND hText = NULL;
	static HWND hEdit = NULL;
	static HWND hBtn = NULL;
	UINT FFTMenuEnable;
	int i = 0;
	int span = 0;

	int trackbar_nob, trackbar_nospb, trackbar_nob_multiplier = 1, trackbar_nospb_multiplier = 1;
	char *s = (char*)malloc( 10 );

	char TrmsString[260];
	int j = 0;
	int xPos = GetCursorPosition();
	int yVal = DisplData[0][xPos];// YVal(1, xPos);

	switch (uMsg)
	{
	case WM_CREATE:
		//enable or disable fftmenu
		if (_ISFFT) FFTMenuEnable = MF_ENABLED;
		else FFTMenuEnable = MF_GRAYED;
		EnableMenuItem( GetMenu( hWnd ), ID_SETRANGEOFINTEREST_3RANGES, FFTMenuEnable );
		EnableMenuItem( GetMenu( hWnd ), ID_SETRANGEOFINTEREST_5RANGES, FFTMenuEnable );
		EnableMenuItem( GetMenu( hWnd ), ID_SETFULLBINNING, FFTMenuEnable );

		//if nos or nospb becomes a higher value then 30000 the gui is not posible to deisplay it
		//so we are checking this and dividing the displayed value. Therefore we are seeing a wrong value when we are using the trackbar
		trackbar_nospb = *Nospb;
		while (trackbar_nospb > 30000)
		{ //max for trackbar length
			trackbar_nospb /= 10;
			trackbar_nospb_multiplier *= 10;
		}
		trackbar_nob = Nob;
		while (trackbar_nob > 30000)
		{ //max for trackbar length
			trackbar_nob /= 10;
			trackbar_nob_multiplier *= 10;
		}

		hwndTrackNos = CreateWindow( TRACKBAR_CLASS,
			"NOS", WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_HORZ |
			TBS_TOOLTIPS | WS_TABSTOP | TBS_FIXEDLENGTH | TBM_SETBUDDY | WS_CAPTION,
			300, 345,
			400, 70,
			hWnd, (HMENU)ID_TRACKBAR,
			hInst,
			NULL );
		SendMessage( hwndTrackNos, TBM_SETRANGE, TRUE,
			MAKELONG( 0/*MIN RANGE*/, trackbar_nospb - 1/*MAX RANGE*/ ) );  //Optional, Default is 0-100

		hwndTrackNob = CreateWindow( TRACKBAR_CLASS,
			"NOB", WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_HORZ |
			TBS_TOOLTIPS | WS_TABSTOP | TBS_FIXEDLENGTH | TBM_SETBUDDY | WS_CAPTION,
			710, 345,
			400, 70,
			hWnd, (HMENU)ID_TRACKBAR,
			hInst,
			NULL );
		SendMessage( hwndTrackNob, TBM_SETRANGE, TRUE,
			MAKELONG( 0/*MIN RANGE*/, trackbar_nob - 1/*MAX RANGE*/ ) );  //Optional, Default is 0-100
		//ShowScrollBar(scrollb, SB_BOTH, TRUE);
		break;
	case WM_HSCROLL://ID_TRACKBAR:
		//Define your function.
		cur_nospb = SendMessage( hwndTrackNos, TBM_GETPOS, 0, 0 );
		cur_nob = SendMessage( hwndTrackNob, TBM_GETPOS, 0, 0 );
		cur_nospb *= trackbar_nospb_multiplier;
		cur_nob *= trackbar_nob_multiplier;
		CopytoDispbuf( cur_nob*(*Nospb) + cur_nospb );
		Display( 1, PLOTFLAG );
		UpdateTxT();
		/*
		//reset auto start in case of setting before
		ResetS0Bit(0, 0x5, choosen_board); // S0Addr_CTRLB = 0x5,
		ResetS0Bit(1, 0x5, choosen_board); // S0Addr_CTRLB = 0x5,
		ResetS0Bit(2, 0x5, choosen_board); // S0Addr_CTRLB = 0x5,
		//Reset partial binning
		WriteLongS0(choosen_board, 0, 0x2C); // S0Addr_ARREG = 0x2C,
		*/
		DLLShowNewBitmap( DRV, cur_nob, 0, _PIXEL, *Nospb );
		break;

	case WM_COMMAND:
		switch (LOWORD( wParam ))
		{
#ifndef _DLL
		case IDM_ABOUT:
			DialogBox( hInst, "AboutBox", hWnd, (DLGPROC)About );
			AboutDrv( choosen_board );
			break;

		case IDM_ABOUTTIME:
			AboutTiming( hWnd );
			break;

		case IDM_ABOUTKEYS:
			AboutKeys( hWnd );
			break;

		case IDM_ABOUTS0:
			AboutS0( choosen_board );
			break;

		case IDM_ABOUTDMA:
			AboutDMA( hWnd );
			break;

		case IDM_ABOUTCFS:
			AboutCFS( hWnd );
			break;
#else
		case IDM_ABOUT:
			DialogBox( hInst, "AboutBox", hWnd, (DLGPROC)About );
			DLLAboutDrv( choosen_board );
			break;

		case IDM_ABOUTTIME:
			AboutTiming( hWnd );
			break;

		case IDM_ABOUTKEYS:
			AboutKeys( hWnd );
			break;

		case IDM_ABOUTS0:
			DLLAboutS0( choosen_board );
			break;

		case IDM_ABOUTDMA:
			AboutDMA( hWnd );
			break;

		case IDM_ABOUTCFS:
			AboutCFS( hWnd );
			break;
#endif
		case IDM_START:
			contffloop = FALSE;
			cont_mode = FALSE;
			if (!Running) startMess( &dummy );
			break;
		case ID_START_STARTCONTINUOUSLY:
			contffloop = TRUE;
			cont_mode = TRUE;
			Nob = 1; 
			*Nospb = 10;
			CALLING_WITH_NOS = TRUE;
			CALLING_WITH_NOB = TRUE;
			if (_IsArea)
			{
				*Nospb = _FFTLINES;
				Nob = 3;
				DialogBox( hInst, MAKEINTRESOURCE( IDD_ALLOCBBUF ), hMSWND, (DLGPROC)AllocateBuf );
				SendMessage( hwndTrackNob, TBM_SETPOS, TRUE, 2 );
			}
			else
			{
				if (_IsROI)
				{
					*Nospb = _IsROI;
					Nob = 3;
					DialogBox( hInst, MAKEINTRESOURCE( IDD_ALLOCBBUF ), hMSWND, (DLGPROC)AllocateBuf );
					SendMessage( hwndTrackNob, TBM_SETPOS, TRUE, 2 );
					SendMessage( hwndTrackNos, TBM_SETPOS, TRUE, 1 );
				}
				else {//full binning
					DialogBox( hInst, MAKEINTRESOURCE( IDD_ALLOCBBUF ), hMSWND, (DLGPROC)AllocateBuf );
				}
			}
			if (!Running) startMess( &dummy );
			Sleep( 100 );
			while (Running)
			{
				CopytoDispbuf( cur_nob*(*Nospb) + cur_nospb );
				Display( 1, PLOTFLAG );
				UpdateTxT();
				DLLShowNewBitmap( DRV, cur_nob, 0, _PIXEL, *Nospb );
			}
			break;

		case IDM_SETEXP:
			DialogBox( hInst, MAKEINTRESOURCE( IDD_DIALOG1 ), hWnd, (DLGPROC)SetupMeasure );
			break;
		case IDM_SETTLEVEL:
			DialogBox( hInst, MAKEINTRESOURCE( IDD_SETTEMP ), hWnd, (DLGPROC)SetupTLevel );
			break;
		case IDM_SETEC:
			DialogBox( hInst, MAKEINTRESOURCE( IDD_SETEC ), hWnd, (DLGPROC)SetupEC );   //IDD_SETEC
			break;
		case ID_START_ALLOC:
			DialogBox( hInst, MAKEINTRESOURCE( IDD_ALLOCBBUF ), hWnd, (DLGPROC)AllocateBuf );//
			break;
		case ID_CHOOSEBOARD:
			DialogBox( hInst, MAKEINTRESOURCE( IDD_CHOOSEBOARD ), hWnd, (DLGPROC)ChooseBoard );
			break;
		case ID_SETRANGEOFINTEREST_3RANGES:
			DialogBox( hInst, MAKEINTRESOURCE( IDD_SETROI_3 ), hWnd, (DLGPROC)Set3ROI );
			break;
		case ID_SETRANGEOFINTEREST_5RANGES:
			DialogBox( hInst, MAKEINTRESOURCE( IDD_SETROI_5 ), hWnd, (DLGPROC)Set5ROI );
			break;
		case ID_SETFULLBINNING:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_SETFULLBIN), hWnd, (DLGPROC)FullBinning);
			break;
		case ID_AREAMODE:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_SETAREA), hWnd, (DLGPROC)AreaMode);
			break;
		case IDM_ShowTRMS: //invert state
			if (ShowTrms == TRUE)
			{
				ShowTrms = FALSE;
			}
			else
			{
				ShowTrms = TRUE;
			}
			break;
		case ID_2DVIEW_SHOW:
		{
			InitProDLL();
			DLLStart2dViewer( DRV, cur_nob, 0, _PIXEL, *Nospb );
			break;
		}
		case ID_2DVIEW_START:
		{
			cur_nob = 0;
			SendMessage( hwndTrackNob, TBM_SETPOS, TRUE, cur_nob );
			SendMessage( hMSWND, WM_HSCROLL, NULL, NULL );
			SetTimer( hMSWND,			// handle to main window 
				IDT_TIMER1,					// timer identifier 
				10,
				(TIMERPROC)NULL );		// no timer callback 
			break;
		}
		case ID_2DVIEW_SETGAMMA:
		{
			DialogBox( hInst, MAKEINTRESOURCE( IDD_SETGAMMA ), hWnd, (DLGPROC)SetGamma );
			break;
		}
		case IDM_EXIT:
			DestroyWindow( hWnd );
			break;
		}
		break;
	case WM_2DVIEWER_CLOSED:
		DLLDeinit2dViewer();
		break;
	case WM_TIMER:
		cur_nob++;
		if (cur_nob >= Nob)
		{
			KillTimer( hMSWND, IDT_TIMER1 );
			cur_nob--;
		}
		else
		{
			SendMessage( hwndTrackNob, TBM_SETPOS, TRUE, cur_nob );
			SendMessage( hMSWND, WM_HSCROLL, NULL, NULL );
		}
		break;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_F1:
			AboutTiming( hWnd );
			break;
		case VK_F6:
			if (!Running) startMess( &dummy );
			break;
#ifndef _DLL
		case VK_F2:
			break;
		case VK_F3:
			break;
		case VK_F4:
		{//check temp good
			int j = 0;
			char header[260];

			if (TempGood( choosen_board, 1 ) == TRUE)
			{
				j = sprintf_s( header, 260, " temp1 good        " );
			}
			else
				j = sprintf_s( header, 260, " temp1 not good " );

			if (TempGood( choosen_board, 2 ) == TRUE)
			{
				j += sprintf_s( header + j, 260, " temp2 good        " );
			}
			else
				j += sprintf_s( header + j, 260, " temp2 not good " );

			TextOut( hMSDC, 100, LOY + YLENGTH + 17, header, j );
			break;
		}
		case VK_F5: //send IR_Setup
		//SetupIR(choosen_board,1); //reset
			// RE&RS enable
		//WriteByteS0(choosen_board,0x0f,0x30);
			break;
		case VK_F7:
			break;
		case VK_F8:
			break;

			//case VK_SHIFT:
		case VK_UP: //change y-scale
			if (GetAsyncKeyState( VK_SHIFT )) { YSHIFT += 1; }
			else XOFF += 1;
			if (YSHIFT > 16) YSHIFT = 16;
			if (XOFF > _PIXEL / 600) XOFF = _PIXEL / 600;
			break;
		case VK_DOWN:
			if (GetAsyncKeyState( VK_SHIFT )) { YSHIFT -= 1; }
			else XOFF -= 1;
			if (YSHIFT < 0) YSHIFT = 0;
			if (XOFF < 1) XOFF = 1;
			break;
		case VK_RIGHT:
			/*XStart += 100;
			if (XStart>_PIXEL-XLENGTH*XOFF)
			XStart -= 100;*/
			PixelOdd = TRUE;
			break;
		case VK_LEFT:
			/* XStart -= 100;
				if (XStart<0) XStart=0; */
			PixelOdd = FALSE;
			break;
		case VK_ESCAPE: //stop measurement
		case VK_SPACE:
			Running = FALSE;
			//Sleep(20);
			//CleanupPCIE_DMA(choosen_board);
			//StopRingReadThread();
			StopSTimer( choosen_board );
			SetIntFFTrig( choosen_board );//disables ext. Trig.
			UpdateTxT();
			break;
		}
		break;
	case WM_MOUSEMOVE:
		if (contimess_run_once)
		{
			UpdateTxT();
		}
		break;
		/*case WM_PAINT:
		case WM_WINDOWPOSCHANGED  :
			ShowWindow(hWnd,SW_SHOW);
			//Display( 1,PLOTFLAG);
			break;
		*/
	case WM_DESTROY:
		//stop timer if it is still running
		Running = FALSE;
		Sleep( 20 ); // if the DMA Interrupt is running
		//CleanupPCIE_DMA(choosen_board);
		//StopRingReadThread();
		//board 1
		if (number_of_boards >= 2)
		{
			StopSTimer( 2 );
			SetIntFFTrig( 2 );//disables ext. Trig.
			CCDDrvExit( 2 );
		}
		StopSTimer( 1 );
		SetIntFFTrig( 1 );//disables ext. Trig.
		//WDC_DriverClose();
		CCDDrvExit( 1 );
		//board 2
		ReleaseDC( hMSWND, hMSDC );
		PostQuitMessage( 0 );
		break;
#else
		case VK_F2://switch cooling on
			DLLActCooling( choosen_board, TRUE );
			break;
		case VK_F3://switch cooling off
			DLLActCooling( choosen_board, FALSE );
			break;
		case VK_F4:
		{//check temp good
			int j = 0;
			char header[260];

			if (DLLTempGood( choosen_board, 1 ) == TRUE)
			{
				j = sprintf_s( header, 260, " temp1 good        " );
			}
			else
				j = sprintf_s( header, 260, " temp1 not good " );

			if (DLLTempGood( choosen_board, 2 ) == TRUE)
			{
				j += sprintf_s( header + j, 260, " temp2 good        " );
			}
			else
				j += sprintf_s( header + j, 260, " temp2 not good " );

			TextOut( hMSDC, 100, LOY + YLENGTH + 17, header, j );
			break;
		}
		case VK_F5: //send IR_Setup
					//SetupIR(choosen_board,1); //reset
					// RE&RS enable
					//WriteByteS0(choosen_board,0x0f,0x30);
			break;
		case VK_F7: //set high amp
			HIAMP = TRUE;
			DLLVOn( choosen_board );
			break;
		case VK_F8: //set low amp
			HIAMP = FALSE;
			DLLVOff( choosen_board );
			break;
			//case VK_SHIFT:
		case VK_UP: //change y-scale
			if (GetAsyncKeyState( VK_SHIFT )) { YSHIFT += 1; }
			else XOFF += 1;
			if (YSHIFT > 16) YSHIFT = 16;
			if (XOFF > _PIXEL / 600) XOFF = _PIXEL / 600;
			break;
		case VK_DOWN:
			if (GetAsyncKeyState( VK_SHIFT )) { YSHIFT -= 1; }
			else XOFF -= 1;
			if (YSHIFT < 0) YSHIFT = 0;
			if (XOFF < 1) XOFF = 1;
			break;
		case VK_RIGHT:
			/*XStart += 100;
			if (XStart>_PIXEL-XLENGTH*XOFF)
			XStart -= 100;*/
			PixelOdd = TRUE;
			break;
		case VK_LEFT:
			/* XStart -= 100;
			if (XStart<0) XStart=0; */
			PixelOdd = FALSE;
			break;
		case VK_ESCAPE: //stop measurement
		case VK_SPACE:
			Running = FALSE;
			//Sleep(20);
			//CleanupPCIE_DMA(choosen_board);
			DLLStopRingReadThread();
			DLLStopFFTimer( choosen_board );
			DLLSetIntTrig( choosen_board );//disables ext. Trig.
			UpdateTxT();
			break;
	}
		break;
	case WM_MOUSEMOVE:
		if (contimess_run_once)
		{
			UpdateTxT();
		}
		break;
		/*case WM_PAINT:
		case WM_WINDOWPOSCHANGED  :
			ShowWindow(hWnd,SW_SHOW);
			//Display( 1,PLOTFLAG);
			break;
			*/
	case WM_DESTROY:
		//stop timer if it is still running
		Running = FALSE;
		Sleep( 20 ); // if the DMA Interrupt is running
		//CleanupPCIE_DMA(choosen_board);
		//StopRingReadThread();
		//board 1

		if (NUMBER_OF_BOARDS >= 2)
		{
			DLLStopFFTimer( 2 );
			DLLSetIntTrig( 2 );//disables ext. Trig.
			//DLLCCDDrvExit(2);//check for two boards happens in the dll
		}

		DLLStopFFTimer( 1 );
		DLLSetIntTrig( 1 );//disables ext. Trig.
		//WDC_DriverClose();
		DLLCCDDrvExit( 1 );
		//board 2
		ReleaseDC( hMSWND, hMSDC );
		PostQuitMessage( 0 );
		break;
#endif
	default:
		return(DefWindowProc( hWnd, uMsg, wParam, lParam ));
	}
	return(0L);
}

LRESULT CALLBACK About( HWND hDlg,
	UINT message,
	WPARAM wParam,
	LPARAM lParam )
{
	switch (message)
	{
	case WM_INITDIALOG:
		return (TRUE);

	case WM_COMMAND:
		if (LOWORD( wParam ) == IDOK
			|| LOWORD( wParam ) == IDCANCEL)
		{
			EndDialog( hDlg, TRUE );
			return (TRUE);
		}
		break;
	}

	return (FALSE);
}

LRESULT CALLBACK SetupMeasure( HWND hDlg,
	UINT message,
	WPARAM wParam,
	LPARAM lParam )
{
	UINT val = 0;
	BOOL success = FALSE;
	TCHAR STI_inputs[5][10] =
	{
		TEXT( "I" ), TEXT( "S1" ), TEXT( "S2" ), TEXT( "S Timer" ),
		TEXT( "ASL" )
	};
	TCHAR BTI_inputs[5][10] =
	{
		TEXT( "I" ), TEXT( "S1" ), TEXT( "S2" ), TEXT( "S1 & S2" ),
		TEXT( "B Timer" )
	};
	TCHAR A[16];
	TCHAR B[16];
	int  k = 0;

	switch (message)
	{
	case WM_INITDIALOG:
		//for comboboxes:
		memset( &A, 0, sizeof( A ) );
		memset( &B, 0, sizeof( B ) );
		for (k = 0; k <= 4; k += 1)
		{
			strcpy_s( A, sizeof( A ) / sizeof( TCHAR ), (TCHAR*)STI_inputs[k] );
			strcpy_s( B, sizeof( B ) / sizeof( TCHAR ), (TCHAR*)BTI_inputs[k] );

			// Add string to combobox.
			SendMessage( GetDlgItem( hDlg, IDC_COMBO_STI ), (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)A );
			SendMessage( GetDlgItem( hDlg, IDC_COMBO_BTI ), (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)B );
		}
		// Send the CB_SETCURSEL message to display an initial item 
		//  in the selection field  
		SendMessage( GetDlgItem( hDlg, IDC_COMBO_STI ), CB_SETCURSEL, (WPARAM)ItemIndex_S, (LPARAM)0 );
		SendMessage( GetDlgItem( hDlg, IDC_COMBO_BTI ), CB_SETCURSEL, (WPARAM)ItemIndex_B, (LPARAM)0 );
		SetDlgItemInt( hDlg, IDC_M_EXPTIME, ExpTime, FALSE );
		SetDlgItemInt( hDlg, IDC_M_REPTIME, RepTime, FALSE );
		SetDlgItemInt( hDlg, IDC_SDAT, sdat, FALSE );
		SetDlgItemInt( hDlg, IDC_BDAT, bdat, FALSE );
		SetDlgItemInt( hDlg, IDC_SEC, sec, FALSE );
		SetDlgItemInt( hDlg, IDC_BEC, bec, FALSE );
		if (TrigMod == 0) CheckDlgButton( hDlg, IDC_RADIO1, BST_CHECKED );
		if (TrigMod == 1) CheckDlgButton( hDlg, IDC_RADIO2, BST_CHECKED );
		if (TrigMod == 2) CheckDlgButton( hDlg, IDC_RADIO3, BST_CHECKED );
		if (TrigMod_B == 1) CheckDlgButton( hDlg, IDC_RADIO1_B, BST_CHECKED );
		if (TrigMod_B == 0) CheckDlgButton( hDlg, IDC_RADIO2_B, BST_CHECKED );
		if (TrigMod_B == 2) CheckDlgButton( hDlg, IDC_RADIO3_B, BST_CHECKED );
		//if (!_MSHUT)//disable Reptime
			//EnableWindow( GetDlgItem( hDlg, IDC_M_REPTIME ), FALSE );
		return (TRUE);
		break;

	case WM_COMMAND:
		switch (LOWORD( wParam ))
		{
		case IDC_ExtTrig:
			//grayed exptime if exttrig is set
			SendMessage( GetDlgItem( hDlg, IDC_M_EXPTIME ), EM_SETREADONLY, IsDlgButtonChecked( hDlg, IDC_ExtTrig ), 0L );
			break;
		case IDC_M_REPTIME:
			//check if the summ of all roi is larger than fftlines 
			//and write message and deactivate the ok button
			if (_MSHUT)
			{
				val = GetDlgItemInt( hDlg, IDC_M_REPTIME, &success, FALSE );
				if (success) RepTime = val;
				if (RepTime < _MINREPTIME)
				{
					//write message
					SetWindowText( GetDlgItem( hDlg, REP_ERR_MESS ), "The Reptime limit is reached.\n Please increase Rep Time or change _MINREPTIME in the code!" );
					//disable ok button
					//EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
				}
				else
				{
					//unshow message
					SetWindowText( GetDlgItem( hDlg, REP_ERR_MESS ), "" );
					//enable ok button
					//EnableWindow(GetDlgItem(hDlg, IDOK), TRUE);
				}
			}
			break;
		case IDCANCEL:
			EndDialog( hDlg, TRUE );
			return (TRUE);
			break;

		case IDOK:
			
			//setting trigger input modes
			ItemIndex_S = SendMessage( GetDlgItem( hDlg, IDC_COMBO_STI ), (UINT)CB_GETCURSEL,
				(WPARAM)0, (LPARAM)0 );
			if(ItemIndex_S < 3)	SetSTI( choosen_board, ItemIndex_S);
			else				SetSTI( choosen_board, ItemIndex_S + 1 );
			ItemIndex_B = SendMessage( GetDlgItem( hDlg, IDC_COMBO_BTI ), (UINT)CB_GETCURSEL,
				(WPARAM)0, (LPARAM)0 );
			SetBTI( choosen_board, ItemIndex_B);
			
			//setting exp time and rep time
			val = GetDlgItemInt( hDlg, IDC_M_EXPTIME, &success, FALSE );
			if (success) ExpTime = val;
			SetSTimer( choosen_board, ExpTime );
			val = GetDlgItemInt( hDlg, IDC_M_REPTIME, &success, FALSE );
			if (success) RepTime = val;
			SetBTimer( choosen_board, RepTime * 1000 );

			//Setting DAT Registers
			val = GetDlgItemInt( hDlg, IDC_SDAT, &success, FALSE );
			if (success) sdat = val;
			if (sdat) SetSDAT( choosen_board, sdat );
			else ResetSDAT( choosen_board );
			val = GetDlgItemInt( hDlg, IDC_BDAT, &success, FALSE );
			if (success) bdat = val;
			if (sdat) SetBDAT( choosen_board, bdat );
			else ResetBDAT( choosen_board );

			//Setting EC Registers
			val = GetDlgItemInt( hDlg, IDC_SEC, &success, FALSE );
			if (success) sec = val;
			if (sdat) SetSEC( choosen_board, sec );
			else ResetSEC( choosen_board );
			val = GetDlgItemInt( hDlg, IDC_BEC, &success, FALSE );
			if (success) bec = val;
			if (sdat) SetBEC( choosen_board, bec );
			else ResetBEC( choosen_board );
			
			//setting slopes
			if (IsDlgButtonChecked( hDlg, IDC_RADIO1 ) == BST_CHECKED) TrigMod = 0;
			if (IsDlgButtonChecked( hDlg, IDC_RADIO2 ) == BST_CHECKED) TrigMod = 1;
			if (IsDlgButtonChecked( hDlg, IDC_RADIO3 ) == BST_CHECKED) TrigMod = 2;

			if (IsDlgButtonChecked( hDlg, IDC_RADIO1_B ) == BST_CHECKED) TrigMod_B = 1;
			if (IsDlgButtonChecked( hDlg, IDC_RADIO2_B ) == BST_CHECKED) TrigMod_B = 0;
			if (IsDlgButtonChecked( hDlg, IDC_RADIO3_B ) == BST_CHECKED) TrigMod_B = 2;

#ifndef _DLL
			if (TrigMod == 0)	HighSlope( choosen_board );
			if (TrigMod == 1)	LowSlope( choosen_board );
			if (TrigMod == 2)	BothSlope( choosen_board );
			SetBSlope( choosen_board, TrigMod_B );
#else
			if (TrigMod == 0)	DLLHighSlope( choosen_board );
			if (TrigMod == 1)	DLLLowSlope( choosen_board );
			if (TrigMod == 2)	DLLBothSlope( choosen_board );
#endif

			EndDialog( hDlg, TRUE );
			return (TRUE);
			break;
		}
		break; //WM_COMMAND

	}
	return (FALSE);
}


LRESULT CALLBACK AllocateBuf( HWND hDlg,
	UINT message,
	WPARAM wParam,
	LPARAM lParam )
{
	UINT	nob_input = 0,
		nospb_input = 0;
	BOOL success = FALSE;
	UINT64 builtinram, freeram, freeram_old, calcram, allocram;
	UINT divMB = 1024 * 1024;
	int trackbar_nob, trackbar_nospb, trackbar_nob_multiplier = 1, trackbar_nospb_multiplier = 1;

	switch (message)
	{
	case WM_INITDIALOG:
		SetDlgItemInt( hDlg, IDC_nob, Nob, FALSE );
		SetDlgItemInt( hDlg, IDC_nospb, *Nospb, FALSE );
		//set Nos to readonly if #this fundtion is called by Range Of Interest fundtion
		SendMessage( GetDlgItem( hDlg, IDC_nospb ), EM_SETREADONLY, CALLING_WITH_NOS, 0L );
		SendMessage( GetDlgItem( hDlg, IDC_nob ), EM_SETREADONLY, CALLING_WITH_NOB, 0L );
		CALLING_WITH_NOS = FALSE;
		CALLING_WITH_NOB = FALSE;
#ifndef _DLL
		FreeMemInfo( &builtinram, &freeram );
#else
		DLLFreeMemInfo( &builtinram, &freeram );
#endif
		SetDlgItemInt( hDlg, IDC_BUILTINRAM, builtinram / divMB, 0 );
		return (TRUE);
		break;

	case WM_COMMAND:
		switch (LOWORD( wParam ))
		{
		case IDCANCEL:
			EndDialog( hDlg, TRUE );
			return (TRUE);
			break;

		case IDOK:
			nob_input = GetDlgItemInt( hDlg, IDC_nob, &success, FALSE );
			nospb_input = GetDlgItemInt( hDlg, IDC_nospb, &success, FALSE );
			if (success)
			{
				Nob = nob_input;
				*Nospb = nospb_input;

#ifdef _DLL
				nDLLSetupDMA( DRV, *Nospb, Nob );
				if (both_boards)
					nDLLSetupDMA( 2, *Nospb, Nob );
#else
				if (!BufLock( choosen_board, CAMCNT ))
					MessageBox( hMSWND, "allocating Buffer fails", "Error", MB_OK );
				else
					MessageBox( hMSWND, "allocating Buffer succeeded", "Message", MB_OK );

				if (both_boards)
				{
					if (!BufLock( 2, CAMCNT ))
						MessageBox( hMSWND, "allocating Buffer of second Board fails", "Error", MB_OK );
					else
						MessageBox( hMSWND, "allocating Buffer of second Board succeeded", "Message", MB_OK );
				}
#endif
			}
			trackbar_nospb = *Nospb;
			while (trackbar_nospb > 30000)
			{ //max for trackbar length
				trackbar_nospb /= 10;
				trackbar_nospb_multiplier *= 10;
			}
			trackbar_nob = Nob;
			while (trackbar_nob > 30000)
			{ //max for trackbar length
				trackbar_nob /= 10;
				trackbar_nob_multiplier *= 10;
			}
			//update trackbars
			SendMessage( hwndTrackNob, TBM_SETRANGE, TRUE,
				MAKELONG( 0/*MIN RANGE*/, trackbar_nob - 1/*MAX RANGE*/ ) );  //Optional, Default is 0-100
			SendMessage( hwndTrackNos, TBM_SETRANGE, TRUE,
				MAKELONG( 0/*MIN RANGE*/, trackbar_nospb - 1/*MAX RANGE*/ ) );  //Optional, Default is 0-100
			EndDialog( hDlg, TRUE );
			return (TRUE);
			break;

		case IDCALC:
			nob_input = GetDlgItemInt( hDlg, IDC_nob, &success, FALSE );
			nospb_input = GetDlgItemInt( hDlg, IDC_nospb, &success, FALSE );
			if (success)
			{
				Nob = nob_input;
				*Nospb = nospb_input;
			}
			calcram = Nob * (*Nospb) * _PIXEL * sizeof( USHORT ) / divMB;
			if (calcram < 100000)
				SetDlgItemInt( hDlg, IDC_CALCRAM, calcram, 0 );
			else
				SetDlgItemText( hDlg, IDC_CALCRAM, "calculation error" );
			break;

		case IDALLOC:
			nob_input = GetDlgItemInt( hDlg, IDC_nob, &success, FALSE );
			nospb_input = GetDlgItemInt( hDlg, IDC_nospb, &success, FALSE );
			if (success)
			{
				Nob = nob_input;
				*Nospb = nospb_input;
#ifdef _DLL
				nDLLSetupDMA( DRV, *Nospb, Nob );
				if (both_boards)
					nDLLSetupDMA( 2, *Nospb, Nob );
			}
#else
				FreeMemInfo( &builtinram, &freeram );
				freeram_old = freeram;

				if (!BufLock( choosen_board, CAMCNT ))
					MessageBox( hMSWND, "allocating Buffer fails", "Error", MB_OK );
				else
					MessageBox( hMSWND, "allocating Buffer succeeded", "Message", MB_OK );
			}
			FreeMemInfo( &builtinram, &freeram );
			SetDlgItemInt( hDlg, IDC_FREERAM, freeram / divMB, 0 );
			SetDlgItemInt( hDlg, IDC_BUILTINRAM, builtinram / divMB, 0 );
			allocram = (freeram_old - freeram) / divMB;
			if (allocram < 100000)
				SetDlgItemInt( hDlg, IDC_ALLOCRAM, allocram, 0 );
			else//if RAM is bigger than 100gb
				SetDlgItemText( hDlg, IDC_ALLOCRAM, "calculation error" );
#endif
			break;
#ifdef _DLL
			nDLLSetupDMA( DRV, *Nospb, Nob );
			if (both_boards)
				nDLLSetupDMA( 2, *Nospb, Nob );
#else
			if (!BufLock( choosen_board, CAMCNT ))
				MessageBox( hMSWND, "allocating Buffer fails", "Error", MB_OK );
			else
				MessageBox( hMSWND, "allocating Buffer succeeded", "Message", MB_OK );
			if (both_boards)
			{
				if (!BufLock( 2, CAMCNT))
					MessageBox( hMSWND, "allocating Buffer of second Board fails", "Error", MB_OK );
				else
					MessageBox( hMSWND, "allocating Buffer of second Board succeeded", "Message", MB_OK );
			}
#endif
			trackbar_nospb = *Nospb;
			trackbar_nob = Nob;
			//update trackbars
			SendMessage( hwndTrackNob, TBM_SETRANGE, TRUE,
				MAKELONG( 0/*MIN RANGE*/, trackbar_nob - 1/*MAX RANGE*/ ) );  //Optional, Default is 0-100
			SendMessage( hwndTrackNos, TBM_SETRANGE, TRUE,
				MAKELONG( 0/*MIN RANGE*/, trackbar_nospb - 1/*MAX RANGE*/ ) );  //Optional, Default is 0-100
			EnableWindow( hwndTrackNos, FALSE );
			UpdateWindow( hwndTrackNos );
			EndDialog( hDlg, TRUE );
			return (TRUE);
			break;
		}
		break; //WM_COMMAND
	}
	return (FALSE);
}

LRESULT CALLBACK ChooseBoard( HWND hDlg,
	UINT message,
	WPARAM wParam,
	LPARAM lParam )
{
	UINT val = 0;
	BOOL success = FALSE;
	switch (message)
	{
	case WM_INITDIALOG:
		//if there is just one board initialized gray out board 2 option and both option
		if (number_of_boards < 2)
		{
			EnableWindow( GetDlgItem( hDlg, IDC_EC_RADIO2 ), FALSE );
			EnableWindow( GetDlgItem( hDlg, IDC_EC_RADIO_BOTH ), FALSE );
			CheckDlgButton( hDlg, IDC_EC_RADIO1, TRUE );
		}
		else
		{
			switch (choosen_board)
			{
			case 1:
				if (both_boards)
					CheckDlgButton( hDlg, IDC_EC_RADIO_BOTH, TRUE );
				else
				{
					CheckDlgButton( hDlg, IDC_EC_RADIO1, TRUE );
				}
				break;
			case 2:
				CheckDlgButton( hDlg, IDC_EC_RADIO2, TRUE );
				break; //EC
			}
		}
		return (TRUE);

	case WM_COMMAND:
		switch (LOWORD( wParam ))
		{
		case IDCANCEL:
			EndDialog( hDlg, TRUE );
			return (TRUE);
			break;

		case IDOK:
			if (IsDlgButtonChecked( hDlg, IDC_EC_RADIO1 ) == TRUE) { choosen_board = 1; both_boards = FALSE; DISP2 = FALSE; }
			if (IsDlgButtonChecked( hDlg, IDC_EC_RADIO2 ) == TRUE) { choosen_board = 2; both_boards = FALSE; DISP2 = FALSE; }
			if (IsDlgButtonChecked( hDlg, IDC_EC_RADIO_BOTH ) == TRUE) { choosen_board = 1; both_boards = TRUE; DISP2 = TRUE; }
			if (both_boards == TRUE)
#ifndef _DLL
				BOARD_SEL = 3;
#endif
			EndDialog( hDlg, TRUE );
			return (TRUE);
			break;
		}
		break; //WM_COMMAND
	}
	return (FALSE);
}

LRESULT CALLBACK SetupTLevel( HWND hDlg,
	UINT message,
	WPARAM wParam,
	LPARAM lParam )
{
	UINT val = 0;
	BOOL success = FALSE;
	switch (message)
	{
	case WM_INITDIALOG:
		SetDlgItemInt( hDlg, IDC_TLevel, TempLevel, FALSE );
		return (TRUE);
		break;

	case WM_COMMAND:
		switch (LOWORD( wParam ))
		{
		case IDCANCEL:
			EndDialog( hDlg, TRUE );
			return (TRUE);
			break;

		case IDOK:
			val = GetDlgItemInt( hDlg, IDC_TLevel, &success, FALSE );
			if (success)
			{
				TempLevel = val;
#ifndef _DLL
				SetTemp( choosen_board, (UCHAR)TempLevel );
			}
#else
				DLLSetTemp( choosen_board, (UCHAR)TempLevel );
		}
#endif
			EndDialog( hDlg, TRUE );
			return (TRUE);
			break;
		}
		break; //WM_COMMAND
	}
	return (FALSE);
}

LRESULT CALLBACK SetupEC( HWND hDlg,
	UINT message,
	WPARAM wParam,
	LPARAM lParam )
{
	UINT val = 0;
	BYTE dbyte = 0;
	UINT32 longval = 0;
	BOOL success = FALSE;
	TCHAR TOR_Outputs[13][14] =
	{
		TEXT("XCKI"), TEXT("Register"), TEXT("TOCNTO"), TEXT("XCKDelay"),
		TEXT("DMA Write Act"), TEXT("INTTRIGO"), TEXT("DATO"), TEXT("BTrigO"),
		TEXT("INTSRO"), TEXT("OPT1"), TEXT("OPT2"), TEXT("BlockOn"),
		TEXT("Measure On")
	};
	TCHAR A[16];
	int  k = 0;

	switch (message)
	{
	case WM_INITDIALOG:
		//for comboboxes:
		memset(&A, 0, sizeof(A));
		for (k = 0; k <= 12; k += 1)
		{
			strcpy_s(A, sizeof(A) / sizeof(TCHAR), (TCHAR*)TOR_Outputs[k]);

			// Add string to combobox.
			SendMessage(GetDlgItem(hDlg, IDC_COMBO_TOR), (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)A);
		}
		// Send the CB_SETCURSEL message to display an initial item 
		//  in the selection field  
		SendMessage(GetDlgItem(hDlg, IDC_COMBO_TOR), CB_SETCURSEL, (WPARAM)m_TOmodus, (LPARAM)0);
		SetDlgItemInt( hDlg, IDC_SETXDLY, tXDLY, FALSE );
		SetDlgItemInt( hDlg, IDC_SETTCNT, tTICNT, FALSE );
		SetDlgItemInt( hDlg, IDC_SETTCNT2, tTOCNT, FALSE );
		CheckDlgButton( hDlg, IDC_CHECK_NOPDARS, m_noPDARS );
		

		
		return (TRUE);
		break;

	case WM_COMMAND:
		switch (LOWORD( wParam ))
		{
		case IDCANCEL:
			EndDialog( hDlg, TRUE );
			return (TRUE);
			break;

		case IDOK:

			//setting  outputs TOR
			m_TOmodus = SendMessage(GetDlgItem(hDlg, IDC_COMBO_TOR), (UINT)CB_GETCURSEL,
				(WPARAM)0, (LPARAM)0);

			//			EXTTRIGFLAG= IsDlgButtonChecked(hDlg,IDC_ExtTrig);
						//get DAT value
			longval = GetDlgItemInt( hDlg, IDC_SETDAT, &success, FALSE );
			if (success)
			{
				tDAT = longval;
				longval = tDAT;
				if (longval != 0) longval |= 0x80000000;
#ifndef _DLL
				WriteLongS0( choosen_board, longval, 0x20 ); // DAT reg
#else
				DLLWriteLongS0( choosen_board, longval, 0x20 ); // DAT reg
#endif
			}
			//get XCKDLY val
			longval = GetDlgItemInt( hDlg, IDC_SETXDLY, &success, FALSE );
			if (success)
			{
				tXDLY = longval;
				longval = tXDLY;
				if (longval != 0) longval |= 0x80000000;
#ifndef _DLL
				WriteLongS0( choosen_board, longval, 0x24 ); // DAT reg
#else
				DLLWriteLongS0( choosen_board, longval, 0x24 ); // DAT reg
#endif
			}
			val = GetDlgItemInt( hDlg, IDC_SETTCNT, &success, FALSE );
			if (success) tTICNT = val;
			val = tTICNT;
			if (val > 1) { val -= 1; }
			else val = 0;
			if (val != 0) val |= 0x80;
			// devider n=1 -> n /2
#ifndef _DLL
			WriteByteS0( choosen_board, (BYTE)val, 0x28 );//TICNT reg
#else
			DLLWriteByteS0( choosen_board, (BYTE)val, 0x28 );//TICNT reg
#endif
			val = GetDlgItemInt( hDlg, IDC_SETTCNT2, &success, FALSE );
			if (success) tTOCNT = val;
			val = tTOCNT;
			if (val > 1) { val -= 1; }
			else val = 0;
			if (val != 0) val |= 0x80;
			// devider n=1 -> n /2
#ifndef _DLL
			WriteByteS0( choosen_board, (BYTE)val, 0x2A );//TOCNT reg
#else
			DLLWriteByteS0( choosen_board, (BYTE)val, 0x2A );//TOCNT reg
#endif
					

#ifndef _DLL
			RsTOREG( choosen_board );
			SetTORReg( choosen_board, m_TOmodus );
#else
			DLLRsTOREG( choosen_board );
			DLLSetTORReg( choosen_board, m_TOmodus );
#endif

			/*
			switch (m_TOmodus)
			{	case 1: dbyte = 0x0; break; //XCK
				case 2: dbyte = 0x80; break; //REG
				case 3: dbyte = 0x40; break; //EC
				case 4: dbyte = 0x08; break; //DAT
				case 5: d


					byte = 0x20; break; //TRIGIN
				case 6: dbyte = 0x10; break; //FFXCK
				case 7: dbyte |= 0x70; break; //Block Trig
				default:  dbyte = 0x0; // XCKI
			}

			m_noPDARS = IsDlgButtonChecked(hDlg,IDC_CHECK_NOPDARS);
			if (m_noPDARS) dbyte |= 0x04;

			WriteByteS0(choosen_board, dbyte,0x2B);//TOFLAG reg
			*/

			if (IsDlgButtonChecked( hDlg, IDC_ECCNT_RADIO1 ) == TRUE) m_ECmodus = 1; //CNT
			if (IsDlgButtonChecked( hDlg, IDC_ECCNT_RADIO2 ) == TRUE) m_ECmodus = 2;
			if (IsDlgButtonChecked( hDlg, IDC_ECCNT_RADIO3 ) == TRUE) m_ECmodus = 3;
			if (IsDlgButtonChecked( hDlg, IDC_ECCNT_RADIO4 ) == TRUE) m_ECmodus = 4;

			if (IsDlgButtonChecked( hDlg, IDC_RADIO11 ) == TRUE) { m_ECTrigmodus = 1; }
			if (IsDlgButtonChecked( hDlg, IDC_RADIO12 ) == TRUE) { m_ECTrigmodus = 2; }
			if (IsDlgButtonChecked( hDlg, IDC_RADIO13 ) == TRUE) { m_ECTrigmodus = 3; }
			if (IsDlgButtonChecked( hDlg, IDC_RADIO14 ) == TRUE) { m_ECTrigmodus = 4; }

			m_ECmodus = 1; //reset to timer mode
			m_ECTrigmodus = 1;
			//			OpenShutter(choosen_board); //EC works only if shutter open
			EndDialog( hDlg, TRUE );
			return (TRUE);
			break;
		} //WM_COMMAND	

	}//	   message
	return (FALSE);
}//SetupEC

LRESULT CALLBACK Set3ROI( HWND hDlg,
	UINT message,
	WPARAM wParam,
	LPARAM lParam )
{
#define ROI 3
	BOOL success = FALSE;
	//roi[0] = 12;

	switch (message)
	{
	case WM_INITDIALOG:
#if  ROI == 5
		SetDlgItemInt( hDlg, IDC_ROI_4, roi[3], FALSE );
		SetDlgItemInt( hDlg, IDC_ROI_3, roi[2], FALSE );
#endif
		SetDlgItemInt( hDlg, IDC_ROI_2, roi[1], FALSE );
		SetDlgItemInt( hDlg, IDC_ROI_1, roi[0], FALSE );
		return (TRUE);
		break;
	case WM_COMMAND:
		switch (LOWORD( wParam ))
		{
		case IDC_ROI_1:
		case IDC_ROI_2:
#if  ROI == 5
		case IDC_ROI_3:
		case IDC_ROI_4:
#endif
			roi[ROI - 1] = _FFTLINES;
			GetDlgItemInt( hDlg, IDC_ROI_1, &success, FALSE );
			if (success)roi[0] = GetDlgItemInt( hDlg, IDC_ROI_1, &success, FALSE );
			if (success) roi[1] = GetDlgItemInt( hDlg, IDC_ROI_2, &success, FALSE );
			if (ROI == 5)
			{
				if (success) roi[2] = GetDlgItemInt( hDlg, IDC_ROI_3, &success, FALSE );
				if (success) roi[3] = GetDlgItemInt( hDlg, IDC_ROI_4, &success, FALSE );
			}
			if (success)
				for (int i = 0; i < ROI - 1; i++)
					roi[ROI - 1] -= roi[i];
			//check if the summ of all roi is larger than fftlines 
			//and write message and deactivate the ok button
			if (roi[ROI - 1] > _FFTLINES)
			{
				//write message
				SetWindowText( GetDlgItem( hDlg, ROI_ERR_MESS ), "The sum of all ranges are larger than FFTLINES.\n Please correct it!" );
				//disable ok button
				EnableWindow( GetDlgItem( hDlg, IDOK ), FALSE );
			}
			else
			{
				//unshow message
				SetWindowText( GetDlgItem( hDlg, ROI_ERR_MESS ), "" );
				//enable ok button
				EnableWindow( GetDlgItem( hDlg, IDOK ), TRUE );
			}
#if  ROI == 5
			SetDlgItemInt( hDlg, IDC_ROI_5, roi[ROI - 1], FALSE );
#else
			SetDlgItemInt( hDlg, IDC_ROI_3, roi[ROI - 1], FALSE );
#endif
			break;

		case IDCANCEL:
			EndDialog( hDlg, TRUE );
			return (TRUE);
			break;

		case IDOK:
			_IsArea = FALSE;
			_IsROI = 5;
			for (int i = 0; i < ROI; i++)
			{
#ifndef _DLL
				SetupVPB( choosen_board, i + 1, roi[i], keep[i] );
#else
				DLLSetupVPB( choosen_board, i + 1, roi[i], keep[i] );
#endif
			}
#ifndef _DLL
			//Set auto start 
			SetS0Bit( 0, 0x5, choosen_board ); // S0Addr_CTRLB = 0x5,
			ResetS0Bit( 1, 0x5, choosen_board ); // S0Addr_CTRLB = 0x5,
			ResetS0Bit( 2, 0x5, choosen_board ); // S0Addr_CTRLB = 0x5,
			//int partial binning
			WriteLongS0( choosen_board, 0, 0x2C ); // S0Addr_ARREG = 0x2C,
			WriteLongS0( choosen_board, ROI, 0x2C ); // S0Addr_ARREG = 0x2C,
			SetS0Bit( 15, 0x2C, choosen_board );// S0Addr_ARREG = 0x2C,
			SetupVCLKReg( choosen_board, _FFTLINES, Vfreqini );
#else
			//Set auto start 
			DLLSetS0Bit( 0, 0x5, choosen_board ); // S0Addr_CTRLB = 0x5,
			DLLResetS0Bit( 1, 0x5, choosen_board ); // S0Addr_CTRLB = 0x5,
			DLLResetS0Bit( 2, 0x5, choosen_board ); // S0Addr_CTRLB = 0x5,
			//int partial binning
			DLLWriteLongS0( choosen_board, 0, 0x2C ); // S0Addr_ARREG = 0x2C,
			DLLWriteLongS0( choosen_board, ROI, 0x2C ); // S0Addr_ARREG = 0x2C,
			DLLSetS0Bit( 15, 0x2C, choosen_board );// S0Addr_ARREG = 0x2C,
#endif
				//allocate Buffer with matching NOS
			*Nospb = ROI;
			CALLING_WITH_NOS = TRUE;
			DialogBox( hInst, MAKEINTRESOURCE( IDD_ALLOCBBUF ), hMSWND, (DLGPROC)AllocateBuf );
			EndDialog( hDlg, TRUE );
			return (TRUE);
			break;
		}
		break; //WM_COMMAND
	}
	return (FALSE);
}

LRESULT CALLBACK Set5ROI( HWND hDlg,
	UINT message,
	WPARAM wParam,
	LPARAM lParam )
{
#define ROI 5
	BOOL success = FALSE;
	//roi[0] = 12;

	switch (message)
	{
	case WM_INITDIALOG:
		SetDlgItemInt( hDlg, IDC_ROI_4, roi[3], FALSE );
		SetDlgItemInt( hDlg, IDC_ROI_3, roi[2], FALSE );
		SetDlgItemInt( hDlg, IDC_ROI_2, roi[1], FALSE );
		SetDlgItemInt( hDlg, IDC_ROI_1, roi[0], FALSE );
		return (TRUE);
		break;
	case WM_COMMAND:
		switch (LOWORD( wParam ))
		{
		case IDC_ROI_1:
		case IDC_ROI_2:
#if  ROI == 5
		case IDC_ROI_3:
		case IDC_ROI_4:
#endif
			roi[ROI - 1] = _FFTLINES;
			GetDlgItemInt( hDlg, IDC_ROI_1, &success, FALSE );
			if (success)roi[0] = GetDlgItemInt( hDlg, IDC_ROI_1, &success, FALSE );
			if (success) roi[1] = GetDlgItemInt( hDlg, IDC_ROI_2, &success, FALSE );
			if (ROI == 5)
			{
				if (success) roi[2] = GetDlgItemInt( hDlg, IDC_ROI_3, &success, FALSE );
				if (success) roi[3] = GetDlgItemInt( hDlg, IDC_ROI_4, &success, FALSE );
			}
			if (success)
				for (int i = 0; i < ROI - 1; i++)
					roi[ROI - 1] -= roi[i];
			//check if the summ of all roi is larger than fftlines 
			//and write message and deactivate the ok button
			if (roi[ROI - 1] > _FFTLINES)
			{
				//write message
				SetWindowText( GetDlgItem( hDlg, ROI_ERR_MESS ), "The sum of all ranges are larger than FFTLINES.\n Please correct it!" );
				//disable ok button
				EnableWindow( GetDlgItem( hDlg, IDOK ), FALSE );
			}
			else
			{
				//unshow message
				SetWindowText( GetDlgItem( hDlg, ROI_ERR_MESS ), "" );
				//enable ok button
				EnableWindow( GetDlgItem( hDlg, IDOK ), TRUE );
			}
			SetDlgItemInt( hDlg, IDC_ROI_5, roi[ROI - 1], FALSE );//
			break;

		case IDCANCEL:
			EndDialog( hDlg, TRUE );
			return (TRUE);
			break;

		case IDOK:
			_IsArea = FALSE;
			_IsROI = 5;
			/*
			SetupVPB( choosen_board, 1, 5, FALSE );
			SetupVPB( choosen_board, 2, 25, TRUE );
			SetupVPB( choosen_board, 3, 5, FALSE );
			SetupVPB( choosen_board, 4, 25, TRUE );
			SetupVPB( choosen_board, 5, 5, FALSE );*/
			for (int i = 0; i < ROI; i++)
			{
#ifndef _DLL
				SetupVPB( choosen_board, i + 1, roi[i], keep[i] );
#else
				DLLSetupVPB( choosen_board, i + 1, roi[i], keep[i] );
#endif
			}
#ifndef _DLL
			//Set auto start 
			SetS0Bit( 0, 0x5, choosen_board ); // S0Addr_CTRLB = 0x5,
			ResetS0Bit( 1, 0x5, choosen_board ); // S0Addr_CTRLB = 0x5,
			ResetS0Bit( 2, 0x5, choosen_board ); // S0Addr_CTRLB = 0x5,
			//init partial binning
			WriteLongS0( choosen_board, 0, 0x2C ); // S0Addr_ARREG = 0x2C,
			WriteLongS0( choosen_board, 5, 0x2C ); // S0Addr_ARREG = 0x2C,
			SetS0Bit( 15, 0x2C, choosen_board );// S0Addr_ARREG = 0x2C,
			SetupVCLKReg( choosen_board, _FFTLINES, Vfreqini );
#else
			//Set auto start
			DLLSetS0Bit( 0, 0x5, choosen_board ); // S0Addr_CTRLB = 0x5,
			DLLResetS0Bit( 1, 0x5, choosen_board ); // S0Addr_CTRLB = 0x5,
			DLLResetS0Bit( 2, 0x5, choosen_board ); // S0Addr_CTRLB = 0x5,
			//int partial binning
			DLLWriteLongS0( choosen_board, 0, 0x2C ); // S0Addr_ARREG = 0x2C,
			DLLWriteLongS0( choosen_board, ROI, 0x2C ); // S0Addr_ARREG = 0x2C,
			DLLSetS0Bit( 15, 0x2C, choosen_board );// S0Addr_ARREG = 0x2C,
#endif
			//allocate Buffer with matching NOS
			*Nospb = ROI;
			CALLING_WITH_NOS = TRUE;
			DialogBox( hInst, MAKEINTRESOURCE( IDD_ALLOCBBUF ), hMSWND, (DLGPROC)AllocateBuf );
			EndDialog( hDlg, TRUE );
			return (TRUE);
			break;
		}
		break; //WM_COMMAND
	}
	return (FALSE);
}

LRESULT CALLBACK FullBinning( HWND hDlg,
	UINT message,
	WPARAM wParam,
	LPARAM lParam )
{
	switch (message)
	{
	case WM_INITDIALOG:
		return (TRUE);
		break;
	case WM_COMMAND:
		switch (LOWORD( wParam ))
		{
		case IDCANCEL:
			EndDialog( hDlg, TRUE );
			return (TRUE);
			break;

		case IDOK:
			_IsArea = FALSE;
			_IsROI = 0;
#ifndef _DLL
			//reset auto start in case of setting before
			ResetS0Bit( 0, 0x5, choosen_board ); // S0Addr_CTRLB = 0x5,
			ResetS0Bit( 1, 0x5, choosen_board ); // S0Addr_CTRLB = 0x5,
			ResetS0Bit( 2, 0x5, choosen_board ); // S0Addr_CTRLB = 0x5,
			//Triger stuff
			ResetS0Bit( 4, 0x5, choosen_board ); // S0Addr_CTRLB = 0x5,
			ResetS0Bit( 5, 0x5, choosen_board ); // S0Addr_CTRLB = 0x5,
			//Reset partial binning
			WriteLongS0( choosen_board, 0, 0x2C ); // S0Addr_ARREG = 0x2C,
			//vclks
			SetupVCLKReg( choosen_board, _FFTLINES, Vfreqini);
#else
			//reset auto start in case of setting before
			DLLResetS0Bit( 0, 0x5, choosen_board ); // S0Addr_CTRLB = 0x5,
			DLLResetS0Bit( 1, 0x5, choosen_board ); // S0Addr_CTRLB = 0x5,
			DLLResetS0Bit( 2, 0x5, choosen_board ); // S0Addr_CTRLB = 0x5,
			//int partial binning
			DLLWriteLongS0( choosen_board, 0, 0x2C ); // S0Addr_ARREG = 0x2C,#
			DLLSetupVCLK( choosen_board, _FFTLINES, 7 );
#endif
			DialogBox( hInst, MAKEINTRESOURCE( IDD_ALLOCBBUF ), hMSWND, (DLGPROC)AllocateBuf );
			EndDialog( hDlg, TRUE );
			return (TRUE);
			break;
		}
		break; //WM_COMMAND
	}
	return (FALSE);
}

LRESULT CALLBACK AreaMode(HWND hDlg,
	UINT message,
	WPARAM wParam,
	LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return (TRUE);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			EndDialog(hDlg, TRUE);
			return (TRUE);
			break;

		case IDOK:
			_IsArea = TRUE;
			_IsROI = 0;
#ifndef _DLL
			//vclks
			SetupVCLKReg( choosen_board, 1, Vfreqini );
			//set auto start
			SetS0Bit(0, 0x5, choosen_board); // S0Addr_CTRLB = 0x5,
			/*ResetS0Bit(1, 0x5, choosen_board); // S0Addr_CTRLB = 0x5,
			ResetS0Bit(2, 0x5, choosen_board); // S0Addr_CTRLB = 0x5,
			//Triger stuff
			ResetS0Bit(4, 0x5, choosen_board); // S0Addr_CTRLB = 0x5,
			ResetS0Bit(5, 0x5, choosen_board); // S0Addr_CTRLB = 0x5,
			*/
			//Reset partial binning
			//WriteLongS0(choosen_board, 0, 0x2C); // S0Addr_ARREG = 0x2C,
			ResetS0Bit( 15, 0x2C, choosen_board );
#else
			//set auto start 
			DLLSetS0Bit(0, 0x5, choosen_board); // S0Addr_CTRLB = 0x5,
			DLLResetS0Bit(1, 0x5, choosen_board); // S0Addr_CTRLB = 0x5,
			DLLResetS0Bit(2, 0x5, choosen_board); // S0Addr_CTRLB = 0x5,
			//int partial binning
			DLLWriteLongS0(choosen_board, 0, 0x2C); // S0Addr_ARREG = 0x2C,#
			DLLSetupVCLK(choosen_board, _FFTLINES, 7);
#endif
			//allocate Buffer with matching NOS
			*Nospb = _FFTLINES;
			CALLING_WITH_NOS = TRUE;
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ALLOCBBUF), hMSWND, (DLGPROC)AllocateBuf);
			EndDialog(hDlg, TRUE);
			return (TRUE);
			break;
		}
		break; //WM_COMMAND
	}
	return (FALSE);
}


LRESULT CALLBACK SetGamma( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	BOOL success = FALSE;

	switch (message)
	{
	case WM_INITDIALOG:
		//TODO: receive gamma values from pro dll to display them when opening dialog
		//if (Direct2dViewer)
		//{
		//	// receive gamma from direct 2d module & write to ccdexamp gamma variables
		//	direct2dviewer_gamma_white = Direct2dViewer_getGammaWhite( Direct2dViewer );
		//	direct2dviewer_gamma_black = Direct2dViewer_getGammaBlack( Direct2dViewer );
		//}
		//// set gamma to dialog box
		//SetDlgItemInt( hDlg, IDC_GAMMA_WHITE, direct2dviewer_gamma_white, FALSE );
		//SetDlgItemInt( hDlg, IDC_GAMMA_BLACK, direct2dviewer_gamma_black, FALSE );
		return (TRUE);
		break;
	case WM_COMMAND:
		switch (LOWORD( wParam ))
		{
		case IDCANCEL:
			EndDialog( hDlg, TRUE );
			return (TRUE);
			break;

		case IDOK:
			// receive gamma from dialog box & write to ccdexamp gamma variables
			GetDlgItemInt( hDlg, IDC_GAMMA_WHITE, &success, FALSE );
			if (success) direct2dviewer_gamma_white = GetDlgItemInt( hDlg, IDC_GAMMA_WHITE, &success, FALSE );
			if (success) direct2dviewer_gamma_black = GetDlgItemInt( hDlg, IDC_GAMMA_BLACK, &success, FALSE );
			DLLSetGammaValue( direct2dviewer_gamma_white, direct2dviewer_gamma_black );
			EndDialog( hDlg, TRUE );
			return (TRUE);
			break;

		case IDDEFAULT:
			// set gamma to dialog box
			switch (CAMERA_SYSTEM)
			{
			case camera_system_3001:
			case camera_system_3010:
				SetDlgItemInt( hDlg, IDC_GAMMA_WHITE, 0xFFFF, FALSE );
				break;
			case camera_system_3030:
				SetDlgItemInt( hDlg, IDC_GAMMA_WHITE, 0x3FFF, FALSE );
				break;
			}
			SetDlgItemInt( hDlg, IDC_GAMMA_BLACK, 0, FALSE );
		}
		break; //WM_COMMAND
	}
	return (FALSE);
}
