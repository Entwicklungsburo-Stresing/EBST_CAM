//  DLL header    V 3.3    2/13

DllAccess int DLLGetProcessCount();
DllAccess int DLLGetThreadCount();

DllAccess  void DLLErrMsgBoxOn(void);	//BOARD.C sends error messages on default
DllAccess  void DLLErrMsgBoxOff(void);	//general deactivate of error message boxes

DllAccess UINT8 DLLCCDDrvInit(UINT32 drv);		// init the driver -> true if found
DllAccess UINT8 nDLLCCDDrvInit(void);		// init the driver -> true if found
DllAccess void DLLCCDDrvExit(UINT32 drv);		// closes the driver
DllAccess void DLLCleanupPCIE_DMA(UINT32 drv);
DllAccess void DLLCleanupDMA(UINT32 drv);
DllAccess UINT8 DLLInitBoard(UINT32 drv, UINT32 pixel, UINT32 flag816, UINT32 pclk, UINT32 xckdelay);
DllAccess UINT8 DLLReadByteS0(UINT32 drv,UINT8 *data,UINT32 PortOff);// read byte from Port, PortOff = Regs of Board
DllAccess UINT8 DLLWriteByteS0(UINT32 drv,UINT8 DataByte, UINT32 PortOff); // writes DataByte to Port
DllAccess UINT8 DLLReadLongS0(UINT32 drv, UINT32 *data, UINT32 PortOff);	// read long from Port, PortOff Regs of Board
DllAccess UINT8 DLLWriteLongS0(UINT32 drv, UINT32 DataL, UINT32 PortOff); // writes DataLong to Port
DllAccess UINT8 DLLReadLongDMA(UINT32 drv, UINT32 *data, UINT32 PortOff);	// read long from Port, PortOff Regs of Board
DllAccess UINT8 DLLWriteLongDMA(UINT32 drv, UINT32 DataL, UINT32 PortOff); // writes DataLong to Port
DllAccess UINT8 DLLReadLongIOPort(UINT32 drv, UINT32 *data,UINT32 PortOff); // writes DataByte to Port
DllAccess UINT8 DLLWriteLongIOPort(UINT32 drv,UINT32 DataL, UINT32 PortOff); // writes DataByte to Port



DllAccess void DLLAboutDrv(UINT32 drv);	// displays the version and board ID = test if board is there

//	functions for managing controlbits in CtrlA register
DllAccess void DLLHighSlope(UINT32 drv);		//set input Trigger slope high
DllAccess void DLLLowSlope(UINT32 drv);		//set input Trigger slope low
DllAccess void DLLBothSlope(UINT32 drv);	//trigger on each slope
DllAccess void DLLOutTrigHigh(UINT32 drv);		//set output Trigger signal high
DllAccess void DLLOutTrigLow(UINT32 drv);		//set output Trigger signal low
DllAccess void DLLOutTrigPulse(UINT32 drv,UINT32 PulseWidth);	// pulses high output Trigger signal
DllAccess void DLLWaitTrigger(UINT32 drv,UINT8 ExtTrigFlag,UINT8 *SpaceKey, UINT8 *EscapeKey);	// waits for trigger input or Key
					  

DllAccess UINT8 DLLSetS0Bit(ULONG bitnumber, CHAR Address, UINT32 drvno);
DllAccess UINT8 DLLResetS0Bit(ULONG bitnumber, CHAR Address, UINT32 drvno);

DllAccess void DLLOpenShutter(UINT32 drv);	// set IFC=high
DllAccess void DLLCloseShutter(UINT32 drv);	// set IFC=low
DllAccess void DLLVOn(UINT32 drv);			// set V_On signal low (V = V_Fak)
DllAccess void DLLVOff(UINT32 drv);			// set V_On signal high (V = 1)
DllAccess UINT8 DLLReadKeyPort(UINT32 drv); //read key scan code on port 0x60
										// works only on PS2 keyboard

DllAccess void DLLClrRead(UINT32 drvno, UINT32 fftlines, UINT32 zadr, UINT32 CCDClrCount);
DllAccess void DLLClrShCam(UINT32 drvno, UINT32 zadr);


// read of NO FIFO version
DllAccess void DLLReadLoop(UINT8 drv, pArrayT pdioden, UINT32 fftlines, INT32 fkt, UINT32 zadr, UINT32 nos, UINT32 exptus, UINT32 freq, UINT32 threadp,UINT32 clrcnt, UINT32 exttrig);


//  FIFO version functions

DllAccess void DLLSetupVCLK(UINT32 drvno, UINT32 lines, UINT8 vfreq);//set the VCLK regs

DllAccess void DLLStartTimer(UINT32 drvno,UINT32 exptime);	//starts 28bit timer of PCI board
DllAccess void DLLSWTrig(UINT32 drvno);						//start a read to FIFO by software
DllAccess void DLLStopFFTimer(UINT32 drvno);					// stop timer
DllAccess UINT8 DLLFFValid(UINT32 drvno);						// TRUE if linecounter>0
DllAccess UINT8 DLLFlagXCKI(UINT32 drvno);						// TRUE if read to FIFO is active
DllAccess void DLLRSFifo(UINT32 drvno);						// reset FIFO and linecounter
DllAccess void DLLSetExtTrig(UINT32 drvno);					// read to FIFO is triggered by external input I of PCI board
DllAccess void DLLSetIntTrig(UINT32 drvno);					// read to FIFO is triggered by Timer
DllAccess BYTE DLLReadFFCounter(UINT32 drvno);					// reads 4bit linecounter 

DllAccess void DLLDisableFifo(UINT32 drvno);			//switch fifo off
DllAccess void DLLEnableFifo(UINT32 drvno);				//switch fifo on
DllAccess void DLLPickOneFifoscan(UINT32 drvno,pArrayT pdioden,UINT8* pabbr,UINT8* pspace,INT32 fkt);//get one line from fifo
DllAccess UINT8 DLLFFOvl(UINT32 drvno);					//TRUE if fifo overflow occured

DllAccess void DLLStartRingReadThread(UINT32 drvno, UINT32 ringfifodepth, UINT32 threadp, __int16 releasems);	//starts 28bit timer and get thread
DllAccess void DLLStopRingReadThread(void);
DllAccess UINT32 DLLReadRingCounter(UINT32 drvno);
DllAccess void DLLReadRingLine(pArrayT pdioden, UINT32 lno); //read ring buffer line number lno 
DllAccess UINT8 DLLReadRingBlock(void* pdioden, UINT32 start, UINT32 stop);// read ring buf to user buf rel. to act pointer
DllAccess void DLLStartFetchRingBuf(void); //start to copy data to copy buffer
DllAccess UINT8 DLLFetchLastRingLine(pArrayT pdioden); //read last ring buffer line 
DllAccess UINT8 DLLRingValid(UINT32 drvno);	// TRUE if linecounter>0
DllAccess UINT8 DLLRingThreadIsOFF(void);   // needed to sync thread stop
DllAccess UINT8 DLLBlockTrig(UINT32 drv,UCHAR btrig_ch); //read trigger input ->ch=1:pci in, ch=2:opto1, ch=3:opto2

//camera reads FIFO version
DllAccess void DLLReadFifo(UINT32 drvno, pArrayT pdioden, INT32 fkt); //read camera
DllAccess void DLLReadFFLoop(UINT32 drv, UINT32 exptus, UINT32 freq, UINT8 exttrig, UINT8 blocktrigger,UINT8 btrig_ch);
DllAccess void nDLLReadFFLoop(UINT32 board_sel, UINT32 exptus, UINT8 exttrig, UINT8 blocktrigger, UINT8 btrig_ch);
DllAccess void DLLStopFFLoop(void);
//************ system timer
DllAccess UINT64 DLLInitSysTimer(void);
DllAccess UINT8 DLLWaitforTelapsed(UINT32 musec);
DllAccess UINT64 DLLTicksTimestamp(void);
DllAccess UINT32 DLLTickstous(UINT64 tks);

//************  Cooling
DllAccess void DLLActCooling(UINT32 drvno, UINT8 on);	
DllAccess UINT8 DLLTempGood(UINT32 drvno, UINT32 ch);	
DllAccess void DLLSetTemp(UINT32 drvno, UINT32 level);
DllAccess void DLLSetEC(UINT32 drvno, UINT64 ecin100ns);
DllAccess void DLLSetTORReg(UINT32 drvno, UINT8 fkt);

DllAccess void DLLSetupDELAY(UINT32 drvno, UINT32 delay);





DllAccess void DLLSetISPDA(UINT32 drvno, UINT8 set);
DllAccess void DLLSetISFFT(UINT32 drvno, UINT8 set);
DllAccess void DLLRsTOREG(UINT32 drvno);
DllAccess void DLLSetupHAModule(UINT8 irsingle,UINT32 fftlines);

DllAccess void DLLInitCDS_AD(UINT32 drvno,UINT8 sha,UINT32 amp,INT32 ofs,UINT32 tigain);
DllAccess void DLLSetupVPB(UINT32 drvno, UINT32 range, UINT32 lines, UINT8 keep);

DllAccess void DLLSetupDMA(UINT32 drv, void*  pdioden, UINT32 nos, UINT32 blocks);
DllAccess void nDLLSetupDMA(UINT32 drv, UINT32 nos, UINT32 blocks);
DllAccess void DLLCleanupDMA(UINT32 drvno);

DllAccess void DLLSetADGain(UINT32 drvno, UINT8 fkt, UINT8 g1, UINT8 g2, UINT8 g3, UINT8 g4, UINT8 g5, UINT8 g6, UINT8 g7, UINT8 g8);
DllAccess void DLLErrorMsg(char ErrMsg[20]);
DllAccess void DLLCalcTrms(UINT32 drvno, UINT32 nos, ULONG TRMSpix, UINT16 CAMpos, double *mwf, double *trms);
DllAccess void DLLStart2dViewer(UINT32 drvno, UINT16 cur_nob, UINT16 cam, UINT pixelAmount, UINT nos);
DllAccess void DLLShowNewBitmap( UINT32 drvno, UINT16 cur_nob, UINT16 cam, UINT pixelAmount, UINT nos );
DllAccess void DLLDeinit2dViewer();
DllAccess void DLLSetGammaValue( UINT white, UINT black );