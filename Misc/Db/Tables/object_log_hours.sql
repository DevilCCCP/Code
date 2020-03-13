-- Table: object_log_hours

-- DROP TABLE IF EXISTS object_log_hours;

CREATE TABLE object_log_hours
(
  _id bigserial,
  _object integer NOT NULL,
  period_hour timestamp with time zone NOT NULL,
  thread_name text,
  work_name text,
  total_time integer DEFAULT 1,
  circles integer DEFAULT 0,
  work_time integer DEFAULT 0,
  longest_work integer DEFAULT 0,
  log_count integer DEFAULT 1,
  CONSTRAINT object_log_hours_pkey PRIMARY KEY (_id),
  CONSTRAINT object_log_hours__object_fkey FOREIGN KEY (_object)
      REFERENCES object (_id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE
)
WITH (
  OIDS=FALSE
);
ALTER TABLE object_log_hours
  OWNER TO su;
GRANT ALL ON TABLE object_log_hours TO su;
GRANT SELECT, UPDATE ON TABLE object_log_hours TO usr;

-- Index: object_log_hours__object_period_hour_idx

-- DROP INDEX object_log_hours__object_period_hour_idx;

CREATE INDEX object_log_hours__object_period_hour_idx
  ON object_log_hours
  USING btree
  (_object, period_hour);
