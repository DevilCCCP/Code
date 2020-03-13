SET head=%1
SET head=%head:"=%
SET dst=%2
SET dst=%dst:"=%
SET dst=%dst:/=\%


ECHO|SET /p="Deploy ffmpeg .. "
SET dll=avcodec-57.dll avdevice-57.dll avfilter-6.dll avformat-57.dll avutil-55.dll postproc-54.dll swresample-2.dll swscale-4.dll

CALL "%head%/Lib/Include/deploy.bat" "%head%\..\Extern\ffmpeg-3.2\bin" "%dst%" "%dll%"
