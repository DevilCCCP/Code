@ECHO OFF

SET Qt=%Qt5%
SET PWD=%~dp0
SET MakeProj=:make_proj

call %MakeProj% Core
call %MakeProj% DbmCreatorUi

%Qt%\bin\qmake -spec win32-msvc2010 -tp vc

GOTO EOF


:make_proj
SET DestDir=%1

IF NOT EXIST %DestDir% GOTO ERROR
ECHO|SET /p=%DestDir% ..
CD %DestDir%
%Qt%\bin\qmake -spec win32-msvc2010 -tp vc
ECHO %Qt%\bin\qmake -spec win32-msvc2010 -tp vc>!make.bat
CD %PWD%
ECHO OK

GOTO :EOF

:ERROR
ECHO %DestDir% not exists

:EOF