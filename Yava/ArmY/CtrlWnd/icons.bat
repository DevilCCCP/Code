SET head=%1
SET head=%head:"=%
SET dst=%2
SET dst=%dst:"=%
SET dst=%dst:/=\%


ECHO|SET /p="Deploy icons .. "
IF NOT EXIST "%dst%/Icons" MD "%dst%/Icons"
CALL "%head%/Lib/Include/deploy.bat" "Images" "%dst%/Icons" "Power.ico Layout0.ico Layout1.ico Layout2.ico Layout3.ico PrimeOnly.ico OtherOnly.ico"
