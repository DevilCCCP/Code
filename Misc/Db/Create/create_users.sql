DO
$body$
BEGIN
  IF NOT EXISTS (SELECT FROM pg_catalog.pg_user WHERE usename = '__ABBR__2') THEN
    CREATE ROLE __ABBR__2 LOGIN
     ENCRYPTED PASSWORD '`1q2w3e'
     NOSUPERUSER INHERIT NOCREATEDB CREATEROLE REPLICATION;
  END IF;
END
$body$;

DO
$body$
BEGIN
  IF NOT EXISTS (SELECT FROM pg_catalog.pg_user WHERE usename = '__ABBR__1') THEN
    CREATE ROLE __ABBR__1 LOGIN
     ENCRYPTED PASSWORD '`1q2w3e'
     NOSUPERUSER INHERIT NOCREATEDB NOCREATEROLE NOREPLICATION;
  END IF;
END
$body$;
