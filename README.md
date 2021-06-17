# EBST_CAM
Software package for the line scan cameras of [Entwicklungsbüro Stresing](http://stresing.de). This package contains multiple projects for different plattforms.

project         | description                           | plattform
:---            | :---                                  | :---
CCDExamp        | Native Windows GUI written in C       | Windows
CCDExamp_Setup  | Installer for CCDExamp                | Windows
ESLSCDLL        | DLL for usage with Labview. See repository [lv64](https://github.com/Entwicklungsburo-Stresing/lv64) | Windows
ESLSCDLL_pro    | DLL for specialized functions         | Windows
Jungo           | Windows driver                        | Windows
doc             | Documentation with [Doxygen](https://www.doxygen.nl/) of DLLs interface and more  | Windows, Linux
lsc-gui         | crossplattform Qt GUI written in C++  | Windows, Linux
lsc-cli         | CLI in C                              | Linux
linux-driver    | Linux driver                          | Linux

# Compiling

## Windows
Open EBST_CAM.sln with Visual Studio and press build.

### Dependencies
* Windows SDK
* [Microsoft Visual Studio Installer Projects](https://marketplace.visualstudio.com/items?itemName=visualstudioclient.MicrosoftVisualStudio2017InstallerProjects) (optional, for CCDExamp Installer)
* [Qt Visual Studio Tools](https://marketplace.visualstudio.com/items?itemName=TheQtCompany.QtVisualStudioTools2019) + [MSVC Qt 5](https://www.qt.io/download) (optional, for Qt GUI)
* Labview Libraries (optional, for DLL usage with Labview)

## Linux

### Dependencies
* Qt 5 (optional, for Qt GUI)
* make
* Linux Kernel 4.19

### Linux GUI
Two possibilities:
1. Open lsc-gui/lsc-gui.pro with [Qt Creator](https://www.qt.io/product/development-tools) and press build.
2. Or run in Terminal `qmake` and `make` in `lsc-gui/`.

### Linux CLI
```
cd lsc-cli
make
```

### Linux driver
```
cd linux-driver/kernelspace/
make
```
