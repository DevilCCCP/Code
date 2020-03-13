@ECHO OFF

SET filename=temp.txt
SET /A index=0

:while
IF NOT EXIST %filename% GOTO break
SET /A index=index+1
SET filename=temp_%index%.txt
GOTO while
:break

ping -n 1 -6 localhost|find "::1">%filename%

FOR %%A IN (%filename%) DO SET fileSize=%%~zA

IF %fileSize% GTR 0 (
 SET /A iv6support=1
)

IF %iv6support% EQU 1 ECHO IPv6 supported
IF %iv6support% NEQ 1 ECHO IPv6 not supported
DEL %filename%