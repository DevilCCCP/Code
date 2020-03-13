-- Table: event_type

-- DROP TABLE event_type;

CREATE TABLE event_type
(
  _id serial NOT NULL,
  name text NOT NULL,
  descr text NOT NULL,
  icon text,
  flag integer NOT NULL DEFAULT 1,
  CONSTRAINT event_type_pkey PRIMARY KEY (_id)
)
WITH (
  OIDS=FALSE
);
ALTER TABLE event_type
  OWNER TO su;

-- Index: event_type_name_idx

-- DROP INDEX event_type_name_idx;

CREATE INDEX event_type_name_idx
  ON event_type
  USING btree
  (name COLLATE pg_catalog."default");

-- Constraint: event_type_name_key

-- ALTER TABLE event_type DROP CONSTRAINT event_type_name_key;

ALTER TABLE event_type
  ADD CONSTRAINT event_type_name_key UNIQUE(name);
