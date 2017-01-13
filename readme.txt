Readme   CCDExample for 2 Threads

This example shows the optimal use of the FIFO- PCI board on a multi core system
It is not intended for a single processor system! (system will hang)
Here one processor is used to poll the FIFO for new complete lines and if there are some, 
they are copied to the main RAM. If a ring buffer or a big linear buffer for a block of scans 
is needed, the pointer pDIODEN in the call of ReadFifo (CCDUNIT)must be changed for each call.
A 2nd thread is used to display the data on the screen.
In a high speed environment (33MHz pclk and short exposuretime) the FIFO polling thread 
(MeasureFifo in CCDUNIT) should be run in a high priority state (Flag USETHREAD in GLBAL.H =TRUE).
This avoids a FIFO overflow if looptime and pixel exceeds FIFO size (64kByte in high speed systems).
The Function FFOvl() (in BOARD.C) is the hardware signal of the FIFO if an overflow occurs.
It has to be reset by the RSFifo Function.

The Example consists of 3 Modules:
CCDexamp : Mini win programm for a simple window and calling of the driver.

Board:  all Functionc of the Interface board
CCDUnit: Readloop and Exapmles for the 2 Threads.
GLOBAL.H sets the most constants for the specific camera.

the ccdexamp is not equal for all cameras. Some parameters must be changed.
Especially the following parameters of Modul GLOBAL.h:

	The most important are:
	PIXEL:  number of pixels of the sensor
		it should be used the next higher value, which can be devided by 600.
		(the display can be faster calculatet then)
	FLAG816: for 8 or 12 Bit Cameras
	YShift: for the devide factor of the YScale 
	FFTLINES: (only area sensors: no. of vertical lines

	with delayms the exposuretime is determined (in ms).
	The other Parameters are explained more deeply in the manual.

	DRV sets the number of the Interface boards (usually=1)

The program must be rebuild after changing a value.
For info only: When delivered all parameters are already set for the delivered camera.

When using other versions of visual studio, the *.dsw file 
must be opened instead of the *.sln file. Visual generates a new sln file then.

BUGFIX
if C1189 unrecognized OS put WINNT in the Preprocessore defines in the properties



