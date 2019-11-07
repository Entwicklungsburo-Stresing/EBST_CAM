To convert an Project with the old WDK Driver to a Project with the Jungodriver follow these Instructions:

1. copy the files from C:\WinDriver\include (if you installed Windriver from Jungo, if not copy it from another Jungoproject)

1.1 add $(SolutionDir)\Jungo in the Project Properties to the VC++ Driectories

2. copy the lscpciej_lib.c and lscpciej_lib.h from another Jungoproject or genrated by the Windriver

3. add C:\WinDriver\lib\amd64\wdapi1200.dll to the Linker in the Project Properties (Input->Additional Dependencies)

4. copy the Board.C and Board.H from an exicsting Jungo Prject

5. check Global for missing constants or variables