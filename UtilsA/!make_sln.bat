@ECHO OFF

SET Qt=%Qt5%

SET MakePath=./Misc/MakeSolution
SET MakeProj=%MakePath%/make_proj.bat


call %MakeProj% Ui
call %MakeProj% Analyser
call %MakeProj% ImageAnalizer
call %MakeProj% FfmpegUi

%Qt%\bin\qmake -spec win32-msvc2010 -tp vc
