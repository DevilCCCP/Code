SET PWD=%~dp0

CALL "%PWD%\db_process.bat" %*

ECHO DROP TABLE IF EXISTS %SourceItem% CASCADE; 1>> "%DestDir%\rm_%DestFile%"
