@ECHO OFF

IF NOT DEFINED DestDir (
 ECHO DestDir must be set
 EXIT -1
)

SET PWD=%~dp0


REM ----------------------------
ECHO echo|set /p=Functions ...
SET SourceDir=%PWD%\..\Db\Functions
SET DestFile=functions.sql

CALL %PWD%\db_prepare

CALL %PWD%\db_init_count.bat
CALL %PWD%\db_process object_complete
CALL %PWD%\db_process object_create
CALL %PWD%\db_process object_create_slave
CALL %PWD%\db_process object_state_init
CALL %PWD%\db_process objects_add_setting
CALL %PWD%\db_process version_is
CALL %PWD%\db_process version_set

IF DEFINED DB_EVENT (
CALL %PWD%\db_process event_init
CALL %PWD%\db_process event_stat_init
) ELSE (
CALL %PWD%\db_skip    event_init
CALL %PWD%\db_skip    event_stat_init
)
IF DEFINED DB_JOB (
CALL %PWD%\db_process job_init
CALL %PWD%\db_process job_take
CALL %PWD%\db_process job_done
) ELSE (
CALL %PWD%\db_skip          3
)
CALL %PWD%\db_validate_count.bat
