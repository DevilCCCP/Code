SET head=%1
SET head=%head:"=%
SET dst=%2
SET dst=%dst:"=%
SET dst=%dst:/=\%


CALL "%head%\LibV\Ffmpeg.bat" "%head%" "%dst%"
IF NOT "%INTELMEDIASDKROOT%" == "" (
 CALL "%head%\LibV\Mfx.bat" "%head%" "%dst%"
)
