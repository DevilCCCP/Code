-- Table: object_state_type

-- DROP TABLE object_state_type;

CREATE TABLE object_state_type
(
  _id serial NOT NULL,
  name text NOT NULL,
  descr text,
  CONSTRAINT object_state_type_pkey PRIMARY KEY (_id),
  CONSTRAINT object_state_type_name_key UNIQUE (name)
)
WITH (
  OIDS=FALSE
);
ALTER TABLE object_state_type
  OWNER TO su;

-- Index: object_state_type_name_idx

-- DROP INDEX object_state_type_name_idx;

CREATE INDEX object_state_type_name_idx
  ON object_state_type
  USING btree
  (name COLLATE pg_catalog."default");

GRANT ALL ON TABLE object_state_type TO su;
GRANT SELECT ON TABLE object_state_type TO usr;

-- Constraint: object_state_type_name_key

-- ALTER TABLE object_state_type DROP CONSTRAINT object_state_type_name_key;

-- ALTER TABLE object_state_type
--  ADD CONSTRAINT object_state_type_name_key UNIQUE(name);
