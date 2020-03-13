@ECHO OFF

SET source=%1
SET dest=%2
SET files=(%3)
SET files=%files:"=%


IF "%files%" == "()" (
	ECHO Usage: Deploy "source path" "dest path" "file1_sub_path file2_sub_path ..."
	GOTO :EOF
)

SET UP2DATE=true

FOR %%i IN %files% DO (CALL :DiffTime "%%i")

IF NOT %UP2DATE% == true (
	FOR %%i IN %files% DO (CALL :Deploy "%%i")
) ELSE (
ECHO All files up to date
)
GOTO :EOF

:DiffTime
SET filename=%1
SET source_file="%source%\%filename%"
SET source_file=%source_file:"=%
SET dest_file="%dest%\%filename%"
SET dest_file=%dest_file:"=%
for %%x in ("%source_file%") do set source_time=%%~tx
for %%x in ("%dest_file%") do set dest_time=%%~tx
REM ECHO source_file: %source_file% time: %source_time%
REM ECHO dest_file: %dest_file% time: %dest_time%
IF "%source_time%" == "" (
  ECHO Source file not found "%source_file%"
  EXIT 1
)
IF NOT "%source_time%" == "%dest_time%" SET UP2DATE=false
GOTO :EOF

:Deploy
SET filename=%1
SET source_file="%source%\%filename%"
SET source_file=%source_file:"=%
SET dest_file="%dest%\%filename%"
SET dest_file=%dest_file:"=%
IF NOT EXIST "%source_file%" (
 ECHO file not found "%source_file%"
 EXIT -13
)
COPY "%source_file%" "%dest_file%"
GOTO :EOF
