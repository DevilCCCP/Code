-- Table: object_settings

-- DROP TABLE object_settings;

CREATE TABLE object_settings
(
  _id serial NOT NULL,
  _object integer NOT NULL,
  key text NOT NULL,
  value text,
  CONSTRAINT object_settings_pkey PRIMARY KEY (_id),
  CONSTRAINT object_settings__object_fkey FOREIGN KEY (_object)
      REFERENCES object (_id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE
)
WITH (
  OIDS=FALSE
);
ALTER TABLE object_settings
  OWNER TO su;

-- Index: object_settings__object_idx
-- DROP INDEX object_settings__object_idx;
CREATE INDEX object_settings__object_idx
  ON object_settings
  USING btree
  (_object, key);

GRANT ALL ON TABLE object_settings TO su;
GRANT SELECT ON TABLE object_settings TO usr;
