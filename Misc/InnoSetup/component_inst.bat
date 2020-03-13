@ECHO OFF

SET component=%1
SET name=%2
SET guid=%3
SET ip=%4

SET filename = Var/%component%

ECHO [General]>%filename%
ECHO Name=%name%>>%filename%
ECHO Guid=%guid%>>%filename%
ECHO IP=%ip%>>%filename%
ECHO Updated=false>>%filename%
