@ECHO OFF

SET DestDir=%1

SET PWD=%~dp0
IF NOT EXIST %DestDir% GOTO ERROR
ECHO|SET /p=%DestDir% ..
CD %DestDir%
%Qt%\bin\qmake -spec win32-msvc2010 -tp vc || GOTO Fail
ECHO %Qt%\bin\qmake -spec win32-msvc2010 -tp vc>!make.bat
CD "%PWD%\..\.."
ECHO OK

GOTO EOF

:ERROR
ECHO %DestDir% not exists

:Fail
ECHO Fail
SET /p done=Make project FAIL
EXIT 1

:EOF