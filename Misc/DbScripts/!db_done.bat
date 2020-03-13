@ECHO OFF

IF NOT DEFINED DestDir (
 ECHO DestDir must be set
 EXIT -1
)

SET DestScript="%DestDir%\install.sql"

ECHO ECHO|SET /p=All ...

ECHO -- AUTO generated Install script -- 1> %DestScript%
ECHO. 1>> %DestScript%
ECHO BEGIN TRANSACTION; 1>> %DestScript%
FOR %%i IN ("%DestDir%\tables.sql" "%DestDir%\functions.sql" "%DestDir%\views.sql" "%DestDir%\write.sql" "%DestDir%\update.sql") DO (
  IF EXIST %%i (
    TYPE %%i 1>> %DestScript%
  )
)

ECHO. 1>> %DestScript%
ECHO -- End of Install script -- 1>> %DestScript%
ECHO END TRANSACTION; 1>> %DestScript%
ECHO  done