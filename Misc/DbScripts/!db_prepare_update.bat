@ECHO OFF

IF NOT DEFINED DestDir (
 ECHO DestDir must be set
 EXIT -1
)

SET PWD=%~dp0
IF DEFINED Lang (
 SET Suffix=_%Lang%
) ELSE (
 SET Suffix=_ru
)

REM ----------------------------
ECHO echo|set /p=Update ...
SET SourceDir=%PWD%\..\Db\Update
SET DestFile=update.sql

CALL %PWD%\db_prepare

CALL %PWD%\db_init_count.bat
CALL %PWD%\db_skip          2
CALL %PWD%\db_validate_count.bat
