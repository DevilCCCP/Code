@ECHO OFF

SET filename=temp.txt
SET /A index=0

:while
IF NOT EXIST %filename% GOTO break
SET /A index=index+1
SET filename=temp_%index%.txt
GOTO while
:break


NET USER>%filename%

SET /A begins=0

FOR /F "tokens=*" %%z IN (%filename%) DO (
 CALL :GetUser "%%z"
)

DEL %filename%
GOTO EOL

:GetUser
 IF %begins% EQU 1 (
  FOR %%u IN (%~1) DO (
   CALL :ReadUser %%u
  )
 )
 SET line=%~1
 IF "%line:~0,6%"=="------" SET /A begins=1
GOTO EOL

:ReadUser
ECHO %1
GOTO EOL

:EOL