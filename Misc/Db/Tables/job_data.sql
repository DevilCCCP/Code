-- Table: job_data

-- DROP TABLE job_data;

CREATE TABLE job_data
(
  _id bigserial NOT NULL,
  _job bigint NOT NULL,
  iter integer NOT NULL,
  data bytea,
  CONSTRAINT job_data_pkey PRIMARY KEY (_id)
)
WITH (
  OIDS=FALSE
);
ALTER TABLE job_data
  OWNER TO su;

-- Index: job_data__job_iter_idx

-- DROP INDEX job_data__job_iter_idx;

CREATE INDEX job_data__job_iter_idx
  ON job_data
  USING btree
  (_job, iter);

