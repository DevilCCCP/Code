@ECHO OFF

IF NOT DEFINED DestDir (
 ECHO DestDir must be set
 EXIT -1
)

SET PWD=%~dp0
IF DEFINED Lang (
 SET Suffix=%Lang%
) ELSE (
 SET Suffix=Ru
)

REM ----------------------------
ECHO echo|set /p=Write ...
SET SourceDir=%PWD%\..\Db\Write%Suffix%
SET DestFile=write.sql

CALL %PWD%\db_prepare
CALL %PWD%\db_init_count.bat
CALL %PWD%\db_process object_states
CALL %PWD%\db_validate_count.bat
