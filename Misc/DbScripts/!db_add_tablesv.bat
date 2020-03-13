@ECHO OFF

IF NOT DEFINED DestDir (
 ECHO DestDir must be set
 EXIT -1
)

SET PWD=%~dp0


REM ----------------------------
SET SourceDir=%PWD%\..\Db\TablesV
SET DestFile=tables.sql

CALL %PWD%\db_init_count.bat
CALL %PWD%\db_process_table arm_monitors
CALL %PWD%\db_process_table arm_monitor_layouts
CALL %PWD%\db_process_table arm_monitor_lay_cameras

IF DEFINED DB_EVENT (
CALL %PWD%\db_process_table va_stat
CALL %PWD%\db_process_table va_stat_type
CALL %PWD%\db_process_table va_stat_hours
CALL %PWD%\db_process_table va_stat_days
) ELSE (
CALL %PWD%\db_skip          4
)

CALL %PWD%\db_validate_count.bat
