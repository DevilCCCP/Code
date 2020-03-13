@ECHO OFF
IF EXIST ../.root GOTO :WARN

DEL /S /Q /F /A *.suo
DEL /S /Q /F /A *.sdf

FOR %%d IN (Lib Server Tests) DO (
  CD %%d
  CALL :CleanSub %%d
  CD ..
)

EXIT

:CleanSub
ECHO Clean '%1'
FOR /D /r %%f IN (*) DO (
  IF /I '%%~nf'=='debug' (
    ECHO "%%f"
    DEL /Q "%%f\*"
    RMDIR "%%f"
  )
)
  
FOR /D /r %%f IN (*) DO (
  IF EXIST "%%f\Makefile" DEL /Q "%%f\Makefile"
  IF EXIST "%%f\Makefile.Debug" DEL /Q "%%f\Makefile.Debug"
  IF EXIST "%%f\Makefile.Release" DEL /Q "%%f\Makefile.Release"
)
GOTO :EOF

:WARN
ECHO Can't apply to root directory

:EOF