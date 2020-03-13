@ECHO OFF

IF NOT DEFINED DestDir (
 ECHO DestDir must be set
 EXIT -1
)

SET PWD=%~dp0


REM ----------------------------
ECHO echo|set /p=View ...
SET SourceDir=%PWD%\..\Db\View
SET DestFile=views.sql

CALL %PWD%\db_prepare_view

