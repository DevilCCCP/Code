-- Table: event_stat

-- DROP TABLE event_stat;

CREATE TABLE event_stat
(
  _id serial NOT NULL,
  _event integer NOT NULL,
  triggered_time timestamp with time zone NOT NULL DEFAULT now(),
  value real NOT NULL DEFAULT 0,
  period integer NOT NULL DEFAULT 0,
  CONSTRAINT event_stat_pkey PRIMARY KEY (_id),
  CONSTRAINT event_stat__event_fkey FOREIGN KEY (_event)
      REFERENCES event (_id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE,
  CONSTRAINT event_stat__event_key UNIQUE (_event)
)
WITH (
  OIDS=FALSE
);
ALTER TABLE event_stat
  OWNER TO su;

-- Index: event_stat__event_triggered_time_idx

-- DROP INDEX event_stat__event_triggered_time_idx;

CREATE INDEX event_stat__event_triggered_time_idx
  ON event_stat
  USING btree
  (_event, triggered_time);

-- Index: event_stat_triggered_time_idx

-- DROP INDEX event_stat_triggered_time_idx;

CREATE INDEX event_stat_triggered_time_idx
  ON event_stat
  USING btree
  (triggered_time);

