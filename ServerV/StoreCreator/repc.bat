SET head=%1
SET head=%head:"=%
SET root=%head%/..
SET dst=%2
SET dst=%dst:"=%
SET dst=%dst:/=\%
SET scripts=storage_resize.sql storage_cell.sql storage_current_cell.sql get_current_cell.sql get_next_cell.sql


ECHO|SET /p="Deploy storage .. "
IF NOT EXIST "%dst%/Scripts" MD "%dst%/Scripts"
CALL "%head%/Lib/Include/deploy.bat" "%head%/Misc/Db/StorageV/" "%dst%/Scripts" "%scripts%"
