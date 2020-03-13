-- Table: object_log

-- DROP TABLE IF EXISTS object_log;

CREATE TABLE object_log
(
  _id bigserial,
  _object integer NOT NULL,
  period_start timestamp with time zone NOT NULL,
  period_end timestamp with time zone NOT NULL,
  thread_name text,
  work_name text,
  total_time integer DEFAULT 1,
  circles integer DEFAULT 0,
  work_time integer DEFAULT 0,
  longest_work integer DEFAULT 0,
  CONSTRAINT object_log_pkey PRIMARY KEY (_id),
  CONSTRAINT object_log__object_fkey FOREIGN KEY (_object)
      REFERENCES object (_id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE
)
WITH (
  OIDS=FALSE
);
ALTER TABLE object_log
  OWNER TO su;
GRANT ALL ON TABLE object_log TO su;
GRANT SELECT, UPDATE ON TABLE object_log TO usr;

-- Index: object_log__object_period_start_idx

-- DROP INDEX object_log__object_period_start_idx;

CREATE INDEX object_log__object_period_start_idx
  ON object_log
  USING btree
  (_object, period_start);
