# EBST_CAM
Software package for the line scan cameras of [Entwicklungsbüro Stresing](http://stresing.de). This package contains multiple projects for different plattforms.

project | description
:--- | :---
CCDExamp | Native Windows GUI written in C
CCDExamp_Setup | Installer for CCDExamp
ESLSCDLL | DLL for usage with Labview. See repository [lv64](https://github.com/Entwicklungsburo-Stresing/lv64)
ESLSCDLL_pro | DLL for specialized functions
Jungo | Windows driver
doc | Documentation with [Doxygen](https://www.doxygen.nl/) of DLLs interface and more
lsc-gui | crossplattform Qt GUI written in C++, for Windows and Linux
lsc-gui/linux/ | Linux driver

# Compiling

## Windows
Open EBST_CAM.sln with Visual Studio and press build.

### Dependencies
* Windows SDK
* [Microsoft Visual Studio Installer Projects](https://marketplace.visualstudio.com/items?itemName=visualstudioclient.MicrosoftVisualStudio2017InstallerProjects) (optional, for CCDExamp Installer)
* [Qt Visual Studio Tools](https://marketplace.visualstudio.com/items?itemName=TheQtCompany.QtVisualStudioTools2019) + [MSVC Qt 5](https://www.qt.io/download) (optional, for Qt GUI)
* Labview Libraries (optional, for DLL usage with Labview)

## Linux GUI
Two possibilities:
1. Open lsc-gui/lsc-gui.pro with [Qt Creator](https://www.qt.io/product/development-tools) and press build.
2. Or run in Terminal `qmake` and `make` in `lsc-gui/`.

## Linux CLI
```
cd lsc-gui/linux/userspace/
make all
```

## Linux driver
```
cd lsc-gui/linux/kernelspace/
make
```
