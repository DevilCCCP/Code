-- Table: event_log_hours

-- DROP TABLE event_log_hours;

CREATE TABLE event_log_hours
(
  _id bigserial NOT NULL,
  _event integer NOT NULL,
  triggered_hour timestamp with time zone NOT NULL,
  value real NOT NULL DEFAULT 0,
  CONSTRAINT event_log_hours_pkey PRIMARY KEY (_id),
  CONSTRAINT event_log_hours__event_fkey FOREIGN KEY (_event)
      REFERENCES event (_id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE
)
WITH (
  OIDS=FALSE
);
ALTER TABLE event_log_hours
  OWNER TO su;

-- Index: event_log_hours__event_triggered_hour_idx

-- DROP INDEX event_log_hours__event_triggered_hour_idx;

CREATE INDEX event_log_hours__event_triggered_hour_idx
  ON event_log_hours
  USING btree
  (_event, triggered_hour);

-- Index: event_log_hours_triggered_hour_idx

-- DROP INDEX event_log_hours_triggered_hour_idx;

CREATE INDEX event_log_hours_triggered_hour_idx
  ON event_log_hours
  USING btree
  (triggered_hour);

