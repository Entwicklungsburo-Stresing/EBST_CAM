//  Board.h				PCI V2.31
//	all functions for managing Interfaceboard
//	with & without Fifo  
//  new: data array ushort
//2.31: 64 bit dma acces

void ErrMsgBoxOn(void);
void ErrMsgBoxOff(void); // switch to suppress error message boxes
void ErrorMsg(char ErrMsg[100]);
void ValMsg(UINT64 val);
void AboutS0(UINT32 drvno);
//  same header file for ISA and PCI version
BOOL CCDDrvInit(void);
void CCDDrvExit(UINT32 drvno);	// closes the driver
BOOL InitBoard(UINT32 drvno);	// init the board and alloc mem, call only once !
void RSInterface(UINT32 drvno);		//set all registers to zero
BOOL SetBoardVars(UINT32 drvno, UINT32 camcnt, ULONG pixel, ULONG flag816, ULONG xckdelay);
BOOL SetupPCIE_DMA(UINT32 drvno, ULONG nos, ULONG nob);
void StartPCIE_DMAWrite(UINT32 drvno);
void StartPCIE_DMAWrite(UINT32 drvno);
void CleanupPCIE_DMA(UINT32 drvno);
void GetLastBufPart(UINT32 drvno);
extern DWORD64 IsrCounter;
int GetNumofProcessors();
void ErrorMsg(char ErrMsg[100]);
BOOL ReadLongIOPort(UINT32 drvno, ULONG *DWData, ULONG PortOff);// read long from IO runreg
BOOL ReadLongS0(UINT32 drvno, ULONG *DWData, ULONG PortOff);	// read long from space0
BOOL ReadByteS0(UINT32 drvno, BYTE *data, ULONG PortOff);	// read byte from space0
BOOL WriteLongIOPort(UINT32 drvno, ULONG DWData, ULONG PortOff);// write long to IO runreg
BOOL WriteLongS0(UINT32 drvno, ULONG DWData, ULONG PortOff);// write long to space0
BOOL WriteByteS0(UINT32 drvno, BYTE DWData, ULONG PortOff); // write byte to space0
void ClrRead(UINT32 drvno, ULONG fftlines, ULONG zadr, ULONG ccdclrcount);
// clear camera with reads
void ClrShCam(UINT32 drvno, UINT32 zadr);// clears Shuttercamera with IFC signal
void AboutDrv(UINT32 drvno);	// displays the version and board ID = test if board is there
//	functions for managing controlbits in CtrlA register
void HighSlope(UINT32 drvno);		//set input Trigger slope high
void LowSlope(UINT32 drvno);		//set input Trigger slope low
void BothSlope(UINT32 drvno);		//set trigger on both slopes 
void NotBothSlope(UINT32 drvno);		//set trigger on both slopes off
void OutTrigHigh(UINT32 drvno);		//set output Trigger signal high
void OutTrigLow(UINT32 drvno);		//set output Trigger signal low
void OutTrigPulse(UINT32 drvno, ULONG PulseWidth);	// pulses high output Trigger signal
void WaitTrigger(UINT32 drvno, BOOL ExtTrigFlag, BOOL *SpaceKey, BOOL *EscapeKey);
// waits for trigger input or Key
void WaitTriggerShort(UINT32 drvno, BOOL ExtTrigFlag, BOOL *SpaceKey, BOOL *EscapeKey);
void EnTrigShort(UINT32 drvno);
void RSTrigShort(UINT32 drvno);
void DisTrigShort(UINT32 drvno);
BOOL CheckFFTrig(UINT32 drvno);		// trigger sets FF - clear via write CtrlA 0x10
void OpenShutter(UINT32 drvno);		// set IFC=high
void CloseShutter(UINT32 drvno);	// set IFC=low
BOOL GetShutterState(UINT32 drvno);	//get the actual state
void V_On(UINT32 drvno);			// set V_On signal low (V = V_Fak)
void V_Off(UINT32 drvno);			// set V_On signal high (V = 1)
void SetOpto(UINT32 drvno, BYTE ch);  // set opto channel if output
void RsetOpto(UINT32 drvno, BYTE ch); // reset opto channel if output
BOOL GetOpto(UINT32 drvno, BYTE ch);	//read opto channel if input
void SetDAT(UINT32 drvno, ULONG tin100ns); // delay after trigger in 100ns
void RSDAT(UINT32 drvno); // disable delay after trigger in S0+0x20
// new Keyboard read which is not interrupt dependend
// reads OEM scan code directly on port 0x60
UCHAR ReadKeyPort(UINT32 drvno);
//TIs electron multiplier
void SetHiamp(UINT32 drvno, BOOL hiamp);
// FIFO functions
void StartFFTimer(UINT32 drvno, ULONG exptime);	//starts 28bit timer of PCI board
void SWTrig(UINT32 drvno);						//start a read to FIFO by software
void StopFFTimer(UINT32 drvno);					// stop timer
BOOL FFValid(UINT32 drvno);						// TRUE if linecounter>0
BOOL FlagXCKI(UINT32 drvno);						// TRUE if read to FIFO is active
void RSFifo(UINT32 drvno);						// reset FIFO and linecounter
void SetExtFFTrig(UINT32 drvno);					// read to FIFO is triggered by external input I of PCI board
void SetIntFFTrig(UINT32 drvno);					// read to FIFO is triggered by Timer
BYTE ReadFFCounter(UINT32 drvno);					// reads 4bit linecounter 
BOOL ReadFifo(UINT32 drvno, void* pdioden, long fkt); //reads fifo data
void DisableFifo(UINT32 drvno);					//switch FIFO off
void EnableFifo(UINT32 drvno);					//switch Fifo on
void SetupVCLKReg(UINT32 drvno, ULONG lines, UCHAR vfreq);//setup hardware vclk generator
void SetupVCLKrt(ULONG vfreq);					//setup vclkfreq for rt version(noFIFO)
void SetupDELAY(UINT32 drvno, ULONG delay);		//setup DELAY for WRFIFO
BOOL FFOvl(UINT32 drvno);							//TRUE if FIFO overflow since last RSFifo call
void PickOneFifoscan(UINT32 drvno, pArrayT pdioden, BOOL* pabbr, BOOL* pspace, ULONG fkt); //get one scan of free running fifo timer
// Class & Thread priority functions
BOOL SetPriority(ULONG threadp);		//set priority threadp 1..31 / 8 = normal and keep old in global variable
BOOL ResetPriority();					//switch back to old level
// System Timer
UINT64 InitHRCounter();				//init system counter and returns TPS: ticks per sec
UINT64 ticksTimestamp();				//reads actual ticks of system counter
UINT64 ustoTicks(ULONG us);			//calcs microsec to ticks  
UINT32 Tickstous(UINT64 tks);			//calcs ticks to microsec
// Cooler& special functions
void ActCooling(UINT32 drvno, BOOL on);				//activates/deactivates cooling
BOOL TempGood(UINT32 drvno, UINT32 ch);						//high if temperature is reached
void SetTemp(UINT32 drvno, ULONG level);				//set temperature - 8 levels possible
void RS_ScanCounter(UINT32 drv);
void RS_DMAAllCounter(UINT32 drv, BOOL hwstop);
void SetISPDA(UINT32 drvno, BOOL set);		//hardware switch for IFC and VON if PDA
void SetISFFT(UINT32 drvno, BOOL set);		//hardware switch for IFC and VON if FFT
void SetTORReg(UINT32 drvno, BYTE fkt);
void RsTOREG(UINT32 drvno);					//reset the TOREG - should be called before SetISPDA or SetISFFT
void SetupHAModule(BOOL irsingle, ULONG fftlines);//set the module C8061&C7041 inits
void SendFLCAM(UINT32 drvno, BYTE maddr, BYTE adaddr, USHORT data);//B! test
void InitCamera3001(UINT32 drvno, UINT16 pixel, UINT16 trigger_input, BOOL IS_FFT);
void InitCamera3010(UINT32 drvno, UINT16 pixel, UINT8 trigger_input, UINT8 adc_mode, UINT16 custom_pattern, BOOL led_on, BOOL gain_high);
void InitCamera3030(UINT32 drvno, UINT8 adc_mode, UINT16 custom_pattern, UINT8 gain);
void SetADGain(UINT32 drvno, UINT8 fkt, UINT8 g1, UINT8 g2, UINT8 g3, UINT8 g4, UINT8 g5, UINT8 g6, UINT8 g7, UINT8 g8);//B!test
//jungo dma
void StartReadWithDma(UINT32 drvno);
unsigned int __stdcall ReadFFLoopThread(void *parg);
// software ring buffer thread functions
void StartRingReadThread(UINT32 drvno, ULONG ringfifodepth, ULONG threadp, __int16 releasems);
void StopRingReadThread(void); //starts and ends background thread 
void ReadFFLoop(UINT32 board_sel, UINT32 exptus, UINT8 exttrig, UINT8 blocktrigger, UINT8 btrig_ch);
void StartFetchRingBuf(void); //starts getting the data to ring
BOOL RingThreadIsOFF(void);	//checks state of thread
void FetchLastRingLine(void* pdioden); //copy last line to pdioden
void ReadRingLine(void* pdioden, UINT32 lno); // read line with index lno to pdioden
UINT8 ReadRingBlock(void* pdioden, INT32 start, INT32 stop); // copy a block of lines to userbuffer pdioden	
//start<0 is in the past, stop>0 is in the future, relative to call of this function
BOOL BlockTrig(UINT32 drv, UINT8 btrig_ch); //read state of trigger in signals during thread loop
void SetADGain(UINT32 drvno, UINT8 fkt, UINT8 g1, UINT8 g2, UINT8 g3, UINT8 g4, UINT8 g5, UINT8 g6, UINT8 g7, UINT8 g8);
int GetIndexOfPixel(UINT32 drvno, ULONG pixel, UINT16 sample, UINT16 block, UINT16 CAM);
UINT8 WaitforTelapsed(LONGLONG musec);