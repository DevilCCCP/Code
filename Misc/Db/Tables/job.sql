-- Table: job

-- DROP TABLE job;

CREATE TABLE job
(
  _id bigserial NOT NULL,
  name text,
  descr text,
  data bytea,
  is_active boolean NOT NULL DEFAULT false,
  priority integer NOT NULL DEFAULT 0,
  iter bigint NOT NULL DEFAULT 0,
  iter_end bigint NOT NULL DEFAULT 0,
  done bigint NOT NULL DEFAULT 0,
  fail bigint NOT NULL DEFAULT 0,
  active_time timestamp with time zone,
  CONSTRAINT job_pkey PRIMARY KEY (_id)
)
WITH (
  OIDS=FALSE
);
ALTER TABLE job
  OWNER TO su;

-- Index: job_is_active_priority_iter_idx

-- DROP INDEX job_is_active_priority_iter_idx;

CREATE INDEX job_is_active_priority_iter_idx
  ON job
  USING btree
  (is_active DESC, priority DESC, iter);

-- Index: job_priority_iter_idx

-- DROP INDEX job_priority_iter_idx;

CREATE INDEX job_priority_iter_idx
  ON job
  USING btree
  (priority DESC, iter);

