@ECHO OFF

SET argc=0
FOR %%x IN (%*) DO SET /A argc += 1

IF %argc% LSS 1 (
 ECHO Usage: "!build_project.bat Product"
 EXIT -1
)

SET ProjName=%1
SET PWD=%~dp0

ECHO Build>"build.log"
CALL :LOG_TIME
"%VS100COMNTOOLS%\..\IDE\devenv.exe" %PWD%\..\..\%ProjName%.sln /build "Release|Win32"
SET RetCode=%ERRORLEVEL%
IF %RetCode% NEQ 0 (
 ECHO Build fail>>"build.log"
 EXIT -1
)
ECHO Build done>>"build.log"
CALL :LOG_TIME

GOTO :EOF

:LOG_TIME
SET full=%TIME%
SET h=%full:~0,2%
SET m=%full:~3,2%
SET s=%full:~6,2%
SET ms=%full:~9,2%

echo %h%:%m%:%s%.%ms%>>"build.log"

GOTO :EOF

:EOF