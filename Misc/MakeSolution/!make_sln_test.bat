@ECHO OFF

IF EXIST ./.root (
 ECHO Can't apply script to root directory
 EXIT 1
)
IF NOT DEFINED Qt (
 ECHO Qt must be defined
 EXIT 1
)

SET PWD=%~dp0
SET MakeProj="%PWD%\make_proj.bat"

call %MakeProj% Test/License
REM call %MakeProj% Test/KeyGenerator

if "%DbBackup%"=="y" (
 call %MakeProj% Test/DbBackup
)
