IF NOT DEFINED DestFile (
 ECHO SourceDir DestDir DestFile must be set
 EXIT -1
)

SET /a files+=1
SET SourceItem=%1
SET PWD=%~dp0

IF NOT EXIST "%SourceDir%\%SourceItem%.sql" (
 ECHO "%SourceDir%\%SourceItem%.sql" not found
 EXIT -1
)

ECHO. 1>> "%DestDir%\%DestFile%"
ECHO -- %SourceItem% -- 1>> "%DestDir%\%DestFile%"
"%PWD%\TypeEx" "%SourceDir%\%SourceItem%.sql">> "%DestDir%\%DestFile%"
