@ECHO OFF

IF NOT DEFINED DestDir (
 ECHO DestDir must be set
 EXIT -1
)

DEL "%DestDir%\install.sql"