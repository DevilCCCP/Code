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

call %MakeProj% LibV/CtrlV
call %MakeProj% LibV/Decoder
call %MakeProj% LibV/MediaServer
call %MakeProj% LibV/Player
call %MakeProj% LibV/Source
call %MakeProj% LibV/Storage
call %MakeProj% LibV/Va
call %MakeProj% LibV/VideoUi
