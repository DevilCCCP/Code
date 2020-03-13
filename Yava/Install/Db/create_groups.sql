--------------------
CREATE ROLE usr
  NOSUPERUSER INHERIT NOCREATEDB NOCREATEROLE NOREPLICATION;
--------------------
CREATE ROLE su
  NOSUPERUSER INHERIT CREATEDB NOCREATEROLE NOREPLICATION;
--------------------
GRANT usr TO yava1;
--------------------
GRANT usr TO yava2;
GRANT su TO yava2;
--------------------
