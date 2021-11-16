# EBST_CAM
Software package for the line scan cameras of [Entwicklungsbüro Stresing](http://stresing.de). This package contains multiple projects for different plattforms.

project         | description                           | plattform
:---            | :---                                  | :---
ESLSCDLL        | DLL for usage with Labview. See repository [lv64](https://github.com/Entwicklungsburo-Stresing/lv64) | Windows
ESLSCDLL_pro    | DLL for specialized functions         | Windows
Jungo           | Windows driver                        | Windows
doc             | Documentation with [Doxygen](https://www.doxygen.nl/) of DLLs interface and more  | Windows, Linux
escam           | crossplattform Qt GUI written in C++  | Windows, Linux
escam_deb       | script to create .deb package         | Linux
escam_setup     | [Microsoft Visual Studio Installer Project](https://marketplace.visualstudio.com/items?itemName=VisualStudioClient.MicrosoftVisualStudio2017InstallerProjects) to create setup for escam | Windows
linux-driver    | Linux driver                          | Linux
lsc-cli         | CLI in C                              | Linux

# Windows

### Build Dependencies
* [Visual Studio](https://visualstudio.microsoft.com/) with C++ Toolbox (Plattform Toolset: v141, Windows SDK Version: 10.0.17763.0).
* [Qt Visual Studio Tools](https://marketplace.visualstudio.com/items?itemName=TheQtCompany.QtVisualStudioTools2019) + [MSVC Qt 5](https://www.qt.io/download) (optional, for Qt GUI)
* Labview Libraries Optional, for DLL usage with Labview. If you want to compile the DLL for usage with other software than Labview, set `COMPILE_FOR_LABVIEW` to `FALSE`.
* [Microsoft Visual Studio Installer Project](https://marketplace.visualstudio.com/items?itemName=VisualStudioClient.MicrosoftVisualStudio2017InstallerProjects) (optional, for creating setup.exe for escam)

### Compile
Open `EBST_CAM.sln` with Visual Studio and press build.

### Installing Windows driver
Run `install_run_as_admin.bat` from `Stresing14.00_Driver_Distribution_Package` as admin. 

### Installing escam
Compile solution first and use `escam_setup/build/setup.exe` or download `setup.zip` file from [releases](https://github.com/Entwicklungsburo-Stresing/EBST_CAM/releases), extract and execute `setup.exe`. If Microsoft Visual C++ Redistributable is missing, this setup should install it automatically.

### Using ESLSCDLL.dll and ESLSCDLL_pro.dll
For the use of `ESLSCDLL.dll` and `ESLSCDLL_pro.dll` [Microsoft Visual C++ Redistributable](https://aka.ms/vs/16/release/vc_redist.x64.exe) must be installed.

### Troubleshooting
If `MSVCP140.dll` and `VCRUNTIME140.dll` are missing, Microsoft Visual C++ Redistributable is not installed. [Install](https://aka.ms/vs/16/release/vc_redist.x64.exe) it.

# Linux

### Dependencies
* Qt 5.15 (optional, for Qt GUI)
* make
* Linux Kernel 4.19

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

### Installing

```
# install dkms
sudo apt install dkms
```
Use .deb from [releases](https://github.com/Entwicklungsburo-Stresing/EBST_CAM/releases) or create your own. First compile escam then:
```
cd escam_deb
./create_escam_deb.sh

# install e.g. for version 3.20.3
sudo apt install ./escam_3.20-3.deb

# install kernel module
sudo dkms build -m lscpcie -v 3.20-3
sudo dkms install -m lscpcie -v 3.20-3
```

