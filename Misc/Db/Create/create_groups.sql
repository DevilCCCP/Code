--------------------
DO $body$ BEGIN
  IF NOT EXISTS (SELECT * FROM pg_catalog.pg_group WHERE groname = 'usr') THEN
    CREATE ROLE usr NOSUPERUSER INHERIT NOCREATEDB NOCREATEROLE NOREPLICATION;
  END IF;
END $body$;
--------------------
DO $body$ BEGIN
  IF NOT EXISTS (SELECT * FROM pg_catalog.pg_group WHERE groname = 'su') THEN
    CREATE ROLE su NOSUPERUSER INHERIT CREATEDB CREATEROLE NOREPLICATION;
  END IF;
END $body$;
--------------------
GRANT usr TO __ABBR__1;
--------------------
GRANT usr TO __ABBR__2;
GRANT su TO __ABBR__2;
--------------------
