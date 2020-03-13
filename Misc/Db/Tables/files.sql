-- Table: files

-- DROP TABLE files;

CREATE TABLE files
(
  _id bigserial NOT NULL,
  _object integer,
  name text,
  mime_type text,
  data bytea,
  CONSTRAINT files_pkey PRIMARY KEY (_id),
  CONSTRAINT files__object_fkey FOREIGN KEY (_object)
      REFERENCES object (_id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE
)
WITH (
  OIDS=FALSE
);
ALTER TABLE files
  OWNER TO su;

-- Index: files__object_idx

-- DROP INDEX files__object_idx;

CREATE INDEX files__object_idx
  ON files
  USING btree
  (_object);

