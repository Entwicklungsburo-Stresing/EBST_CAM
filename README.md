# EBST_CAM
This repository contains software for operating the line scan cameras of [Entwicklungsbüro Stresing](http://stresing.de). There are multiple projects for different plattforms. This readme is giving an overview of these projects and contains instructions for compiling. If you are looking for more documentation about the software, look [here](https://entwicklungsburo-stresing.github.io/). Also refer to your given manual FLbook.pdf for more documentation.

project         | description                           | plattform
:---            | :---                                  | :---
ESLSCDLL        | DLL project                           | Windows
Jungo           | Library for using the Windows Driver  | Windows
doc             | Documentation with [Doxygen](https://www.doxygen.nl/) of DLLs API and more  | Windows, Linux
escam           | crossplattform [Qt](https://www.qt.io/) GUI written in C++  | Windows, Linux
escam_deb       | script to create .deb package         | Linux
escam_setup     | [Microsoft Visual Studio Installer Project](https://marketplace.visualstudio.com/items?itemName=VisualStudioClient.MicrosoftVisualStudio2017InstallerProjects) to create setup for escam | Windows
linux-driver    | Linux driver                          | Linux
lsc-cli         | CLI in C                              | Linux

# Windows

### Build Dependencies
* [Visual Studio](https://visualstudio.microsoft.com/) with C++ Toolbox (Plattform Toolset: v142, Windows SDK Version: 10.0.18362.0).
* Visual Studio Extention: [Qt Visual Studio Tools](https://marketplace.visualstudio.com/items?itemName=TheQtCompany.QtVisualStudioTools2019) + [MSVC Qt 6.5](https://www.qt.io/download) (both needed for Escam)
* Labview Libraries: Optional, for DLL usage with Labview. If you want to compile the DLL for usage with other software than Labview, set compile options "Debug" and "Release" instead of "Debug-Labview" and "Release-Labview".
* Visual Studio Extention: [Microsoft Visual Studio Installer Project](https://marketplace.visualstudio.com/items?itemName=VisualStudioClient.MicrosoftVisualStudio2017InstallerProjects) (optional, for creating setup.exe for escam)

### Compile
Open `EBST_CAM.sln` with Visual Studio and press build. There are different compiling configurations available.

configuration	| description
:---			| :---
Debug			| Debug build for escam and DLL.
Debug-Labview	| Debug build for escam and DLL with Labview libriaries for communication with Labview software.
Release			| Release build for escam and DLL. Use this for production usage for escam and for DLL usage with Python, Matlab or other usage.
Release_minimal	| Release build for escam and DLL. Use this for production usage for escam and for DLL usage with Python, Matlab or other usage when you don't need additional functions like the greyscale viewer, math functions or other additional appearing windows.
Release-Labview	| Release build for escam and DLL with Labview libriaries for communication with Labview software. Use this for production usage with Labview.

### Installing Windows driver
Run `install_run_as_admin.bat` from `Stresing14.00_Driver_Distribution_Package` as admin. 

### Installing escam
Compile solution first and use `escam_setup/build/setup.exe` or use `setup.exe` from a [release](https://github.com/Entwicklungsburo-Stresing/EBST_CAM/releases). If Microsoft Visual C++ Redistributable is missing, this setup should install it automatically.

### Using ESLSCDLL.dll
For the use of `ESLSCDLL.dll` [Microsoft Visual C++ Redistributable](https://aka.ms/vs/16/release/vc_redist.x64.exe) must be installed.

### Troubleshooting
If `MSVCP140.dll` and `VCRUNTIME140.dll` are missing, Microsoft Visual C++ Redistributable is not installed. [Install](https://aka.ms/vs/16/release/vc_redist.x64.exe) it.

# Linux

### Build Dependencies
* Qt 5.15 (for escam Qt GUI)
* make
* C++ compiler g++
* libqt5charts5-dev
* Linux Kernel 4.19
```
sudo apt install qt5-default qt5-qmake g++ libqt5charts5-dev make
```

### Run Dependencies
* libqt5charts5
```
sudo apt install libqt5charts
```

### Compile Escam
Two possibilities:
1. Open escam.pro with [Qt Creator](https://www.qt.io/product/development-tools) and press build.
2. Or run in Terminal `qmake` and `make` in `escam/`.

### Compile lsc-cli
```
cd lsc-cli
make
```

### Compile kernel module
```
cd linux-driver/kernelspace/
make
```

### Installing driver and escam

```
# install dkms
sudo apt install dkms
```
Use given .deb or create your own. First compile escam then:
```
cd escam_deb
./create_escam_deb.sh

# install e.g. for version 3.20.3
sudo apt install ./escam_3.20-3.deb

# install kernel module
sudo dkms build -m lscpcie -v 3.20-3
sudo dkms install -m lscpcie -v 3.20-3
```

### Running escam

Before running escam the driver file must be accessible for normal user (will go at some point into the driver).
```
sudo chmod 666 /dev/lscpcie0
escam
```
