# Entwicklungsbuero Stresing software documentation

This documentation is about the [software](https://github.com/Entwicklungsburo-Stresing) for operating the specialized line scan cameras of [Entwicklungsbuero Stresing](https://stresing.de). For instructions about compiling have a look at the readme in the [EBST_CAM](https://github.com/Entwicklungsburo-Stresing/EBST_CAM) repository. For more detailed information about your camera refer to your given manual FLbook.pdf.

## How to use Stresing software

There are two possibilities to use this software.

1. Use the compiled library ESLSCDLL.dll and operate the camera from your software with DLL calls. There are some examples available that are operating the camera with the DLL for [LabVIEW](https://github.com/Entwicklungsburo-Stresing/lv64), [Python](https://github.com/Entwicklungsburo-Stresing/stresing_python) and [Matlab](https://github.com/Entwicklungsburo-Stresing/stresing_matlab).
2. Include the source files in your project and compile it as one software project. Escam and lsc-cli are using this way to operate the camera, both located in the [EBST_CAM](https://github.com/Entwicklungsburo-Stresing/EBST_CAM) repository.

Have a look at the block diagrams at the bottom of this page. They are illustrating both ways of using the Stresing software.

## How to operate Stresing cameras

For a simple measurement the following DLL calls should be used. While including the source code of the project, the calls are similar just without the DLL in the function name. For a complete API documentation go to the DLL documentation [page](_e_s_l_s_c_d_l_l_8c.html).

1. [DLLInitDriver](@ref DLLInitDriver): Initialize the driver. Call it once at startup. 
2. [DLLInitBoard](@ref DLLInitBoard): Initialize PCIe board. Call it once at startup.
3. [DLLSetGlobalSettings](@ref DLLSetGlobalSettings) or [DLLSetGlobalSettings_matlab](@ref DLLSetGlobalSettings_matlab): Set settings parameter according to your camera system. Call it once at startup and every time you changed settings.
4. [DLLInitMeasurement](@ref DLLInitMeasurement): Initialize Hardware and Software for the Measurement. Call it once at startup and every time you changed settings.
5. [DLLStartMeasurement_blocking](@ref DLLStartMeasurement_blocking) or [DLLStartMeasurement_nonblocking](@ref DLLStartMeasurement_nonblocking): Start the measurement. Call it every time you want to measure.
6. [DLLAbortMeasurement](@ref DLLAbortMeasurement): Use this call, if you want to abort the measurement.
7. [DLLReturnFrame](@ref DLLReturnFrame), [DLLCopyAllData](@ref DLLCopyAllData), or [DLLCopyOneBlock](@ref DLLCopyOneBlock): Get the data with one of the following 3 calls. Call it how many times you want.
8. [DLLExitDriver](@ref DLLExitDriver): Before exiting your software, use this call for cleanup.

## How to use escam

### Setting up escam
After opening the program, you will be greeted by the main page. To configure the settings for the camera, open the 'Settings' tab at the top and choose 'Edit' and the 'General' tab of the settings will open. The first time you start the program on your PC, the initial values have to be imported. We have saved your setup in a config.ini file in the escam folder. So import this file at the first start. These values are stored then in windows (registry) and you do not need to import it again. 

### General
In the 'General' tab you can choose the amount of samples the camera should take in a block, and the amount of blocks. To edit the samples amount, edit the number at 'nos'. To edit the blocks amount, edit the number at 'nob'. For the concept of scans and blocks please see the manual. For a first try just enter scans 500 and blocks 1.

### Camera Settings
When opening the 'Camera Settings' tab you start at the 'Measurement' tab, where you can configure which trigger is used, if you want to trigger the samples/blocks external or configure a time at which the samples/blocks should start.

#### Measurement
The 'Measurement' tab is where you can configure the settings related to the measurement.

##### external Trigger: 
In the 'Measurement' tab you can set the sti (scan trigger input) and bti (block trigger input) you can change the values from STimer or BTimer to I, S1 or S2, depending on where you have connected your trigger to the PCIe-Board. The associated stimer/btimer field should gray out when changing the input. 

##### Use STimer and/or BTimer:
If you want to use the internal timer, you can set STimer or BTimer. Set the sti/bti to STimer and BTimer and configure the time at which it should trigger at the stimer/btimer field. The time is displayed as microseconds, so if you want to trigger every millisecond, the value should be 1.000 microseconds. Use stimer to set the time between 2 reads (exposure time for the sensor). The btimer is only needed if the blocks are > 1. In this case btime sets the distance between 2 blocks.

#### Camera Setup
The 'Camera Setup' tab is where you can configure escam to your camera. 

##### SENSOR TYPE
Depending on the Type of your Sensor, pick PDA or FFT. All sensors are PDA except the FFTs with more than 1 line.

##### CAMERA SYSTEM
Depending on your Camera System, pick 3001, 3010 or 3030

##### CAMCNT
If you have 1 Camera you can leave this number at 1, if you have multiple cameras change the number to the amount of cameras you have connected to your pc. 

##### PIXEL
Depending on the Sensor you have, choose the amount of pixels your Sensor has. It is recommended to scroll through the values since there is a small set of valid numbers. 

##### cooled camera
If your camera is cooled, tick the checkbox and set the camera cooling level to your desired amount. 

##### START
Push the start button to start one block of scans. If you want a continuous run, set the loop button. After each block a new circle is started until you press the space or ESC key. ESC stops immediately, space after the block is done.

## Software structure
![software structure as block diagram](software_structure.drawio.svg)

## Software structure of Escam

![escam structure as block diagram](escam.drawio.svg)