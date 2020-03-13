-- Table: object_settings_type

-- DROP TABLE object_settings_type;

CREATE TABLE object_settings_type
(
  _id serial NOT NULL,
  _otype integer,
  key text,
  name text,
  descr text,
  type text,
  min_value text,
  max_value text,
  CONSTRAINT object_settings_type_pkey PRIMARY KEY (_id),
  CONSTRAINT object_settings_type__otype_fkey FOREIGN KEY (_otype)
      REFERENCES object_type (_id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE
)
WITH (
  OIDS=FALSE
);
ALTER TABLE object_settings_type
  OWNER TO su;

-- Index: object_settings_type__otype_key_idx

-- DROP INDEX object_settings_type__otype_key_idx;

CREATE INDEX object_settings_type__otype_key_idx
  ON object_settings_type
  USING btree
  (_otype, key COLLATE pg_catalog."default");

GRANT ALL ON TABLE object_settings_type TO su;
GRANT SELECT ON TABLE object_settings_type TO usr;
