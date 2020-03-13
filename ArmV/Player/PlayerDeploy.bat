SET head=%1
SET head=%head:"=%
SET dst=%2
SET dst=%dst:"=%
SET dst=%dst:/=\%


ECHO|SET /p="Deploy SDL2 .. "
CALL "%head%/Lib/Include/deploy.bat" "%head%\..\Extern\sdl2\lib\x86" "%dst%" "SDL2.dll"

