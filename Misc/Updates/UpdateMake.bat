@ECHO OFF

SET pwd=%~dp0
SET dst=%pwd%\..\..\bin\release
SET upd=%pwd%\..\..\Db\Update
SET srt=%pwd%\..\..\Install\Scripts

SET Qt=%Qt5%
SET PATH=%PATH%;%Qt%\bin


%dst%\%ProjName%_inst.exe pack %dst% %dst%\pack
IF %ERRORLEVEL% NEQ 0 (
 exit 1
)
