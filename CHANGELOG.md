# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).
This project doesn't use strictly [Semantic Versioning](https://semver.org/spec/v2.0.0.html) yet. Instead the version scheme is as follows: major version . hardware version number . minor version.
The hardware version number is the current version at release time.

## Unreleased
### API Changes
#### Added
#### Changed
#### Removed
#### Fixed
#### Deprecated
### Other Changes
#### Added
#### Changed
#### Removed
#### Fixed
#### Deprecated

## 4.17.4 - 2024-12-28
### API Changes
#### Added
* Add setting manipulate_data_mode
* Add setting manipulate_data_custom_factor
#### Changed
* Change parameter address from uint16_t to uint32_t in
	* DLLsetBitS0_32
	* DLLresetBitS0_32
	* DLLReadBitS0_32
	* DLLReadBitS0_32_multipleBoards
	* DLLReadBitS0_8
	* DLLReadBitS0_8_multipleBoards

## 4.17.3 - 2024-11-26
### API Changes
#### Added
* Add DLLCopyOneBlockOfOneCamera
* Add DLLCopyOneBlockOfOneCamera_multipleBoards
#### Changed
* Rename setting bnc_out to monitor
### Other Changes
#### Added
* Add support for Linux kernel 6.8
* Add hook example to CsimpleExample
#### Fixed
* Fix greyscale viewer crash for camcnt > 1

## 4.17.2 - 2024-11-18
### API Changes
#### Changed
* Rename DLLwaitForMeasureReady to DLLWaitForMeasureDone
* Rename DLLwaitForBlockReady to DLLWaitForBlockDone
* Rename DLLisMeasureOn to DLLGetMeasureOn
* Rename DLLisMeasureOn_multipleBoards to DLLGetMeasureOn_multipleBoards
#### Removed
* Remove DLLisBlockOn (use DLLGetBlockOn instead)
### Other Changes
#### Added
* Add Cam_DoSoftReset
* Add Cam_Initialize

## 4.17.1 - 2024-10-31

This release changes the way how the GUI Escam interacts with the Stresing library. For a better separation between GUI and library and a better consistency to other software like Labview, Escam now interacts with the dynamically linked DLL ESLSCDLL. Additionally there is a new simple example in C, which can be compiled for both Windows and Linux. The example is using the ESLSCDLL as well.

### API Changes
#### Added
* Add function DLLGetVerifiedDataDialog
* Add function DLLGetIsRunning
* Add function DLLGetBlockIndex
* Add function DLLGetScanIndex
* Add function DLLGetS1State
* Add function DLLGetS2State
* Add function DLLGetImpactSignal1
* Add function DLLGetImpactSignal2
* Add function DLLGetVirtualCamcnt
* Add function DLLGetTestModeOn
* Add function DLLDAC8568_setAllOutputs_multipleBoards
* Add function DLLIOCtrl_setT0_multipleBoards
* Add function DLLCheckFifoValid
* Add function DLLCheckFifoOverflow
* Add function DLLCheckFifoEmpty
* Add function DLLCheckFifoFull
* Add function DLLExportMeasurementHDF5 (exclude in minimal build)
* Add function DLLSetMeasureStartHook
* Add function DLLSetMeasureDoneHook
* Add function DLLSetBlockStartHook
* Add function DLLSetBlockDoneHook
* Add function DLLSetAllBlocksDoneHook
* Add function DLLDumpS0Registers
* Add function DLLDumpHumanReadableS0Registers
* Add function DLLDumpDmaRegisters
* Add function DLLDumpTlpRegisters
* Add function DLLDumpMeasurementSettings
* Add function DLLDumpCameraSettings
* Add function DLLDumpPciRegisters
* Add function DLLAboutDrv
* Add function DLLAboutGPX
* Add function DLLSetTORReg_multipleBoards
* Add function DLLResetDSC_multipleBoards
* Add function DLLSetDIRDSC_multipleBoards
* Add function DLLDAC8568_setOutput
* Add function DLLResetScanFrequencyBit_multipleBoards
* Add function DLLResetBlockFrequencyBit_multipleBoards
* Add function DLLInitSettingsStruct
#### Changed
* Change DLLDAC8568_setAllOutputs to single board function
* Change DLLIOCtrl_setT0 to single board function
* Change DLLSetTORReg to single board function
* Change DLLResetDSC to single board function
* Change DLLSetDIRDSC to single board function
* Change DLLResetScanFrequencyBit to single board function
* Change DLLResetBlockFrequencyBit to single board function
#### Removed
* Remove function TestMsg
### Other Changes
#### Added
* Add CsimpleExample
#### Changed
* Use DLL functions in Escam instead of Board.h functions
#### Removed
* Remove lsc-cli

## 4.17.0 - 2024-10-23
### API Changes
#### Added
* Add setting shift_s1s2_to_next_scan
#### Changed
* Rename setting sensor_reset_length to sensor_reset_or_hsir_ec
#### Removed
* Remove deprecated setting file split mode

## 4.15.1 - 2024-10-01
### API Changes
#### Added
* Add function DLLCopyDataArbitrary
* Add function DLLGetOneSamplePointer
* Add function DLLGetOneBlockPointer
* Add function DLLGetAllDataPointer
* Add function DLLGetPixelPointer
#### Changed
* Rename function DLLReturnFrame to DLLCopyOneSample and remove parameter pixel
* Rename function DLLReturnFrame_multipleBoards to DLLCopyOneSample_multipleBoards and remove parameter pixel
### Other Changes
none

## 4.15.0 - 2024-09-25
### API Changes
#### Added
* Add function DLLGetScanTriggerDetected
* Add function DLLGetBlockTriggerDetected
* Add function DLLResetScanTriggerDetected
* Add function DLLResetBlockTriggerDetected
* Add function DLLGetScanTriggerDetected_multipleBoards
* Add function DLLGetBlockTriggerDetected_multipleBoards
* Add function DLLResetScanTriggerDetected_multipleBoards
* Add function DLLResetBlockTriggerDetected_multipleBoards
* Add setting channel_select (replace deprecated setting is_hs_ir)
#### Removed
* Remove deprecated setting is_hs_ir
### Other Changes
#### Added
* Add software version to settings export
* Add lamps STD (scan trigger detected) and BTD (block trigger detected). Hardware feature implemented in P222.15.
#### Changed
* Change default of autotune to 500
#### Fixed
* Fix possible crash when passing a error code out of bounds to ConvertErrorCodeToMsg()
* Fix autotune for HSIR
* Fix InitCamera3030() CAMCNT iteration
* Fix crash after not enough RAM error
* Fix writeBit functions: make it thread safe
* Fix ASL mode. Probably broken with P222.14 and 4.14.0.
* Fix crash with two PCIe boards

## 4.14.0 - 2024-08-21
### API Changes
#### Added
- Add DLLGetBlockOn(), DLLGetBlockOn_multipleBoards()
### Other Changes
#### Added
- Add new TOR options since 222.14: BLOCK_ON, BLOCK_ON_SYNCED
#### Changed
- Change block on lamp from UI abstraction layer signal to bit polling. Since 222.14 the old block on signal is not available anymore because the software handshake was removed to reduce the time between the block trigger and the block on signal.
#### Fixed
- Improve guided settings mode

## 4.13.1 - 2024-07-22
### API Changes
- none
### Other Changes
#### Added
- Add SetSensorGain(). This function sets the new register sensor gain for HSIR. The settings parameter sensor_gain is used for setting this register.

## 4.13.0 - 2024-07-18
### API Changes
#### Changed
- Change the name of the setting sensor_reset_length_in_4_ns to sensor_reset_length. This parameter has now different meanings depending on sensor type. For HSVIS the step size is 4 ns, like before. For HSIR the step size is 160 ns.
### Other Changes
#### Added
- new dialog Chart -> Settings:
	- show / hide crosshair
	- set axes min and max
	- mirror x axis
- Add autotune support for HSIR
#### Changed
- Add duration parameter to pulse bit functions
#### Removed
- remove Chart -> Axes

## 4.12.2 - 2024-06-24
### API Changes
- none
### Other Changes
#### Changed
- Make WaitForBlockReady() and WaitForMeasureReady() abortable
- Set minimum of ioctrl_impact_start_pixel to 23
#### Fixed
- Deactivate group separator for values with 0 suffix
- Fix live view for small nos and nob
- Fix nos and nob in free settings mode

## 4.12.1 - 2024-06-13
### API Changes
#### Fixed
- Fix parameters of DLLGetXckLength_multipleBoards, DLLGetXckPeriod_multipleBoards, DLLGetBonLength_multipleBoards, DLLGetBonPeriod_multipleBoards
### Other Changes
#### Added
- Show data point dots when there are less than 50 data points in the current zoom level
#### Fixed
- Improve position label: round instead of floor

## 4.12.0 - 2024-06-12
### API Changes
#### Added
- Add bticnt (rename deprecated setting shortrs to bticnt)
- Add DLL functions for new registers: DLLGetXckLength(), DLLGetXckPeriod(), DLLGetBonLength(), DLLGetBonPeriod(), DLLGetXckLength_multipleBoards(), DLLGetXckPeriod_multipleBoards(), DLLGetBonLength_multipleBoards(), DLLGetBonPeriod_multipleBoards()
#### Changed
- Rename ticnt to sticnt
### Other Changes
#### Added
- ToolTips, WhatsThis
- Context help action
- Explanation about WhatsThis, ToolTips in mainpage.md
- new S0 register: XCKLEN, BONLEN, XCKPERIOD, BONPERIOD (since 222.12)
- new dialog: trigger info. This dialog displays XCKLEN, BONLEN, XCKPERIOD and BONPERIOD
#### Changed
- Rename file version.txt to CHANGELOG.md
- Change live view fixed sample: vsync for area sensor. Only show completed blocks.
- Make position label right aligned
- Reduce decimal places in crosshair to 1
- Improve human readable register dump
#### Removed
- Remove unused function WaitTrigger()
#### Fixed
- Fix setup project: add hdf.dll
- Fix block slider position after allBlocksDone

## 4.11.3 - 2024-05-24
### API Changes
#### Changed
- Rename trigger mode cc to trigger mode integrator
- Rename is_cooled_cam to is_cooled_camera_legacy_mode
### Other Changes
#### Added
- Add DAC autotune in DAC dialog
- Add mirror x axis in axes dialog
- Add HDF5 export under file -> export data
- Add ToolTips and WhatsThis to settings dialog
- Add Tocnt and Ticnt to settings dialog
- Add cursor crosshair in chart
#### Changed
- Change settings level basic, advanced, expert to guided, free
- Change max number of regions to 5
#### Fixed
- Fix tab order in multiple dialogs
- Fix bug after dump board register, because the registerReadWriteMutex was not released
- Improve documentation
- Improve human readable dump

## 4.11.2 - 2024-04-11
- Fix measurements interrupting on fast computers
- Change xck out cc Option from 0V to Vin sample
- Fix ASL for non FFT sensors
- Update special pixels dialog on allBlocksDone
- Set axes to default on settings saved
Linux: 
- Fix aborting measurement in some cases
- Increase DMA buffer size

## 4.11.1 - 2024-02-05
- Add DLLReadBitS0_32(), DLLReadBitS0_32_multipleBoards(), DLLReadBitS0_8(), DLLReadBitS0_8_multipleBoards()
- Extend human readable S0 dump
- Remove dialog fifo pixels
- Set cursor to wait while dumping board registers
- Add is_fft_legacy

## 4.11.0 - 2024-01-09
- Add register S1S2ReadDelay
- Add setting s1s2_read_delay_in_10ns (deprecated setting keep is beeing replaced)
- Add S1S2ReadDelay to TOR
- Extend human readable S0 dump
- Add BncOut to camerasettingswidget

## 4.10.3 - 2023-12-20
- Add bnc_out to SetConfigRegister() and struct camera_settings, not yet added to UI
- Extend human readable register dump
- Fix DLLGetCameraStatusTempGood()
Linux:
- Improve .deb package
- Fix lsc-cli
- Move dev permissions from kernel module to udev

## 4.10.2 - 2023-12-12
- Fix DLLCopyAllData_multipleBoards()

## 4.10.1 - 2023-12-07
- Hide temperature LED when not needed
- Improve dumpTlpRegisters()

## 4.10.0 - 2023-12-05
- Readd legacy ISFFT, which was removed in 4.9.0

## 4.9.1 - 2023-11-16
- Add missing function declerations: DLLIOCtrl_setOutput, DLLSetSTimer, DLLSetBTimer, TestMsg
- Show cursor position on chart
- Rename settings default button
- Add SetNosRegister() and SetNobRegister()
- Improve documentation
- Add human readable register dump
- Change OutTrigPulse to microseconds
- Add timeout to WaitForAllInterruptsDone()
- Add timeout to FindCam()

## 4.9.0 - 2023-10-18
- Deprecate setting use ec
- Remove Cam3030_ADC_SetIfcMode
- Add Cam_SetSensorResetLength
- Add sensor_reset_length_in_8_ns setting, position of former use ec	
- Remove early reset, sendrs, enec3030, shortrs
- Remove en sec and en bec bits. Setting any bit is now enabling sec and bec
- Add SetCameraSystem, extend SetSensorType
- Remove is_hs_ir
- Improve use of Use_ENFFW_protection
- Remove mshut (deprecate setting)
- Remove CloseShutter from InitPcieBoard
- Rename SSHUT/MSHUT to SEC_MSHUT/BEC_MSHUT

## 4.8.3 - 2023-09-26
- Add DLLGetCameraStatusOverTemp
- Add DLLGetCameraStatusOverTemp_multipleBoards
- Add DLLGetCameraStatusTempGood
- Add DLLGetCameraStatusTempGood_multipleBoards
- Add Fifo Pixles Dialog and improve functionality
- Fix camera found lamp

## 4.8.2 - 2023-09-07
- Add DLLReadScanFrequencyBit_multipleBoards
- Add DLLReadBlockFrequencyBit_multipleBoards
- Add DLLFindCam_multipleBoards
- Make DLLResetScanFrequencyBit and DLLResetBlockFrequencyBit multiple boards compatible.

## 4.8.1 - 2023-09-06
- Add DLLFindCam
- Add camera found lamp

## 4.8.0 - 2023-09-06
- Fix DLLReadBlockFrequencyBit and DLLResetBlockFrequencyBit

## 4.7.0 - 2023-09-05
- Improve Axes Dialog
- Add Scan and Block frequency warning
- Add functions for new camera registers structure

## 4.5.1 - 2023-06-19
- Fix DLLreadRegisterS0_32_multipleBoards

## 4.5.0 - 2023-06-19
- Fix setting led off
- Rename scan index 2 to scan index end
- Set default of sample and block sliders to end
- Update platform toolset to v143

## 4.4.9 - 2023-06-08
- Make escam compatible with linux

## 4.4.8 - 2023-06-07
- Fix DLL minimal build
- Fix typo in DLL function name: DLLGetCurrentScanNumber_multipleBoards
- Cleanup dependencies of escam installer

## 4.4.7 - 2023-06-06
- Fix crash when using DLL in Labview (bug since 4.4.6)

## 4.4.6 - 2023-06-05
- Update Qt to 6.5.1
- Fix settings value ranges
- Fix write to disc
- Improve DLL API, add *_multipleBoards functions
- Improve documentation
- Add build configuration *_minimal

## 4.4.5 - 2023-05-11
- Set sample and block slider to last sample when measurement stopped
- Prevent sending DAC values from DAC dialog too often
- Fix abort measurement

## 4.4.4 - 2023-05-04
- Add cameraPosition to DAC functions for controlling multiple 3030 cameras in line

## 4.4.3 - 2023-04-25
- Fix camcnt 0 mode
- Improve documentation
- Add Github action for publishing doxygen documentation
- Change settingContinuousPausInMicrosecondsDefault to 0
- Add SetCameraPosition
- Send DAC values to camera position 0 and 1

## 4.4.2 - 2023-04-18
- Add DLLSetGlobalSettings_matlab

## 4.4.1 - 2023-04-18
- Add FPGA Version Number in the special pixels window
- Fix crash when ExitDriver is called twice
- Fix CalcRamUsageInMB
- Fix memory leakage in escam

## 4.4.0 - 2023-04-05
- Integrate pro DLL into DLL

## 4.2.7 - 2023-04-03
- Fix camcnt spin boxes in dialogs
- Add notifyAllBlocksDone to trigger displaying data in measurement loop
- Improve live view: show last scan instead of scan 1 when measuring in loop

## 4.2.6 - 2023-03-30
- Add no board mode
- Fix software polling mode
- Fix block on lamp
- Fix bdat setting
- Fix stimer and btimer settings
- Add DLLFillUserBufferWithDummyData
- Improve WaitforTelapsed

## 4.2.5 - 2023-03-27
- Fix measurement not ending for scans < 500

## 4.2.4 - 2023-03-23
- Fix possible loss of data with e.g. scans = 510

## 4.2.3 - 2023-03-09
- Implement greyscale viewer for CAMCNT > 1

## 4.2.2 - 2023-03-06
- Fix axes in escam on startup
- Improve start / stop buttons in escam
- Improve measureOn signaling: move to outside of loop measurement

## 4.2.1 - 2023-03-02
- Fix gpx offset in esacm
- Fix DLL pro project settings
- Add special pixel functions
	- Add GetCameraStatusOverTemp, GetCameraStatusTempGood, GetBlockIndex, GetScanIndex, GetS1State, GetS2State, GetImpactSignal1, GetImpactSignal2, GetAllSpecialPixelInformation. Add all these functions to class lsc. Add enum special_last_pixels. Add struct special_pixels.
	- Add enum pixel_camera_status_bits
	- Add enum special_pixels
	- Add dialogspecialpixels
	- Add DLLGetAllSpecialPixelInformation

## 4.2.0 - 2023-02-22
- Add special case CAMCNT = 0

## 4.1.5 - 2023-02-21
- Make continiousMeasurementFlag and other variables volatile. These variables are accessed by different threads and / or ISR, hence they should be declared as volatile. The interrupt didn't work correctly without continiousMeasurementFlag beeing volatile. There are probably more variables that should be volatile.
- Other small fixes

## 4.1.4 - 2023-02-15
- Fix GetScanNumber / live view for camcnt > 1
- Fix RMS
- Fix 2d viewer
- Fix writeToDisc for multiple boards

## 4.1.3 - 2023-02-15
- Fix continuous mode in escam
- Add all boards to DLLDAC8568_setAllOutputs
- Fix GetScanNumber / live view

## 4.1.2 - 2023-02-14
- Change DLL API to use board_sel instead of drvno
- Cleanup DLL API
- Add continuous measurement to settings struct

## 4.1.1 - 2023-02-09
- Transform board select and drvno:
	- drvno is now counting 0, 1, 2, 3, 4
	- board select is used bitwise, bit 0 selects board 0, bit 1 selects board 1 ...
	- 5 boards can now be select independently
- Split settings struct in measurement settings and camera settings
	- measurement settings are settings that are the same for all boards
	- camera settings are settings that are specific for every board
	- the camera settings struct is included 5 (MAXPCIEBOARDS) times in the measurement settings struct
- Escam got a new settings dialog according to the changes above. Also the other dialogs changed.

## 4.1.0 - 2023-01-31
- Update PCIe board from 202.26 to 222.1 (new PCB: 2167-7)
- Remove setting parameter useDac
- Add control for DAC8568 at PCIe board extension PCB 2226-3 (EWS board)
	- rename function SendFLCAM_DAC to DAC8568_sendData
	- rename function DAC_setOutput to DAC8568_setOutput
	- rename function DAC_setAllOutputs to DAC8568_setAllOutputs
	- add function DAC8568_enableInternalReference
	- add parameter location to all DAC8568_* functions to choose between DAC in camera or DAC at PCIe board
	- invert logic for reordering channels in DAC8568_setAllOutputs
- Drop 'v' in version scheme, because there is no need for it

## v3.26.8 - 2023-01-24
- Set SEC register in init 3010
- Upgrade Windows SDK to 10.0.18362 and Platform Toolset to v142

## v3.26.7 - 2023-01-18
- remove file split mode

## v3.26.6 - 2023-01-18
- Fix write to disc, probably working now
- Add bit EN EC 3030 to SEC register
- Update Qt visual tool set to v304

## v3.26.5 - 2023-01-16
- Fix and improve write to disc
- Add verify data
- Change size of settings struct
- Add import settings dialog when no settings are found

## v3.26.4 - 2022-12-12
- Add setting parameter is_cooled_cam

## v3.26.3 - 2022-12-08
- Add write data to disc

## v3.26.2 - 2022-11-24
- Add Cam3030_ADC_SetIfcMode

## v3.26.1 - 2022-11-21
- Add SetSensorResetEarly
- Add I to tor out
- Add shortrs to settings dialog

## v3.26.0 - 2022-11-16
- Add sample mode and filter functions for ADC
- Improve control of sensor reset signal (IFC)
- Rename RS_MONITOR to EXPOSURE_WINDOW

## v3.24.4 - 2022-10-18
- Remove settings parameter useGPX and decide initialization of GPX by TDC bit
- Improve capitalzation of settings
- Extend TOR outputs

## v3.24.3 - 2022-10-12
- Improve names and parameters of high level board/DLL calls

## v3.24.2 - 2022-10-10
- Add DLLResetDSC, DLLSetDIRDSC and DLLGetDSC

## v3.24.1 - 2022-10-10
- Add DLLGetIsTdc and DLLGetIsDsc

## v3.24.0 - 2022-10-06
- Add Labview configurations. There are two build configurations in the visual studio solution now: Debug-Labview and Release-Labview.
	Debug-Labview: Debug configurations and Labview libraries are included for triggering events in Labview
	Release-Labview: Same as above, but release configurations
	Debug: Debug configurations and no Labview libraries are included. You can use this, when you want to use the DLL for other purposes e.g. Matlab. This option is needed to successfully compile the solution, when no Labview is installed on the computer, because then the Labview libraries are missing.
	Release: Same as above, but release configurations.
- Add setUseEC
- Fix TDC display
- Activate AboutGPX and fix it
- Remove DSC 3
- Add check for DSC and TDC flags

## v3.23.0 - 2022-05-19
-  bugfix in function copyRestData() of adress calculation (G.S.)

## v3.22.2 - 2022-05-12
-  bugfix in return address calculation fuer labview frame return 2nd cam in board.c function GetIndexOfPixel. Removed "+ 4" (A.M.)

## v3.22.1 - 2022-03-30
- Fix crash at exit with board sel = 3
- Add possibilty to add dummy data when no camera is found
- Improve SendFLCam: Add FindCam to SendFLCam
- Change default of Vfreq to 7
- Fix RMS calculation

## v3.22.0 - 2022-02-02
- The previous statement, that the new scan counter at the last pixel was introduced in P202_21 was false. P202_22 does that and this version is required to use software polling.
- Fix ValMsg and ErrorMsg
- Other small fixes

## v3.21.1 - 2022-01-11
- Add live view on linux
- Add Tocnt and Ticnt
- Fix some crashes
Escam:
- Improve dump registers dialog

## v3.21.0 - 2021-12-14
- Increase PCIe board version to P202_21
- There is an additional scan counter at the last pixel now. This is used in PollDmaBufferToUserBuffer.
- Measured: Software polling is possible up to 33 kHz. Measured with 3030, 1088 pixel

## v3.20.6 - 2021-12-08
- Improve software polling
- Improve escam settings dialog
- Add live view options to escam

## v3.20.5 - 2021-12-01
- Add software polling option
- Add live view

## v3.20.4 - 2021-11-17
- Add script for building .deb packages
- Improve README.md
- Fix DMA to user buffer copy process on linux
- Make Makefile of linux kernel module compatible with DKMS
- Change default file permissions of /dev/lscpcie0 to 666
- Add software flag COMPILE_FOR_LABVIEW

## v3.20.3 - 2021-11-09
- Add escam_setup
- Add dumpPciRegisters
- Add AboutSettings
- And more small fixes

## v3.20.2 - 2021-10-27
- Fix greyscale viewer (by fixing pro dlls global variables)
- Remove keep
- Improve dump tlp
- Improve order of StartStimer in startMeasurement
- Add continiousPauseInMicroseconds
escam:
- Add greyscale viewer
- Add setting import / export function

## v3.20.1 - 2021-10-20
- Refactor gain and gain to sensor_gain and adc_gain
The new sensor_gain is the old boolean gain. Sensor_gain is now an
unsigned integer, because the new 3030 IR sensor got 4 different gain modes.
Setting the sensor gain is moved to InitCameraGeneral.

## v3.20.0 - 2021-10-08
- Increase required PCIe board version: 202.20
- Add DLLIOCtrl_setOutput and DLLIOCtrl_setT0
- Add IOCtrl_setAllOutputs
- Improve SetDmaRegister: now all integer multiples of 64 are possible for pixelcount
- Add SetPriority and ResetPriority
- Improve StartMeasurements order and timing
- Change DLLSetupROI: replace automatic alternating keep order based on keep_first with manual keep order
Escam:
- Add BEC to setting dialog
- Add keep to setting dialog
- Improve setting dialog
- Add chopper options for BTI

## v3.19.7 - 2021-09-02
- Add time check in UIAbstractionLayer to prevent too many notifies
- Add function IOCtrl_setImpactStartPixel
- Add function IOCtrl_setOutput
- Add function IOCtrl_setT0
escam:
- Set ymax depending on camera system
- Fix bslope
- Fix dac_output board2

## v3.19.6 - 2021-08-18
- Rename lsc-gui to escam
DLL:
- Insert correct tabbing for register dump
escam:
- Make DAC compatible with two cams
- Hide board select when there is only one cam
- Improve dialogRMS: add board select and campos

## v3.19.5 - 2021-08-11
- Add DSC functions
- Remove CCDExamp
- Fix board sel functionality
- Allow different values for DACs on different boards
lsc-gui:
- Add DSC window
- Add RMS window

## v3.19.4 - 2021-08-06
- Fix two board functionality
- Change high level board functions from using drvno to using board_sel
Lsc-gui:
- Add live editable DAC window
- Make gui compatible with two boards

## v3.19.3 - 2021-07-30
- Readd check for keys:
	- Esc for abortion
	- Space for stop continuous
- Add DAC_setAllOutputs
- Add ValMsg
- Make Qt GUI compatible with Qt versions lower than 5.15
- Linux Kernel module is now probably compatible with kernel version >= 5
- Add setting isIR

## v3.19.2 - 2021-07-15
Lsc-gui:
- Make chart zoomable
- Add continuous mode
- Add settings level
- Add error messages for user
- Add dialog: cameras
- Add dialog: about
- Add dialog: about Qt
- Add dialog: axes
- Improve register dump dialog
- Improve dark mode
- Improve settings dialog

## v3.19.0 - 2021-06-24
- This version introduces complete crossplattform support (Linux + Windows)
- New project structure: one crossplattform Board.c is now used in ESLSCDLL, ESLSCDLL_pro and lsc-gui. CCDExamp is still using the deprecated Board_old
- Improve logging
- Add RsDSC
- lsc-gui: 
	- Add Tab in dump board registers for settings struct
	- Add some intelligence to settings dialog
	- Add feature for multiple cameras

## v3.17.1 - 2021-04-26
- Add DLLInitMeasurement

## v3.17.0  - 2021-04-23
- Introduce switch for software trigger on and off
- Merge Qt GUI project with mainline of EBST_CAM
- Add CCDExamp Setup project which creates an installer for CCDExamp
- More small fixes

## v3.16.6 - 2021-03-23
- Introduce error handling: es_status_codes

## v3.16.5 - 2021-03-16
 - general init routine common for all cameras replaces (InitCam3001 in labview)
 - reduced InitCam3010 to cam specific parameters
 - added LedOff and SetGain function inclduing vias for LabView
 - removed old trigger config functions

## v3.16.4 - 2021-03-03
 - Function SetPartialBinning() must call WriteLongS0() before SetS0Bit()

## v3.16.3 - 2021-02-04

## v3.16.1 - 2021-02-02
- with event on MeasureOn and BlockOn
- with delay stage counter

## v3.15.7 - 2021-01-22
- Add DLLCopyOneBlock
- Fix XCKDELAY
- Fix notifys beeing at wrong position
- Fix block trigger

## v3.15.6 - 2021-01-21
- Add new way to communicate from DLL to Labview (PostLVUserEvent)

## v3.15.5 - 2021-01-15
- Lots of refactoring. Rename nDLLSetupDMA to DLLSetMEasurementParameters
- CCDExamp: add TRMS for second camera
- Replace CCDExamples version numbers with global ones
- Add MANUAL_OVERRIDE_TLP+

## v3.15.4 - 2021-01-08
- Add workaround for measure on beeing reset too early

## v3.15.3 - 2020-12-18
- isr routine needs SubBufCounter which was reset by main loop - last DMA block not copied
- ReturnFrame has offset of 4 pixel to next scan

## v3.15.2 - 2020-12-11
- fix TRMS calculation in CCDExamp

## v3.15.1 - 2020-12-10
- reduce TLPS by 1

## v3.14.0 - 2020-11-20
- new version scheme: MAJOR VERSION . CORRESPONDING PCIe BOARD VERSION . MINOR VERSION
- new input scheme: BTI and STI
- change Bit ISR_Active from PCIEREG to IRQREG
- AboutS0 shows 4 memory slots more (TR - test register)
- Changed Sets0Bit and ResetS0Bit to use BitTestAndSet/BitTestAndReset and get more speed(old = 15microsec; new 3,5(+/-3)microsec)
- Added new Scan and Blocktrigger options in CCDExamp as an Dialog Window and changed dll function for that
- Add SetEC Dialog for CCDExamp
- Fix CalcTrms

## v3.2.0 - 2020-11-06
- Add functions SetBTimer, SetBSlope

## v3.1.0 - 2020-10-23
- Remove most parameters of ReadFFLoop
- Seperate setting s timer on and setting the time
- Add isBlockOn
- Add wairForBlockReady

## v3.0.0 - 2020-10-23
- new registers R16-19: BTIMER, BDAT, BEC, BFLAGS 
	- block trigger path is now completely independent from scan and got own registers
	- the scan has now SSLOPE SDAT and SEC and the block has BSLOPE BDAT and BEC
	- so the blocks and scans can be triggert independent.  
- new function: ClearAllUserRegs - resets the S and B regs to zero

## v2.3.1 - 2020-10-16
- Fix greyscale viewer not changing frame

## v2.3.0 - 2020-10-02
- Add function DLLisMeasureOn
- Add function DLLwaitForMeasureReady
- Control of BlockOn / BON will be moved to software:
	- Add function setBlockOn
	- Add function resetBlockOn
- Refactoring: Clarify blocktrigger cases
- Refactoring: Add enum PCIEFLAGS_bits
- Refactoring: extract function countBlocksByHardware

## v2.2.0 - 2020-09-11
- Add DLLCopyAllData

## v2.1.1 - 2020-09-11
- Remove sleep(100) from ReadFFLoopThread

## v2.1.0 - 2020-07-29
- Fix hDev not beeing initialized in pro DLL which led to partially not working pro DLL
