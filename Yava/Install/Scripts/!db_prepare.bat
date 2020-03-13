@ECHO OFF

SET PWD=%~dp0
SET Scripts=%PWD%\..\..\..\Install\Scripts
SET DbDirGlobal=%PWD%\..\..\..\Db
SET DbDirLocal=%PWD%\..\..\Db
SET DestDir=%PWD%\..\Db

SET DB_EVENT=YES

ECHO Prepare DB>"db_prepare.log"
CALL :LOG_TIME

IF NOT EXIST "%DestDir%" MD "%DestDir%"

CALL %Scripts%\!db_init.bat

REM -----------------------------------
CALL "%Scripts%\!db_prepare_functions.bat"

ECHO  done
REM ===================================

REM -----------------------------------
CALL "%Scripts%\!db_prepare_tables.bat"
CALL "%Scripts%\!db_add_tablesv.bat"

ECHO  done
REM ===================================

REM -----------------------------------
CALL "%Scripts%\!db_prepare_write.bat"

SET SourceDir=%DbDirLocal%\Write
SET DestFile=write.sql

CALL "%Scripts%\db_process"       objects
CALL "%Scripts%\db_process"       object_settings

ECHO  done
REM ===================================

CALL %Scripts%\!db_done.bat


ECHO Prepare DB done>>"db_prepare.log"
CALL :LOG_TIME
GOTO :EOF

:LOG_TIME
SET full=%TIME%
SET h=%full:~0,2%
SET m=%full:~3,2%
SET s=%full:~6,2%
SET ms=%full:~9,2%

echo %h%:%m%:%s%.%ms%>>"db_prepare.log"

GOTO :EOF

:EOF