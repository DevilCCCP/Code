@ECHO OFF

SET Qt=%Qt5%

%Qt%\bin\qmake -spec win32-msvc2010 -tp vc
