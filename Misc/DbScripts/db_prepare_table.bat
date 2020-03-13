SET PWD=%~dp0

CALL "%PWD%\db_prepare.bat"

ECHO -- AUTO generated script -- 1> "%DestDir%\rm_%DestFile%"
