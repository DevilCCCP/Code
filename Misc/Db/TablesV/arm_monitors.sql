-- Table: arm_monitors

-- DROP TABLE arm_monitors CASCADE;

CREATE TABLE arm_monitors
(
  _id serial NOT NULL,
  _object integer,
  name text NOT NULL,
  descr text NOT NULL,
  num integer NOT NULL,
  width integer NOT NULL,
  height integer NOT NULL,
  size point DEFAULT '(0,0)'::point,
  used boolean DEFAULT FALSE,
  CONSTRAINT arm_monitors_pkey PRIMARY KEY (_id),
  CONSTRAINT arm_monitors__object_fkey FOREIGN KEY (_object)
      REFERENCES object (_id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE
)
WITH (
  OIDS=FALSE
);
ALTER TABLE arm_monitors
  OWNER TO su;

-- Index: arm_monitors__object_idx

-- DROP INDEX arm_monitors__object_idx;

CREATE INDEX arm_monitors__object_idx
  ON arm_monitors
  USING btree
  (_object);

GRANT ALL ON TABLE arm_monitors TO su;
GRANT SELECT ON TABLE arm_monitors TO usr;
