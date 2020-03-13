-- Table: object_state

-- DROP TABLE object_state CASCADE;

CREATE TABLE object_state
(
  _id serial NOT NULL,
  _object integer NOT NULL,
  _ostype integer NOT NULL,
  state integer DEFAULT 0,
  change_time timestamp with time zone NOT NULL,
  CONSTRAINT object_state_pkey PRIMARY KEY (_id),
  CONSTRAINT object_state__object_fkey FOREIGN KEY (_object)
      REFERENCES object (_id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE,
  CONSTRAINT object_state__ostype_fkey FOREIGN KEY (_ostype)
      REFERENCES object_state_type (_id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE
)
WITH (
  OIDS=FALSE
);
ALTER TABLE object_state
  OWNER TO su;

-- Index: object_state__object__ostype_idx

-- DROP INDEX object_state__object__ostype_idx;

CREATE INDEX object_state__object__ostype_idx
  ON object_state
  USING btree
  (_object, _ostype);

GRANT ALL ON TABLE object_state TO su;
GRANT SELECT, UPDATE ON TABLE object_state TO usr;
