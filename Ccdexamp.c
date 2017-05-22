/* simple window App for calling CCDlsc Driver via Jungo driver
	Version 1.0		© Entwicklungsbuero G. Stresing	1/16
*/

#include <windows.h> 
#include <stdlib.h> 
#include <conio.h>
#include <string.h>
#include <stdio.h>
#include <process.h>	  // for Thread example
#include <windows.h>
#include <CommCtrl.h>

#include "resource.h"
#include "BOARD.h"
#include "GLOBAL.h" 
#include "BOARD.C"
#include "CCDUNIT.C"

 

#if defined (WIN32)
	#define IS_WIN32 TRUE
#else
	#define IS_WIN32 FALSE
#endif

#define IS_NT      IS_WIN32 && (BOOL)(GetVersion() < 0x80000000)
#define IS_WIN32S  IS_WIN32 && (BOOL)(!(IS_NT) && (LOBYTE(LOWORD(GetVersion()))<4))
#define IS_WIN95   (BOOL)(!(IS_NT) && !(IS_WIN32S)) && IS_WIN32


HINSTANCE hInst;   // current instance

LPCTSTR lpszAppName  = "CCDEXAMP";
LPCTSTR lpszTitle    = "CCDEXAMP"; 

HWND     hwndTrack;
HWND     hwndTrack2;


DWORD cur_nospb = 0;
DWORD cur_nob = 0;

BOOL RegisterWin95( CONST WNDCLASS* lpwc );

int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
                      LPTSTR lpCmdLine, int nCmdShow)
{
   MSG      msg;
   HWND     hWnd; 
   WNDCLASS wc;


   // Register the main application window class.
   //............................................
   wc.style         = CS_HREDRAW | CS_VREDRAW;
   wc.lpfnWndProc   = (WNDPROC)WndProc;       
   wc.cbClsExtra    = 0;                      
   wc.cbWndExtra    = 0;                      
   wc.hInstance     = hInstance;              
   wc.hIcon         = LoadIcon( hInstance, lpszAppName ); 
   wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
   wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
   wc.lpszMenuName  = lpszAppName;              
   wc.lpszClassName = lpszAppName;              

   if ( IS_WIN95 )
   {
      if ( !RegisterWin95( &wc ) )
         return( FALSE );
   }
   else if ( !RegisterClass( &wc ) )
      return( FALSE );


  if (! CCDDrvInit()) 
		{ErrorMsg(" Can't open CCD driver ");
		return (FALSE);
		};


  if (!InitBoard(1))
  {
	  ErrorMsg(" Can't open first board ");
	  return (FALSE);
  };
  if (number_of_boards >= 2)
	  if (!InitBoard(2))
	  {
		  ErrorMsg(" Can't open second board ");
		  return (FALSE);
	  }
 

   hInst = hInstance; 
  DialogBox(hInst, MAKEINTRESOURCE(IDD_ALLOCBBUF), hMSWND, (DLGPROC)AllocateBuf);

   // Create the main application window.
   //....................................
   hWnd = CreateWindow( lpszAppName, 
                        lpszTitle,    
                        WS_OVERLAPPEDWINDOW, 
                        CW_USEDEFAULT, 0, 
                        XLENGTH/XOFF+40 , YLENGTH + 220,//640/380,  
                        NULL,              
                        NULL,              
                        hInstance,         
                        NULL               
                      );

   if ( !hWnd )  return( FALSE );

 
//RSInterface(choosen_board);

//	if (! InitBoard(choosen_board)) //Error message in InitBoard
//		return (FALSE); 

 

	//set global handle for our window
	// must be outside the thread
	hMSWND = hWnd;
	hMSDC = GetDC(hMSWND) ;

	ShowWindow( hWnd, nCmdShow ); 
	UpdateWindow( hWnd );
//	AboutDrv(choosen_board);		// shows driver version and Board ID


	// init high resolution counter 	
//	TPS = InitHRCounter();
//	if (TPS==0) return (FALSE);





/*
	CloseShutter(choosen_board); //set cooling  off

	if (_ISPDA)	{SetISPDA(choosen_board, TRUE); } else SetISPDA(choosen_board, FALSE);
	if (_ISFFT) {SetISFFT(choosen_board, TRUE); } else SetISFFT(choosen_board, FALSE);

	if (_AD16cds)  {//resets EC reg!
					InitCDS_AD(choosen_board, m_SHA,m_Amp,m_Ofs,m_TIgain);
					OpenShutter(choosen_board);	//IFC must be hi or EC would not work				
					}
*/
   while( GetMessage( &msg, NULL, 0, 0) )   
   {
      TranslateMessage( &msg );			// exit is the only message
      DispatchMessage( &msg );  
   }

   return( msg.wParam ); 
}	// END WinMain


BOOL RegisterWin95( CONST WNDCLASS* lpwc )
{
   WNDCLASSEX wcex;

   wcex.style         = lpwc->style;
   wcex.lpfnWndProc   = lpwc->lpfnWndProc;
   wcex.cbClsExtra    = lpwc->cbClsExtra;
   wcex.cbWndExtra    = lpwc->cbWndExtra;
   wcex.hInstance     = lpwc->hInstance;
   wcex.hIcon         = lpwc->hIcon;
   wcex.hCursor       = lpwc->hCursor;
   wcex.hbrBackground = lpwc->hbrBackground;
   wcex.lpszMenuName  = lpwc->lpszMenuName;
   wcex.lpszClassName = lpwc->lpszClassName;

   // Added elements for Windows 95.
   //...............................
   wcex.cbSize = sizeof(WNDCLASSEX);
   wcex.hIconSm = LoadImage(wcex.hInstance, lpwc->lpszClassName, 
                            IMAGE_ICON, 16, 16,
                            LR_DEFAULTCOLOR );
			
   return RegisterClassEx( &wcex );
}

void AboutTiming(HWND hWnd)
{
int j=0;
char fn[260];
ULONG TDispus=0;

TDispus = Tickstous(TICKSDISP); // in us

j=sprintf_s(fn,260,"Timing  \n");
//j+=sprintf(fn+j,"treadpix:\t\t%04d ns \n",TReadPix);
j+=sprintf_s(fn+j,260,"tdisplay:\t\t%06d µs \n",TDispus);
j+=sprintf_s(fn+j,260,"exp. time:\t\t%04d ms ",ExpTime);
MessageBox(hWnd,fn,"time",MB_OK);	
}


void AboutKeys(HWND hWnd)
{
int j=0;
#define s_size 1000
char fn[s_size];


j=sprintf_s(fn,s_size,"F- Keys  \n");
j+=sprintf_s(fn+j,s_size,"F1 : timing info \n");
j+=sprintf_s(fn+j,s_size,"F2 : activate cooling \n");
j+=sprintf_s(fn+j,s_size,"F3 : deactivate cooling \n");
j+=sprintf_s(fn+j,s_size,"F4 : check cool good? \n");
//j+=sprintf_s(fn+j,s_size,"F5 : send IR Setup \n");
j+=sprintf_s(fn+j,s_size,"F6 : start measure \n");
j+=sprintf_s(fn+j,s_size,"F7 : high amp on \n");
j+=sprintf_s(fn+j,s_size,"F8 : high amp off \n");
j+=sprintf_s(fn+j,s_size,"arrow up : change X-scale span\n");
j+=sprintf_s(fn+j,s_size,"arrow dn : change X-scale span\n");
j+=sprintf_s(fn+j,s_size,"arrow up : change X-scale offset\n");
j+=sprintf_s(fn+j,s_size,"arrow dn : change X-scale offset\n");
j+=sprintf_s(fn+j,s_size,"shift + arrow up : change Y-scale span\n");
j+=sprintf_s(fn+j,s_size,"shift + arrow dn : change Y-scale span\n");
j+=sprintf_s(fn+j,s_size,"\n");
MessageBox(hWnd,fn,"time",MB_OK);	
}




void AboutCFS(HWND hWnd)
{
	int i, j = 0;
	char fn[600];
	ULONG S0Data = 0;
	ULONG length = 0;
	ULONG BData;
	ULONG actpayload;


	if (!ReadLongIOPort(choosen_board, &S0Data, 0))
	{
		ErrorMsg(" BOARD not found! ");
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
	for (i = 0x0; i<0x40; i = i + 4)
	{
		ReadLongIOPort(choosen_board, &BData, i);
		j += sprintf(fn + j, "i=0x%x : 0x%.8x\n", i, BData);
		//i+=3;
	}

	MessageBox(hWnd, fn, "Conf Space Header", MB_OK);

	for (j = 0, i = 0x40; i<0x6c; i = i + 4)
	{
		ReadLongIOPort(choosen_board, &BData, i);
		j += sprintf(fn + j, "i=0x%x : 0x%.8x\n", i, BData);
	}
	MessageBox(hWnd, fn, "Conf Space ext. capabilities", MB_OK);
	//the following code have to be printed in binary code , not in hex

	j = 0;
	j += sprintf(fn + j, "PAY_LOAD values : 0 = 128 bytes, 1 = 256 bytes, 2 = 512 bytes\n");
	ReadLongIOPort(choosen_board, &BData, 0x5C);//0x4c		
	j += sprintf(fn + j, "PAY_LOAD Supported : 0x%x\n", BData & 0x7);

	//		WriteLongIOPort(choosen_board,0x2840,0x60);  not working  !! destroys PC? !!
	ReadLongIOPort(choosen_board, &BData, 0x60);
	actpayload = (BData >> 5) & 0x7;
	j += sprintf(fn + j, "PAY_LOAD : 0x%x\n", actpayload);
	ReadLongIOPort(choosen_board, &BData, 0x60);
	j += sprintf(fn + j, "MAX_READ_REQUEST_SIZE : 0x%x\n\n", (BData >> 12) & 0x7);

	BData = aPIXEL[choosen_board];
	j += sprintf(fn + j, "pixel: %d \n", BData);

	switch (actpayload)
	{
	case 0: BData = 0x20;		break;
	case 1: BData = 0x40;		break;
	case 2: BData = 0x80;		break;
	case 3: BData = 0x100;		break;
	}

	j += sprintf(fn + j, "TLP_SIZE is: %d DWORDs = %d BYTEs\n", BData, BData * 4);

	BData = (_PIXEL - 1) / (BData * 2) + 1;
	j += sprintf(fn + j, "number of TLPs should be: %d\n", BData);
	ReadLongDMA(choosen_board, &BData, 16);
	j += sprintf(fn + j, "number of TLPs is: %d \n", BData);

	MessageBox(hWnd, fn, "DMA transfer payloads", MB_OK);


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
	*/

/*
	for (i = 0; i <= 31; i++)
	{
		ReadLongS0(choosen_board, &S0Data, i * 4);
		j += sprintf(fn + j, "%s \t: 0x%I32x\n", LUTS0Reg[i], S0Data);
	}

	MessageBox(hWnd, fn, "S0 regs", MB_OK);

}//AboutS0
*/

void AboutDMA(HWND hWnd)
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
		ReadLongDMA(choosen_board, &S0Data, i * 4);
		j += sprintf(fn + j, "%s \t : 0x%x\n", LUTDMAReg[i], S0Data);
	}

	MessageBox(hWnd, fn, "DMA regs", MB_OK);
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




void AboutPCI(HWND hWnd)
{
int i,j=0;
#define s_size 1000
char fn[s_size];
ULONG S0Data=0;
ULONG length=0;

j=sprintf_s(fn,s_size,"PCI - registers   \n");

	//00-0f
	for (i=0; i<0x30;i++)
		{
		ReadLongIOPort(choosen_board,&S0Data,i*4);
		j+=sprintf_s(fn+j,s_size,"0x%x = 0x%x\n",i*4,S0Data);	
		}

MessageBox(hWnd,fn,"PCI regs",MB_OK);	
}//AboutPCI



LRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
		int dummy;
static HWND hText = NULL;
static HWND hEdit = NULL;
static HWND hBtn  = NULL;
int i=0;
int span=0;

int trackbar_nob, trackbar_nospb, trackbar_nob_multiplier = 1, trackbar_nospb_multiplier = 1;
char *s = (char*)malloc(10);


char TrmsString[260];
int j = 0;
int xPos = GetCursorPosition();
int yVal = DisplData[0][xPos];// YVal(1, xPos);

   switch ( uMsg ) 
   {
   case WM_CREATE:
	   //if nos or nospb becomes a higher value then 30000 the gui is not posible to deisplay it
	   //so we are checking this and dividing the displayed value. Therefore we are seeing a wrong value when we are using the trackbar

	   trackbar_nospb = Nospb;
	   while(trackbar_nospb > 30000){ //max for trackbar length
		   trackbar_nospb /= 10;
		   trackbar_nospb_multiplier *= 10;
	   }
	   trackbar_nob = Nob;
	   while (trackbar_nob > 30000){ //max for trackbar length
		   trackbar_nob /= 10;
		   trackbar_nob_multiplier *= 10;
	   }

	   hwndTrack = CreateWindow(TRACKBAR_CLASS,
		   "NOS", WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_HORZ |
		   TBS_TOOLTIPS | WS_TABSTOP | TBS_FIXEDLENGTH | TBM_SETBUDDY | WS_CAPTION,
		   300, 345,
		   400, 70,
		   hWnd, (HMENU)ID_TRACKBAR,
		   hInst,
		   NULL);
	   SendMessage(hwndTrack, TBM_SETRANGE, TRUE,
		   MAKELONG(0/*MIN RANGE*/, trackbar_nospb - 1/*MAX RANGE*/));  //Optional, Default is 0-100

	   hwndTrack2 = CreateWindow(TRACKBAR_CLASS,
		   "NOB", WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_HORZ |
		   TBS_TOOLTIPS | WS_TABSTOP | TBS_FIXEDLENGTH | TBM_SETBUDDY | WS_CAPTION,
		   710, 345,
		   400, 70,
		   hWnd, (HMENU)ID_TRACKBAR,
		   hInst,
		   NULL);
	   SendMessage(hwndTrack2, TBM_SETRANGE, TRUE,
		   MAKELONG(0/*MIN RANGE*/, trackbar_nob - 1/*MAX RANGE*/));  //Optional, Default is 0-100
	   //ShowScrollBar(scrollb, SB_BOTH, TRUE);
	   break;
   case WM_HSCROLL://ID_TRACKBAR:
	   //Define your function.

	   cur_nospb = SendMessage(hwndTrack, TBM_GETPOS, 0, 0);
	   cur_nob = SendMessage(hwndTrack2, TBM_GETPOS, 0, 0);
	   cur_nospb *= trackbar_nospb_multiplier;
	   cur_nob *= trackbar_nob_multiplier;
	   CopytoDispbuf(cur_nob*cur_nospb + cur_nospb);
	   Display(1, PLOTFLAG);

	   UpdateTxT();
	   //j = sprintf_s(TrmsString + j, 260, " x: %i y: %i=0x%x ", xPos, yVal, yVal);
	   //TextOut(hMSDC, 20, YLENGTH + 50, TrmsString, j);

	   //sprintf(s, "%d", dwPos);
	   //MessageBox(hMSWND, s, "Position", MB_OK);
	   break;

      case WM_COMMAND :
              switch( LOWORD( wParam ) )
              {

                 case IDM_ABOUT :
				 		DialogBox( hInst, "AboutBox", hWnd, (DLGPROC)About );
						AboutDrv(choosen_board);
						break;

                 case IDM_ABOUTTIME :
					    AboutTiming(hWnd);
						break;

                 case IDM_ABOUTKEYS :
					    AboutKeys(hWnd);
						break;

	             case IDM_ABOUTS0 :
					    AboutS0(choosen_board);
						break;

				 case IDM_ABOUTDMA:
					 AboutDMA(hWnd);
					 break;


				 case IDM_ABOUTCFS:
					 AboutCFS(hWnd);
					 break;

				 case IDM_START :
						if (! Running) Contimess(&dummy);
						break;

				 case IDM_SETEXP :
				 		DialogBox( hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, (DLGPROC)SetupMeasure);
						break;

				 case IDM_SETTLEVEL :
				 		DialogBox( hInst, MAKEINTRESOURCE(IDD_SETTEMP), hWnd, (DLGPROC)SetupTLevel);
						break;
	
				 case IDM_SETEC :
					 DialogBox(hInst, MAKEINTRESOURCE(IDD_SETEC), hWnd, (DLGPROC)SetupEC);   //IDD_SETEC
						break;	
				 case ID_CHOOSEBOARD :
					 DialogBox(hInst, MAKEINTRESOURCE(IDD_CHOOSEBOARD), hWnd, (DLGPROC)ChooseBoard);
					 break;



//!!!
				 
//				 case IDM_SETAD:
	//				 DialogBox(hInst, MAKEINTRESOURCE(IDD_SETUPAD), hWnd, (DLGPROC)SetupEC);
		//			 break;

				case IDM_ShowTRMS : //invert state
					 if (ShowTrms==TRUE) 
						{ShowTrms=FALSE;}
					   else
						{ShowTrms=TRUE;}
						break;

				case ID_START_ALLOC :
					DialogBox(hInst, MAKEINTRESOURCE(IDD_ALLOCBBUF), hMSWND, (DLGPROC)AllocateBuf);
					break;

				 case IDM_EXIT :

                        DestroyWindow( hWnd );
                        break;
			  }
              break;


		case WM_KEYDOWN: 
              switch( wParam ) 
              {
 				 case VK_F1:
					AboutTiming(hWnd);
					break;
				 case VK_F6:
					if (! Running) Contimess(&dummy);
                    break;
				 case VK_F2://switch cooling on
					ActCooling(choosen_board,TRUE);
					break;
				 case VK_F3://switch cooling off
					ActCooling(choosen_board,FALSE);
					break;
				 case VK_F4:
					 {//check temp good
					int j=0;
					char header[260];

					if (TempGood(choosen_board,1)==TRUE)
					{j=sprintf_s(header,260," temp1 good        " );}
					else
					j=sprintf_s(header,260," temp1 not good " );

					if (TempGood(choosen_board,2)==TRUE)
					{j+=sprintf_s(header+j,260," temp2 good        " );}
					else
					j+=sprintf_s(header+j,260," temp2 not good " );

					TextOut(hMSDC,100,LOY+YLENGTH+17,header,j);
					break;
					 }
				 case VK_F5: //send IR_Setup
					//SetupIR(choosen_board,1); //reset
					 // RE&RS enable
				//WriteByteS0(choosen_board,0x0f,0x30);
                    break;
				 case VK_F7: //set high amp
					HIAMP=TRUE;
					V_On(choosen_board);
                    break;
				 case VK_F8: //set low amp
					HIAMP=FALSE;
					V_Off(choosen_board);
					break;

				 //case VK_SHIFT:
				 case VK_UP: //change y-scale
						if (GetAsyncKeyState(VK_SHIFT)) {YSHIFT += 1;}
						else XOFF += 1;
				 if (YSHIFT>16) YSHIFT=16;
				 if (XOFF>_PIXEL/600) XOFF=_PIXEL/600;
				 break;
				 case VK_DOWN:
						if (GetAsyncKeyState(VK_SHIFT)) {YSHIFT -= 1;}
						else XOFF -=1 ;
				 if (YSHIFT<0) YSHIFT=0;
				 if (XOFF<1) XOFF=1;
				 break;
				 case VK_RIGHT:
					 /*XStart += 100;	
					 if (XStart>_PIXEL-XLENGTH*XOFF)
						XStart -= 100;*/			 
					PixelOdd=TRUE;
				 break;
				 case VK_LEFT:
					/* XStart -= 100;
					 if (XStart<0) XStart=0; */
					PixelOdd=FALSE;
				 break;
				 case VK_ESCAPE: //stop measurement
				 case VK_SPACE:
					Running=FALSE;
					//Sleep(20);
					//CleanupPCIE_DMA(choosen_board);
					StopRingReadThread();
					StopFFTimer(choosen_board);
					SetIntFFTrig(choosen_board);//disables ext. Trig.
					UpdateTxT();
                    break;


			  }
        break;

		case WM_MOUSEMOVE :
			if (contimess_run_once){
				UpdateTxT();
			}
			
			break;

/*		case WM_PAINT:
		case WM_WINDOWPOSCHANGED  :
              ShowWindow(hWnd,SW_SHOW);
				  //Display( 1,PLOTFLAG);
 			  break;			  
*/
		case WM_DESTROY :
			  //stop timer if it is still running
			  Running=FALSE;
			  Sleep(20); // if the DMA Interrupt is running
			  //CleanupPCIE_DMA(choosen_board);
			  //StopRingReadThread();
			  //board 1

			  if (number_of_boards >= 2){
				  StopFFTimer(2);
				  SetIntFFTrig(2);//disables ext. Trig.
				  CCDDrvExit(2);
			  }

			  StopFFTimer(1);
			  SetIntFFTrig(1);//disables ext. Trig.
			  //WDC_DriverClose();
			  CCDDrvExit(1);
			  //board 2
			

			  ReleaseDC(hMSWND,hMSDC);
              
			  PostQuitMessage(0);
              break;

      default : 
            return( DefWindowProc( hWnd, uMsg, wParam, lParam ) );
   }
   return( 0L );
}


LRESULT CALLBACK About( HWND hDlg,           
                        UINT message,        
                        WPARAM wParam,       
                        LPARAM lParam)
{
   switch (message) 
   {
       case WM_INITDIALOG:
               return (TRUE);

       case WM_COMMAND:                              
               if (   LOWORD(wParam) == IDOK         
                   || LOWORD(wParam) == IDCANCEL)    
               {
                       EndDialog(hDlg, TRUE);        
                       return (TRUE);
               }
               break;
   }

   return (FALSE); 
}






LRESULT CALLBACK SetupMeasure( HWND hDlg,           
                        UINT message,        
                        WPARAM wParam,       
                        LPARAM lParam)
{UINT val=0;
BOOL success=FALSE;
   switch (message) 
   {
       case WM_INITDIALOG:
		   SetDlgItemInt(hDlg,IDC_M_EXPTIME,ExpTime,FALSE);
		   CheckDlgButton(hDlg,IDC_ExtTrig,EXTTRIGFLAG);
		   if (TrigMod==0) CheckDlgButton(hDlg,IDC_RADIO1,BST_CHECKED);
		   if (TrigMod==1) CheckDlgButton(hDlg,IDC_RADIO2,BST_CHECKED);
		   if (TrigMod==2) CheckDlgButton(hDlg,IDC_RADIO3,BST_CHECKED);
           return (TRUE);
		   break;

       case WM_COMMAND:      
		   switch( LOWORD( wParam ) )
		   {
			case IDCANCEL:
                EndDialog(hDlg, TRUE);        
                return (TRUE);
				break;

			case IDOK:
				EXTTRIGFLAG= IsDlgButtonChecked(hDlg,IDC_ExtTrig);
				val = GetDlgItemInt(hDlg,IDC_M_EXPTIME,&success ,FALSE);	
				if (success) ExpTime=val;
				if (IsDlgButtonChecked(hDlg,IDC_RADIO1)==BST_CHECKED) TrigMod=0;
				if (IsDlgButtonChecked(hDlg,IDC_RADIO2)==BST_CHECKED) TrigMod=1;
				if (IsDlgButtonChecked(hDlg,IDC_RADIO3)==BST_CHECKED) TrigMod=2;

				// set slope for ext. trigger
				if (TrigMod == 0)	HighSlope(choosen_board);
				if (TrigMod == 1)	LowSlope(choosen_board);
				if (TrigMod == 2)	BothSlope(choosen_board);


				EndDialog(hDlg, TRUE);        
                return (TRUE);
				break;
		   }
        break; //WM_COMMAND
	
   }

   return (FALSE); 
}


LRESULT CALLBACK AllocateBuf(HWND hDlg,
	UINT message,
	WPARAM wParam,
	LPARAM lParam)
{
	UINT nob_input = 0, nospb_input = 0;
	BOOL success = FALSE;
	UINT64 builtinram, freeram, freeram_old, calcram, allocram;
	UINT divMB = 1024 * 1024;
	int trackbar_nob, trackbar_nospb, trackbar_nob_multiplier = 1, trackbar_nospb_multiplier = 1;


	switch (message)
	{
	case WM_INITDIALOG:
		SetDlgItemInt(hDlg, IDC_nob, Nob, FALSE);
		SetDlgItemInt(hDlg, IDC_nospb, Nospb, FALSE);

		FreeMemInfo(&builtinram, &freeram);
		SetDlgItemInt(hDlg, IDC_FREERAM, freeram / divMB, 0);
		SetDlgItemInt(hDlg, IDC_BUILTINRAM, builtinram / divMB, 0);

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
			nob_input = GetDlgItemInt(hDlg, IDC_nob, &success, FALSE);
			nospb_input = GetDlgItemInt(hDlg, IDC_nospb, &success, FALSE);
			if (success)
			{
				Nob = nob_input;
				Nospb = nospb_input;
				if (!BufLock(choosen_board, Nob, Nospb))
					MessageBox(hMSWND, "allocating Buffer fails", "Error", MB_OK);
				else
					MessageBox(hMSWND, "allocating Buffer succeeded", "Message", MB_OK);
				if (both_boards)
					if (!BufLock(2, Nob, Nospb))
						MessageBox(hMSWND, "allocating Buffer of secound Board  fails", "Error", MB_OK);
					else
						MessageBox(hMSWND, "allocating Buffer of secound Board succeeded", "Message", MB_OK);
			}

			trackbar_nospb = Nospb;
			while (trackbar_nospb > 30000){ //max for trackbar length
				trackbar_nospb /= 10;
				trackbar_nospb_multiplier *= 10;
			}
			trackbar_nob = Nob;
			while (trackbar_nob > 30000){ //max for trackbar length
				trackbar_nob /= 10;
				trackbar_nob_multiplier *= 10;
			}
			//update trackbars
			SendMessage(hwndTrack2, TBM_SETRANGE, TRUE,
				MAKELONG(0/*MIN RANGE*/, trackbar_nob - 1/*MAX RANGE*/));  //Optional, Default is 0-100
			SendMessage(hwndTrack, TBM_SETRANGE, TRUE,
				MAKELONG(0/*MIN RANGE*/, trackbar_nospb - 1/*MAX RANGE*/));  //Optional, Default is 0-100

			EndDialog(hDlg, TRUE);
			return (TRUE);
			break;

		case IDCALC:
			nob_input = GetDlgItemInt(hDlg, IDC_nob, &success, FALSE);
			nospb_input = GetDlgItemInt(hDlg, IDC_nospb, &success, FALSE);
			if (success)
			{
				Nob = nob_input;
				Nospb = nospb_input;
			}
			calcram = Nob * Nospb * _PIXEL * sizeof(USHORT) / divMB;
			if (calcram < 100000)
				SetDlgItemInt(hDlg, IDC_CALCRAM, calcram, 0);
			else
				SetDlgItemText(hDlg, IDC_CALCRAM, "calculation error");
			break;

		case IDALLOC:
			nob_input = GetDlgItemInt(hDlg, IDC_nob, &success, FALSE);
			nospb_input = GetDlgItemInt(hDlg, IDC_nospb, &success, FALSE);
			if (success)
			{
				Nob = nob_input;
				Nospb = nospb_input;

				if (pBLOCKBUF[choosen_board])
					free(pBLOCKBUF[choosen_board]);

				FreeMemInfo(&builtinram, &freeram);
				freeram_old = freeram;

				if (!BufLock(choosen_board, Nob, Nospb))
					MessageBox(hMSWND, "allocating Buffer fails", "Error", MB_OK);
				else
					MessageBox(hMSWND, "allocating Buffer succeeded", "Message", MB_OK);
			}

			FreeMemInfo(&builtinram, &freeram);
			SetDlgItemInt(hDlg, IDC_FREERAM, freeram/divMB, 0);
			SetDlgItemInt(hDlg, IDC_BUILTINRAM, builtinram/divMB, 0);

			allocram = (freeram_old - freeram) / divMB;
			if (allocram < 100000)
				SetDlgItemInt(hDlg, IDC_ALLOCRAM, allocram, 0);
			else//if RAM is bigger than 100gb
				SetDlgItemText(hDlg, IDC_ALLOCRAM, "calculation error");

			
				

			break;

		}
		break; //WM_COMMAND

	}

	return (FALSE);
}

LRESULT CALLBACK ChooseBoard(HWND hDlg,
	UINT message,
	WPARAM wParam,
	LPARAM lParam)
{

	switch (message)
	{
	case WM_INITDIALOG:

		switch (choosen_board)
		{
		case 1: 
			if (both_boards)
				CheckDlgButton(hDlg, IDC_EC_RADIO_BOTH, TRUE);
			else
				CheckDlgButton(hDlg, IDC_EC_RADIO1, TRUE); 
			break;
		case 2: 		   CheckDlgButton(hDlg, IDC_EC_RADIO2, TRUE); break; //EC

		}
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
			if (IsDlgButtonChecked(hDlg, IDC_EC_RADIO1) == TRUE) { choosen_board = 1; both_boards = FALSE; }
			if (IsDlgButtonChecked(hDlg, IDC_EC_RADIO2) == TRUE) { choosen_board = 2; both_boards = FALSE; }
			if (IsDlgButtonChecked(hDlg, IDC_EC_RADIO_BOTH) == TRUE) { choosen_board = 1; both_boards = TRUE; }

			EndDialog(hDlg, TRUE);
			return (TRUE);
			break;
		} //WM_COMMAND	

	}//	   message
}

LRESULT CALLBACK SetupTLevel( HWND hDlg,           
                        UINT message,        
                        WPARAM wParam,       
                        LPARAM lParam)
{UINT val=0;
BOOL success=FALSE;
   switch (message) 
   {
       case WM_INITDIALOG:
		   SetDlgItemInt(hDlg,IDC_TLevel,TempLevel,FALSE);
           return (TRUE);
		   break;

       case WM_COMMAND:      
		   switch( LOWORD( wParam ) )
		   {
			case IDCANCEL:
                EndDialog(hDlg, TRUE);        
                return (TRUE);
				break;

			case IDOK:
				val = GetDlgItemInt(hDlg,IDC_TLevel,&success ,FALSE);	
				if (success) 
					{TempLevel=val;
					SetTemp(choosen_board,(UCHAR)TempLevel);}


				EndDialog(hDlg, TRUE);        
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
                        LPARAM lParam)
{UINT val=0;
BYTE dbyte=0;
ULONG longval=0;
BOOL success=FALSE;

switch (message)
{
case WM_INITDIALOG:

				SetDlgItemInt(hDlg, IDC_SETDAT, tDAT, FALSE);
				SetDlgItemInt(hDlg, IDC_SETXDLY, tXDLY, FALSE);
			   SetDlgItemInt(hDlg,IDC_SETTCNT,tTICNT,FALSE);
			   SetDlgItemInt(hDlg,IDC_SETTCNT2,tTOCNT,FALSE);
			   CheckDlgButton(hDlg,IDC_CHECK_NOPDARS,m_noPDARS);
			   switch (m_TOmodus)
			   {	case 2: 		   CheckDlgButton(hDlg,IDC_EC_RADIO2,TRUE); break; //REG
			   case 3: 		   CheckDlgButton(hDlg,IDC_EC_RADIO3,TRUE); break; //EC
			   case 4: 		   CheckDlgButton(hDlg,IDC_EC_RADIO4,TRUE); break; //DAT
			   case 5: 		   CheckDlgButton(hDlg,IDC_EC_RADIO5,TRUE); break; //TRIGIN
			   case 6: 		   CheckDlgButton(hDlg,IDC_EC_RADIO6,TRUE); break; //FFXCK
			   default: 		   CheckDlgButton(hDlg,IDC_EC_RADIO1,TRUE); // XCKI 0 or 5
			   }

			   switch (m_ECmodus)
			   {	case 2: 		   CheckRadioButton(hDlg,IDC_ECCNT_RADIO1,IDC_ECCNT_RADIO4,IDC_ECCNT_RADIO2); break; //REG
			   case 3: 		   CheckRadioButton(hDlg,IDC_ECCNT_RADIO1,IDC_ECCNT_RADIO4,IDC_ECCNT_RADIO3); break; //EC
			   case 4: 		   CheckRadioButton(hDlg,IDC_ECCNT_RADIO1,IDC_ECCNT_RADIO4,IDC_ECCNT_RADIO4); break; //DAT
			   default: 		   CheckRadioButton(hDlg,IDC_ECCNT_RADIO1,IDC_ECCNT_RADIO4,IDC_ECCNT_RADIO1); // XCKI 0 or 5
			   }

			   switch (m_ECTrigmodus)
			   {	case 2: 		   CheckDlgButton(hDlg,IDC_RADIO12,TRUE); break; //REG
			   case 3: 		   CheckDlgButton(hDlg,IDC_RADIO13,TRUE); break; //EC
			   case 4: 		   CheckDlgButton(hDlg,IDC_RADIO14,TRUE); break; //DAT
			   default: 		   CheckDlgButton(hDlg,IDC_RADIO11,TRUE); // XCKI 0 or 5
			   }

			   
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
		//			EXTTRIGFLAG= IsDlgButtonChecked(hDlg,IDC_ExtTrig);
					//get DAT value
					longval = GetDlgItemInt(hDlg, IDC_SETDAT, &success, FALSE);
					if (success)
						{
						tDAT = longval;
						longval = tDAT;
						if (longval != 0) longval |= 0x80000000;
						WriteLongS0(choosen_board, longval, 0x20); // DAT reg
						}
					//get XCKDLY val
					longval = GetDlgItemInt(hDlg, IDC_SETXDLY, &success, FALSE);
					if (success)
					{
						tXDLY = longval;
						longval = tXDLY;
						if (longval != 0) longval |= 0x80000000;
						WriteLongS0(choosen_board, longval, 0x24); // XCKDLY reg
					}


					val = GetDlgItemInt(hDlg,IDC_SETTCNT,&success ,FALSE);
					if (success) tTICNT=val;
					val = tTICNT;
					if (val>1) {val -= 1;}
					else val = 0;
					if (val != 0) val |= 0x80;
					// devider n=1 -> n /2
					WriteByteS0(choosen_board,(BYTE) val,0x28);//TICNT reg
					val = GetDlgItemInt(hDlg,IDC_SETTCNT2,&success ,FALSE);
					if (success) tTOCNT=val;
					val = tTOCNT;
					if (val>1) {val -= 1;}
					else val = 0;
					if (val != 0) val |= 0x80;
					// devider n=1 -> n /2
					WriteByteS0(choosen_board,(BYTE) val,0x2A);//TOCNT reg

					//				CheckRadioButton(hDlg,IDC_RADIO1,IDC_RADIO5,m_TOmodus);
					if (IsDlgButtonChecked(hDlg,IDC_EC_RADIO1)==TRUE) m_TOmodus=1;
					if (IsDlgButtonChecked(hDlg,IDC_EC_RADIO2)==TRUE) m_TOmodus=2;
					if (IsDlgButtonChecked(hDlg,IDC_EC_RADIO3)==TRUE) m_TOmodus=3;
					if (IsDlgButtonChecked(hDlg,IDC_EC_RADIO4)==TRUE) m_TOmodus=4;
					if (IsDlgButtonChecked(hDlg,IDC_EC_RADIO5)==TRUE) m_TOmodus=5;
					if (IsDlgButtonChecked(hDlg,IDC_EC_RADIO6)==TRUE) m_TOmodus=6;

					switch (m_TOmodus)
					{	case 1: dbyte = 0x0; break; //XCK
					case 2: dbyte = 0x80; break; //REG
					case 3: dbyte = 0x40; break; //EC
					case 4: dbyte = 0x08; break; //DAT
					case 5: dbyte = 0x20; break; //TRIGIN
					case 6: dbyte = 0x10; break; //FFXCK
					case 7: dbyte |= 0x70; break; //Block Trig
					default:  dbyte = 0x0; // XCKI
					}

					m_noPDARS = IsDlgButtonChecked(hDlg,IDC_CHECK_NOPDARS);
					if (m_noPDARS==TRUE) dbyte |= 0x04;
					WriteByteS0(choosen_board, dbyte,0x2B);//TOFLAG reg

					if (IsDlgButtonChecked(hDlg,IDC_ECCNT_RADIO1)==TRUE) m_ECmodus=1; //CNT
					if (IsDlgButtonChecked(hDlg,IDC_ECCNT_RADIO2)==TRUE) m_ECmodus=2;
					if (IsDlgButtonChecked(hDlg,IDC_ECCNT_RADIO3)==TRUE) m_ECmodus=3;
					if (IsDlgButtonChecked(hDlg,IDC_ECCNT_RADIO4)==TRUE) m_ECmodus=4;

					if (IsDlgButtonChecked(hDlg,IDC_RADIO11)==TRUE) {m_ECTrigmodus=1;}
					if (IsDlgButtonChecked(hDlg,IDC_RADIO12)==TRUE) {m_ECTrigmodus=2;}
					if (IsDlgButtonChecked(hDlg,IDC_RADIO13)==TRUE) {m_ECTrigmodus=3;}
					if (IsDlgButtonChecked(hDlg,IDC_RADIO14)==TRUE) {m_ECTrigmodus=4;}


					m_ECmodus=1; //reset to timer mode
					m_ECTrigmodus=1;



		//			OpenShutter(choosen_board); //EC works only if shutter open
					
		EndDialog(hDlg, TRUE);
		return (TRUE);
		break;
		} //WM_COMMAND	

	}//	   message
   return (FALSE); 
}//SetupEC



