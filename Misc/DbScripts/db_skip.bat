IF NOT DEFINED DestFile (
 ECHO SourceDir DestDir DestFile must be set
 EXIT -1
)

SET /A count=%1

IF %count% LSS 1 (
 SET /A files+=1
) ELSE (
 SET /A files+=%count%
)