SET head=%1
SET head=%head:"=%
SET dst=%2
SET dst=%dst:"=%
SET dst=%dst:/=\%


IF NOT EXIST "%dst%/Updates" MD "%dst%/Updates"
