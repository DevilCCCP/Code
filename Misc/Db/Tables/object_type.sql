-- Table: object_type

-- DROP TABLE object_type;

CREATE TABLE object_type
(
  _id serial NOT NULL,
  _odefault integer,
  name text NOT NULL,
  descr text,
  CONSTRAINT object_type_pkey PRIMARY KEY (_id)
)
WITH (
  OIDS=FALSE
);
ALTER TABLE object_type
  OWNER TO su;

-- Index: object_type_name_idx

-- DROP INDEX object_type_name_idx;

CREATE INDEX object_type_name_idx
  ON object_type
  USING btree
  (name COLLATE pg_catalog."default");

GRANT ALL ON TABLE object_type TO su;
GRANT SELECT ON TABLE object_type TO usr;

-- Constraint: object_type_name_key

-- ALTER TABLE object_type DROP CONSTRAINT object_type_name_key;

ALTER TABLE object_type
  ADD CONSTRAINT object_type_name_key UNIQUE(name);

