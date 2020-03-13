SET head=%1
SET head=%head:"=%
SET dst=%2
SET dst=%dst:"=%
SET dst=%dst:/=\%


ECHO|SET /p="Deploy mfx .. "
SET dll=libmfxsw32.dll libmfxaudiosw32.dll

CALL "%head%/Lib/Include/deploy.bat" "%INTELMEDIASDKROOT%\bin\win32" "%dst%" "%dll%"
