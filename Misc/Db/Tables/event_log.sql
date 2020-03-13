-- Table: event_log

-- DROP TABLE event_log;

CREATE TABLE event_log
(
  _id bigserial NOT NULL,
  _event integer NOT NULL,
  triggered_time timestamp with time zone NOT NULL,
  value real NOT NULL DEFAULT 1,
  info text,
  CONSTRAINT event_log_pkey PRIMARY KEY (_id),
  CONSTRAINT event_log__event_fkey FOREIGN KEY (_event)
      REFERENCES event (_id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE
)
WITH (
  OIDS=FALSE
);
ALTER TABLE event_log
  OWNER TO su;

-- Index: event_log__event_triggered_time_idx

-- DROP INDEX event_log__event_triggered_time_idx;

CREATE INDEX event_log__event_triggered_time_idx
  ON event_log
  USING btree
  (_event, triggered_time);

-- Index: event_log_triggered_time_idx

-- DROP INDEX event_log_triggered_time_idx;

CREATE INDEX event_log_triggered_time_idx
  ON event_log
  USING btree
  (triggered_time);

