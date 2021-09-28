-- Table: xxx_job

-- DROP TABLE xxx_job;

CREATE TABLE xxx_job
(
  _id bigserial NOT NULL,
  name text,
  descr text,
  is_active boolean NOT NULL DEFAULT false,
  priority integer NOT NULL DEFAULT 0,
  iter bigint NOT NULL DEFAULT 0,
  iter_end bigint NOT NULL DEFAULT 0,
  done bigint NOT NULL DEFAULT 0,
  fail bigint NOT NULL DEFAULT 0,
  active_time timestamp with time zone,
--  place for xxx_job settings start
--  place for xxx_job settings end
  CONSTRAINT xxx_job_pkey PRIMARY KEY (_id)
)
WITH (
  OIDS=FALSE
);
ALTER TABLE xxx_job
  OWNER TO su;

-- Index: xxx_job_is_active_priority_iter_idx

-- DROP INDEX xxx_job_is_active_priority_iter_idx;

CREATE INDEX xxx_job_is_active_priority_iter_idx
  ON xxx_job
  USING btree
  (is_active DESC, priority DESC, iter);

-- Index: xxx_job_priority_iter_idx

-- DROP INDEX xxx_job_priority_iter_idx;

CREATE INDEX xxx_job_priority_iter_idx
  ON xxx_job
  USING btree
  (priority DESC, iter);

