devenv.com EBST_CAM.sln /Rebuild Release
MSBuild.exe .\ESLSCDLL\ESLSCDLL.vcxproj /p:Configuration=Debug /p:Platform=x64
MSBuild.exe .\ESLSCDLL\ESLSCDLL.vcxproj /p:Configuration=Debug_minimal /p:Platform=x64
MSBuild.exe .\ESLSCDLL\ESLSCDLL.vcxproj /p:Configuration=Debug-Labview /p:Platform=x64
MSBuild.exe .\ESLSCDLL\ESLSCDLL.vcxproj /p:Configuration=Release_minimal /p:Platform=x64
MSBuild.exe .\ESLSCDLL\ESLSCDLL.vcxproj /p:Configuration=Release-Labview /p:Platform=x64
