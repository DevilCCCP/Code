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

call %MakeProj% Lib/Common
call %MakeProj% Lib/Crypto
call %MakeProj% Lib/Ctrl
call %MakeProj% Lib/Db
call %MakeProj% Lib/DbUi
call %MakeProj% Lib/Dispatcher
call %MakeProj% Lib/Log
call %MakeProj% Lib/Monitoring
call %MakeProj% Lib/Net
call %MakeProj% Lib/NetServer
call %MakeProj% Lib/Settings
call %MakeProj% Lib/Updater
call %MakeProj% Lib/Ui

if "%Backup%"=="y" (
 call %MakeProj% Lib/Backup
)
if "%Reporter%"=="y" (
 call %MakeProj% Lib/Reporter
 call %MakeProj% Lib/Smtp
)
if "%Smtp%"=="y" (
 call %MakeProj% Lib/Smtp
)
if "%Unite%"=="y" (
 call %MakeProj% Lib/Unite
)
