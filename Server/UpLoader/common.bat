SET head=%1
SET head=%head:"=%
SET dst=%2
SET dst=%dst:"=%
SET dst=%dst:/=\%
SET ExternDir=%head%/../Extern/
SET openssl=libeay32MD.dll libeay32MD.pdb ssleay32MD.dll ssleay32MD.pdb


ECHO|SET /p="Deploy OpenSSL .. "
CALL "%head%/Lib/Include/deploy.bat" "%ExternDir%/openssl/bin" "%dst%" "%openssl%"
