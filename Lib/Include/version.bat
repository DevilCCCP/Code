SET PWD=%~dp0
SET DestDir=%1
SET DestDir=%DestDir:"=%
SET DestFile=%2
SET LocalDir=%PWD%\..\..\Local


ECHO Deploy versions
svnversion ../.. >"%DestDir%/%DestFile%.ver"
svnversion ../.. >./%DestFile%.ver

"%PWD%\deploy.bat" "%LocalDir%" "%DestDir%" Version.ini
