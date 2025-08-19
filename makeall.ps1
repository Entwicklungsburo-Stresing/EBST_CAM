# If you want to run this script, first you need to set the execution policy in an admin power shell:
# Set-ExecutionPolicy -ExecutionPolicy Bypass
# Also you need to add the path of devenv.com and MsBuild.exe to your environment variable PATH:
# [Environment]::SetEnvironmentVariable("Path", $env:Path + ";C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\IDE;C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin", "Machine")

param(
    [ValidateSet("Build", "Rebuild")]
    [string]$buildAction = "Rebuild"
)

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
if(test-path .\ESLSCDLL\lscpciej\)
{
	rm -r .\ESLSCDLL\lscpciej\
}
mkdir .\ESLSCDLL\lscpciej\
# Build lscpciej
cd ..\lscpciej
.\makeall.ps1 -buildAction $buildAction
# Copy lscpciej source files to EBST_CAM
cd lscpciej
cp lscpciej.h ..\..\EBST_CAM\ESLSCDLL\lscpciej
cd ..\shared_src
cp es_status_codes.c ..\..\EBST_CAM\shared_src
cp es_status_codes.h ..\..\EBST_CAM\shared_src
cd ..\..\EBST_CAM
cp -r ..\lscpciej\Release\lscpciej-*\* .\ESLSCDLL\lscpciej\

# Build Escam, and setup in release config
devenv.com EBST_CAM.sln /$buildAction Release

cp -r ..\ESLSCDLL\Release\* .\Release\
# zip setup
mkdir Release\Escam-setup-$major.$pcie.$minor\
cp -r .\escam_setup\Release\* .\Release\Escam-setup-$major.$pcie.$minor\
Compress-Archive -Path .\Release\Escam-setup-$major.$pcie.$minor\ -DestinationPath .\Release\Escam-setup-$major.$pcie.$minor.zip