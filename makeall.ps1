# If you want to run this script, first you need to set the execution policy in an admin power shell:
# Set-ExecutionPolicy -ExecutionPolicy Bypass
# Also you need to add the path of devenv.com and MsBuild.exe to your environment variable PATH:
# [Environment]::SetEnvironmentVariable("Path", $env:Path + ";C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\IDE;C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin", "Machine")

# Remove DLL target dir
if(test-path ./x64/)
{
	rm -r ./x64/
}
# Build Escam, DLL and setup in release config
devenv.com EBST_CAM.sln /Rebuild Release
# Build DLL all other configs
MSBuild.exe .\ESLSCDLL\ESLSCDLL.vcxproj /p:Configuration=Debug /p:Platform=x64
MSBuild.exe .\ESLSCDLL\ESLSCDLL.vcxproj /p:Configuration=Debug_minimal /p:Platform=x64
MSBuild.exe .\ESLSCDLL\ESLSCDLL.vcxproj /p:Configuration=Debug-Labview /p:Platform=x64
MSBuild.exe .\ESLSCDLL\ESLSCDLL.vcxproj /p:Configuration=Release_minimal /p:Platform=x64
MSBuild.exe .\ESLSCDLL\ESLSCDLL.vcxproj /p:Configuration=Release-Labview /p:Platform=x64
# Recreate Release folder
if(test-path Release)
{
	rm -r Release
}
mkdir Release
# zip DLL
$string = Get-Content .\version.h | Select-String -Pattern "#define VERSION_MAJOR_ESCAM"
$major = $string -replace "[^0-9]", ''
$string = Get-Content .\version.h | Select-String -Pattern "#define VERSION_PCIE_BOARD_VERSION"
$pcie = $string -replace "[^0-9]", ''
$string = Get-Content .\version.h | Select-String -Pattern "#define VERSION_MINOR_ESCAM"
$minor = $string -replace "[^0-9]", ''
mkdir Release\ESLSCDLL-$major.$pcie.$minor\
cp -r .\x64\* .\Release\ESLSCDLL-$major.$pcie.$minor\
Compress-Archive -Path .\Release\ESLSCDLL-$major.$pcie.$minor\ -DestinationPath .\Release\ESLSCDLL-$major.$pcie.$minor.zip
# zip setup
mkdir Release\Escam-setup-$major.$pcie.$minor\
cp .\escam_setup\Release\* .\Release\Escam-setup-$major.$pcie.$minor\
Compress-Archive -Path .\Release\Escam-setup-$major.$pcie.$minor\ -DestinationPath .\Release\Escam-setup-$major.$pcie.$minor.zip