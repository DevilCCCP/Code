@ECHO OFF

SET pwd=%~dp0
SET dst=%pwd%\..\..\bin\release
SET upd=%pwd%\..\..\Db\Update
SET srt=%pwd%\..\..\Install\Scripts

SET Qt=%Qt5%
SET PATH=%PATH%;%Qt%\bin


IF NOT DEFINED ProjName (
 ECHO ProjName must be defined
 EXIT 1
)

ECHO D .>%dst%\.info
CD %dst%
FOR %%f in (%ProjName%_*.exe) DO (
 ECHO X %%f>>%dst%\.info
)

ECHO D Scripts>>%dst%\.info
CD %srt%
FOR /L %%i IN (1,1,9) DO (
 IF EXIST up_00%%i.bat (
  COPY /Y up_00%%i.bat %dst%\Scripts\*
  ECHO S up_00%%i.bat>>%dst%\.info
 )
)

FOR /L %%i IN (10,1,99) DO (
 IF EXIST up_0%%i.bat (
  COPY /Y up_0%%i.bat %dst%\Scripts\*
  ECHO S up_0%%i.bat>>%dst%\.info
 )
)

ECHO D Updates>>%dst%\.info
CD %upd%
FOR /L %%i IN (1,1,9) DO (
 IF EXIST up_00%%i.sql (
  COPY /Y up_00%%i.sql %dst%\Updates\*
  ECHO Q up_00%%i.sql>>%dst%\.info
 )
)

FOR /L %%i IN (10,1,99) DO (
 IF EXIST up_0%%i.sql (
  COPY /Y up_0%%i.sql %dst%\Updates\*
  ECHO Q up_0%%i.sql>>%dst%\.info
 )
)

ECHO D .>>%dst%\.info
ECHO F Version.ini>>%dst%\.info
ECHO I %ProjName%_inst.exe>>%dst%\.info

IF "%License%"=="y" (
 IF NOT EXIST %dst%\Var\key.ini (
  %dst%\License.exe
  IF NOT EXIST %dst%\Var MD %dst%\Var
  COPY %dst%\key.ini %dst%\Var\key.ini
 )
)
