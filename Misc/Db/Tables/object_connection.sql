-- Table: object_connection

-- DROP TABLE object_connection;

CREATE TABLE object_connection
(
  _id serial NOT NULL,
  _omaster integer,
  _oslave integer,
  type integer,
  CONSTRAINT object_connection_pkey PRIMARY KEY (_id),
  CONSTRAINT object_connection__omaster_fkey FOREIGN KEY (_omaster)
      REFERENCES object (_id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE,
  CONSTRAINT object_connection__oslave_fkey FOREIGN KEY (_oslave)
      REFERENCES object (_id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE
)
WITH (
  OIDS=FALSE
);
ALTER TABLE object_connection
  OWNER TO su;

-- Index: object_connection__omaster_idx

-- DROP INDEX object_connection__omaster_idx;

CREATE INDEX object_connection__omaster_idx
  ON object_connection
  USING btree
  (_omaster);

-- Index: object_connection__oslave_idx

-- DROP INDEX object_connection__oslave_idx;

CREATE INDEX object_connection__oslave_idx
  ON object_connection
  USING btree
  (_oslave);

GRANT ALL ON TABLE object_connection TO su;
GRANT SELECT ON TABLE object_connection TO usr;
