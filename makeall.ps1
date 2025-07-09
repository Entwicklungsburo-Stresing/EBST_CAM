# If you want to run this script, first you need to set the execution policy in an admin power shell:
# Set-ExecutionPolicy -ExecutionPolicy Bypass
# Also you need to add the path of devenv.com and MsBuild.exe to your environment variable PATH:
# [Environment]::SetEnvironmentVariable("Path", $env:Path + ";C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\IDE;C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin", "Machine")

$string = Get-Content .\version.h | Select-String -Pattern "#define VERSION_MAJOR_ESCAM"
$major = $string -replace "[^0-9]", ''
$string = Get-Content .\version.h | Select-String -Pattern "#define VERSION_PCIE_BOARD_VERSION"
$pcie = $string -replace "[^0-9]", ''
$string = Get-Content .\version.h | Select-String -Pattern "#define VERSION_MINOR_ESCAM"
$minor = $string -replace "[^0-9]", ''

# Recreate Release folder
if(test-path Release)
{
	rm -r Release
}
mkdir Release
if(test-path .\ESLSCDLL\x64\)
{
	rm -r .\ESLSCDLL\x64\
}
mkdir .\ESLSCDLL\x64\

# Build ESLSCDLL
cd ..\ESLSCDLL
.\makeall.ps1
# Copy ESLSCDLL source files to EBST_CAM
cd ESLSCDLL
cp Board.h ..\..\EBST_CAM\ESLSCDLL
cp Board.c ..\..\EBST_CAM\ESLSCDLL
cp Board_ll.h ..\..\EBST_CAM\ESLSCDLL
cp linux\Board_ll.c ..\..\EBST_CAM\ESLSCDLL\linux
cp win\Board_ll.c ..\..\EBST_CAM\ESLSCDLL\win
cp Camera.h ..\..\EBST_CAM\ESLSCDLL
cp Camera.c ..\..\EBST_CAM\ESLSCDLL
cp Direct2dViewer_c.h ..\..\EBST_CAM\ESLSCDLL
cp Direct2dViewer_c.cpp ..\..\EBST_CAM\ESLSCDLL
cp ESLSCDLL.h ..\..\EBST_CAM\ESLSCDLL
cp ESLSCDLL.c ..\..\EBST_CAM\ESLSCDLL
cp globals.h ..\..\EBST_CAM\ESLSCDLL
cp globals.c ..\..\EBST_CAM\ESLSCDLL
cp resource.h ..\..\EBST_CAM\ESLSCDLL
cp UIAbstractionLayer.h ..\..\EBST_CAM\ESLSCDLL
cp UIAbstractionLayer.c ..\..\EBST_CAM\ESLSCDLL
cd ..\shared_src
cp default_settings.h ..\..\EBST_CAM\shared_src
cp enum_hardware.h ..\..\EBST_CAM\shared_src
cp enum_settings.h ..\..\EBST_CAM\shared_src
cp es_status_codes.c ..\..\EBST_CAM\shared_src
cp es_status_codes.h ..\..\EBST_CAM\shared_src
cp struct.h ..\..\EBST_CAM\shared_src
cd ..\..\EBST_CAM
## Copy ESLSCDLL
cp -r ..\ESLSCDLL\Release\ESLSCDLL-$major.$pcie.$minor\* .\ESLSCDLL\x64\

# Build Escam, and setup in release config
devenv.com EBST_CAM.sln /Rebuild Release

cp -r ..\ESLSCDLL\Release\* .\Release\
# zip setup
mkdir Release\Escam-setup-$major.$pcie.$minor\
cp -r .\escam_setup\Release\* .\Release\Escam-setup-$major.$pcie.$minor\
Compress-Archive -Path .\Release\Escam-setup-$major.$pcie.$minor\ -DestinationPath .\Release\Escam-setup-$major.$pcie.$minor.zip