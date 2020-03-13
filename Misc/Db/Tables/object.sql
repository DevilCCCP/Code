-- Table: object

-- DROP TABLE object CASCADE;

CREATE TABLE object
(
  _id serial NOT NULL,
  _otype integer NOT NULL,
  _parent integer,
  guid text NOT NULL,
  name text NOT NULL,
  descr text,
  version text,
  revision integer DEFAULT 1,
  uri text,
  status integer DEFAULT 0,
  CONSTRAINT object_pkey PRIMARY KEY (_id),
  CONSTRAINT object__otype_fkey FOREIGN KEY (_otype)
      REFERENCES object_type (_id) MATCH SIMPLE
      ON UPDATE NO ACTION ON DELETE NO ACTION,
  CONSTRAINT object__parent_fkey FOREIGN KEY (_parent)
      REFERENCES object (_id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE,
  CONSTRAINT object__otype_guid_key UNIQUE (guid)
)
WITH (
  OIDS=FALSE
);
ALTER TABLE object
  OWNER TO su;

-- Index: object__otype_idx

-- DROP INDEX object__otype_idx;

CREATE INDEX object__otype_idx
  ON object
  USING btree
  (_otype);

-- Index: object_guid_idx

-- DROP INDEX object_guid_idx;

CREATE INDEX object_guid_idx
  ON object
  USING btree
  (guid COLLATE pg_catalog."default");

ALTER TABLE object_type ADD
CONSTRAINT object_type__odefault_fkey FOREIGN KEY (_odefault)
      REFERENCES object (_id) MATCH SIMPLE
      ON UPDATE NO ACTION ON DELETE NO ACTION;

GRANT ALL ON TABLE object TO su;
GRANT SELECT ON TABLE object TO usr;
