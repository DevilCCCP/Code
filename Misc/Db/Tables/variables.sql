-- Table: variables

-- DROP TABLE variables;

CREATE TABLE variables
(
  _id bigserial NOT NULL,
  _object integer,
  key text,
  value text,
  CONSTRAINT variables_pkey PRIMARY KEY (_id),
  CONSTRAINT variables__object_fkey FOREIGN KEY (_object)
      REFERENCES object (_id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE
)
WITH (
  OIDS=FALSE
);
ALTER TABLE variables
  OWNER TO su;
GRANT ALL ON TABLE variables TO su;
GRANT SELECT ON TABLE variables TO usr;

-- Index: variables__null_key_key

-- DROP INDEX variables__null_key_key;

CREATE UNIQUE INDEX variables__null_key_key
  ON variables
  USING btree
  (key COLLATE pg_catalog."default")
  WHERE _object IS NULL;

-- Index: variables__object_key_key

-- DROP INDEX variables__object_key_key;

CREATE UNIQUE INDEX variables__object_key_key
  ON variables
  USING btree
  (_object, key COLLATE pg_catalog."default")
  WHERE _object IS NOT NULL;

