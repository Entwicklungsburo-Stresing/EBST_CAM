# If you want to run this script, first you need to set the execution policy in a admin power shell:
# Set-ExecutionPolicy -ExecutionPolicy Bypass
# Also you need to add the path of devenv.com and MsBuild.exe to your environment variable PATH:
# [Environment]::SetEnvironmentVariable("Path", $env:Path + ";C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\IDE;C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin", "Machine")
devenv.com EBST_CAM.sln /Rebuild Release
MSBuild.exe .\ESLSCDLL\ESLSCDLL.vcxproj /p:Configuration=Debug /p:Platform=x64
MSBuild.exe .\ESLSCDLL\ESLSCDLL.vcxproj /p:Configuration=Debug_minimal /p:Platform=x64
MSBuild.exe .\ESLSCDLL\ESLSCDLL.vcxproj /p:Configuration=Debug-Labview /p:Platform=x64
MSBuild.exe .\ESLSCDLL\ESLSCDLL.vcxproj /p:Configuration=Release_minimal /p:Platform=x64
MSBuild.exe .\ESLSCDLL\ESLSCDLL.vcxproj /p:Configuration=Release-Labview /p:Platform=x64
