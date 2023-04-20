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

## Software structure
![software structure as block diagram](software_structure.drawio.svg)

## Software structure of Escam

![escam structure as block diagram](escam.drawio.svg)