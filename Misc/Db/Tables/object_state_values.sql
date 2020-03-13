-- Table: object_state_values

-- DROP TABLE object_state_values;

CREATE TABLE object_state_values
(
  _id serial NOT NULL,
  _ostype integer NOT NULL,
  state integer NOT NULL,
  descr text NOT NULL,
  color text NOT NULL,
  CONSTRAINT object_state_values_pkey PRIMARY KEY (_id),
  CONSTRAINT object_state_values__ostype_fkey FOREIGN KEY (_ostype)
      REFERENCES object_state_type (_id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE
)
WITH (
  OIDS=FALSE
);
ALTER TABLE object_state_values
  OWNER TO su;

-- Index: object_state_values_name_idx

-- DROP INDEX object_state_values_name_idx;

CREATE INDEX object_state_values_name_idx
  ON object_state_values
  USING btree
  (_ostype, state);

GRANT ALL ON TABLE object_state_values TO su;
GRANT SELECT ON TABLE object_state_values TO usr;
