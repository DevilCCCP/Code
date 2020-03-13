SET PWD=%~dp0

CALL "%PWD%\db_process.bat" %*

ECHO DROP VIEW IF EXISTS %SourceItem% CASCADE; 1>> "%DestDir%\rm_tables.sql
