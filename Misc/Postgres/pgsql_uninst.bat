SET PgPath=%1
SET Product=%2

SET PgPath=%PgPath:"=%


net stop %Product%_pgsql
"%PgPath%\bin\pg_ctl.exe" unregister -N "%Product%_pgsql" -w
