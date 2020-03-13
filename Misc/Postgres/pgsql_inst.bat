ECHO ON

SET argc=0
FOR %%x IN (%*) DO SET /A argc += 1

IF %argc% LSS 4 (
 ECHO Usage: "install.bat PgPath Product PgIp (not used) PgPort"
 EXIT -1
)

SET PgPath=%1
SET Product=%2
SET PgData=%3
SET PgPort=%4

SET PWD=%~dp0
SET PgPath=%PgPath:"=%
SET PgData=%PgData:"=%


ECHO Install Log file>"%PgPath%\install.log"
CALL :LOG_TIME
ECHO Port:%PgPort%>>"%PgPath%\install.log"
MD "%PgData%">>"%PgPath%\install.log" 2>>&1
icacls "%PgData%" /grant *S-1-1-0:F>>"%PgPath%\install.log" 2>>&1

REM pg_hba settings
CALL :LOG_TIME

CALL :test_ipv6

IF DEFINED iv6support (
 ECHO IPv6 supported>>"%PgPath%\install.log" 2>>&1
 COPY "%PWD%\pg_hba6.conf" "%PWD%\pg_hba.conf">>"%PgPath%\install.log" 2>>&1
) else (
 ECHO IPv6 not supported>>"%PgPath%\install.log" 2>>&1
 COPY "%PWD%\pg_hba4.conf" "%PWD%\pg_hba.conf">>"%PgPath%\install.log" 2>>&1
)
ECHO # IPv4 connections:>>"%PWD%\pg_hba.conf"
ECHO host	%Product%db	%Product%1	0.0.0.0/0	md5>>"%PWD%\pg_hba.conf"
ECHO host	%Product%db	%Product%2	0.0.0.0/0	md5>>"%PWD%\pg_hba.conf"
ECHO host	%Product%db	+su	0.0.0.0/0	md5>>"%PWD%\pg_hba.conf"
ECHO host	%Product%db	+usr	0.0.0.0/0	md5>>"%PWD%\pg_hba.conf"
IF DEFINED iv6support (
ECHO # IPv6 connections:>>"%PWD%\pg_hba.conf"
ECHO host	%Product%db	%Product%1	::/0     	md5>>"%PWD%\pg_hba.conf"
ECHO host	%Product%db	%Product%2	::/0     	md5>>"%PWD%\pg_hba.conf"
ECHO host	%Product%db	+su	::/0     	md5>>"%PWD%\pg_hba.conf"
ECHO host	%Product%db	+usr	::/0     	md5>>"%PWD%\pg_hba.conf"
ECHO #-------------------------------------->>"%PWD%\pg_hba.conf"
)

CALL :LOG_TIME
ECHO init DB>>"%PgPath%\install.log"
"%PgPath%\bin\initdb.exe" -E UTF8 --no-locale -U root --pwfile="%PWD%\pwd_su" -D "%PgData%">>"%PgPath%\install.log" 2>>&1
icacls "%PgData%" /remove *S-1-1-0>>"%PgPath%\install.log" 2>>&1
ECHO Replace hba>>"%PgPath%\install.log"
COPY /Y "%PWD%\pg_hba.conf" "%PgData%">>"%PgPath%\install.log" 2>>&1
ECHO Replace config>>"%PgPath%\install.log"
IF EXIST "%PWD%\postgresql.conf" COPY /Y "%PWD%\postgresql.conf" "%PgData%\postgresql.conf">>"%PgPath%\install.log" 2>>&1
IF EXIST "%PWD%\postgresqlb.conf" COPY /Y "%PWD%\postgresqlb.conf" "%PgData%\postgresql.conf">>"%PgPath%\install.log" 2>>&1
ECHO Change old Moscow timezone>>"%PgPath%\install.log"
COPY /Y "%PWD%\Moscow" "%PgPath%\share\timezone\Europe\Moscow">>"%PgPath%\install.log" 2>>&1

CALL :LOG_TIME
ECHO register DB claster>>"%PgPath%\install.log"
"%PgPath%\bin\pg_ctl.exe" register -N "%Product%_pgsql" -D "%PgData%" -o "-i -p %PgPort%" -w>>"%PgPath%\install.log" 2>>&1
rem "%PgPath%\bin\pg_ctl.exe" -D "%PgData%" -w -o "-p %PgPort%" -l log start>>"%PgPath%\install.log"

CALL :LOG_TIME
net start %Product%_pgsql>>"%PgPath%\install.log" 2>>&1

CALL :LOG_TIME
ECHO executing scripts>>"%PgPath%\install.log"
IF NOT EXIST "%APPDATA%\postgresql" MD "%APPDATA%\postgresql">>"%PgPath%\install.log" 2>>&1
COPY /Y "%APPDATA%\postgresql\pgpass.conf" "%PWD%\">>"%PgPath%\install.log" 2>>&1
COPY /Y "%PWD%\pwd.conf" "%APPDATA%\postgresql\pgpass.conf">>"%PgPath%\install.log" 2>>&1
ECHO executing roles>>"%PgPath%\install.log"
"%PgPath%\bin\psql.exe" --host=127.0.0.1 --port=%PgPort% --username=root --file=create_roles.sql --dbname=postgres>>"%PgPath%\install.log" 2>>&1
ECHO executing groups>>"%PgPath%\install.log"
"%PgPath%\bin\psql.exe" --host=127.0.0.1 --port=%PgPort% --username=root --file=create_groups.sql --dbname=postgres>>"%PgPath%\install.log" 2>>&1

CALL :LOG_TIME
ECHO executing db create>>"%PgPath%\install.log"
"%PgPath%\bin\psql.exe" --host=127.0.0.1 --port=%PgPort% --username=root --file=create_db.sql --dbname=postgres>>"%PgPath%\install.log" 2>>&1

CALL :LOG_TIME
ECHO executing install script>>"%PgPath%\install.log"
"%PgPath%\bin\psql.exe" --host=127.0.0.1 --port=%PgPort% --username=root --file=install.sql --dbname=%Product%db>>"%PgPath%\install.log" 2>>&1
DEL /F /Q "%APPDATA%\postgresql\pgpass.conf">>"%PgPath%\install.log" 2>>&1
MOVE /Y "%PWD%\pgpass.conf" "%APPDATA%\postgresql\pgpass.conf">>"%PgPath%\install.log" 2>>&1

CALL :LOG_TIME
IF EXIST ..\Var COPY connect2.ini ..\Var\connection.ini 2>>&1
ECHO clearing>>"%PgPath%\install.log"

REM MD debug
REM COPY *.sql debug\*.*
REM COPY pwd*.* debug\*.*

DEL /F /Q create_roles.sql>>"%PgPath%\install.log" 2>>&1
DEL /F /Q pwd_su>>"%PgPath%\install.log" 2>>&1
DEL /F /Q pwd_user>>"%PgPath%\install.log" 2>>&1
DEL /F /Q pwd.conf>>"%PgPath%\install.log" 2>>&1
DEL /F /Q pg_hba.conf>>"%PgPath%\install.log" 2>>&1

GOTO :EOF

:LOG_TIME
SET full=%TIME%
SET h=%full:~0,2%
SET m=%full:~3,2%
SET s=%full:~6,2%
SET ms=%full:~9,2%

echo %h%:%m%:%s%.%ms%>>"%PgPath%\install.log"

GOTO :EOF

:test_ipv6
ping -n 1 -6 localhost|find "::1">ping.txt

FOR %%A IN (ping.txt) DO SET fileSize=%%~zA

IF %fileSize% GTR 0 (
 SET /A iv6support=1
)

GOTO :EOF

:EOF