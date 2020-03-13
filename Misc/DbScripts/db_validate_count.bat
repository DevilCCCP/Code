IF NOT EXIST "%SourceDir%" (
 ECHO "%SourceDir%" not found
 EXIT -1
)

SET /A totalFiles=0
FOR %%f IN ("%SourceDir%\*") DO SET /A totalFiles+=1

IF %files% NEQ %totalFiles% (
 ECHO Not all scripts in dir '%SourceDir%' used [used: %files%, total: %totalFiles%]
 EXIT -1
)
