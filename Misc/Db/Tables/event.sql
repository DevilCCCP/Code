-- Table: event

-- DROP TABLE event;

CREATE TABLE event
(
  _id serial NOT NULL,
  _object integer NOT NULL,
  _etype integer NOT NULL,
  CONSTRAINT event_pkey PRIMARY KEY (_id),
  CONSTRAINT event__etype_fkey FOREIGN KEY (_etype)
      REFERENCES event_type (_id) MATCH SIMPLE
      ON UPDATE NO ACTION ON DELETE NO ACTION,
  CONSTRAINT event__object_fkey FOREIGN KEY (_object)
      REFERENCES object (_id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE
)
WITH (
  OIDS=FALSE
);
ALTER TABLE event
  OWNER TO su;

-- Index: event__object_idx

-- DROP INDEX event__object_idx;

CREATE INDEX event__object_idx
  ON event
  USING btree
  (_object);

