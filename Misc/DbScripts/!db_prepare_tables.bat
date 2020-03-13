@ECHO OFF

IF NOT DEFINED DestDir (
 ECHO DestDir must be set
 EXIT -1
)

SET PWD=%~dp0


REM ----------------------------
ECHO echo|set /p=Tables ...
SET SourceDir=%PWD%\..\Db\Tables
SET DestFile=tables.sql

CALL %PWD%\db_prepare_table

CALL %PWD%\db_init_count.bat
CALL %PWD%\db_process_table object_type
CALL %PWD%\db_process_table object
CALL %PWD%\db_process_table object_connection
CALL %PWD%\db_process       object_triggers
CALL %PWD%\db_process       object_connection_triggers

CALL %PWD%\db_process_table files

CALL %PWD%\db_process_table object_settings_type
CALL %PWD%\db_process_table object_settings
CALL %PWD%\db_process       object_settings_triggers

CALL %PWD%\db_process_table object_state_type
CALL %PWD%\db_process_table object_state_values
CALL %PWD%\db_process_table object_state
CALL %PWD%\db_process_table object_state_log
CALL %PWD%\db_process       object_state_triggers

IF DEFINED DB_EVENT (
CALL %PWD%\db_process_table event_type
CALL %PWD%\db_process_table event
CALL %PWD%\db_process_table event_log
CALL %PWD%\db_process_table event_log_hours
CALL %PWD%\db_process       event_log_triggers
CALL %PWD%\db_process_table event_stat
CALL %PWD%\db_process_table event_stat_hours
CALL %PWD%\db_process       event_stat_triggers
) ELSE (
CALL %PWD%\db_skip          8
)

IF DEFINED DB_REPORT (
CALL %PWD%\db_process_table report
CALL %PWD%\db_process_table report_files
CALL %PWD%\db_process_table report_send
) ELSE (
CALL %PWD%\db_skip          3
)

IF DEFINED DB_JOB (
CALL %PWD%\db_process_table job
CALL %PWD%\db_process_table job_action
CALL %PWD%\db_process_table job_data
) ELSE (
CALL %PWD%\db_skip          3
)

CALL %PWD%\db_process_table variables
CALL %PWD%\db_validate_count.bat
