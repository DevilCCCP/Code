-- Table: xxx_job_data

-- DROP TABLE xxx_job_data;

CREATE TABLE xxx_job_data
(
  _id bigserial NOT NULL,
  _job bigint NOT NULL,
  iter integer NOT NULL,
  data bytea,
  CONSTRAINT xxx_job_data_pkey PRIMARY KEY (_id)
)
WITH (
  OIDS=FALSE
);
ALTER TABLE xxx_job_data
  OWNER TO su;

-- Index: xxx_job_data__job_iter_idx

-- DROP INDEX xxx_job_data__job_iter_idx;

CREATE INDEX xxx_job_data__job_iter_idx
  ON xxx_job_data
  USING btree
  (_job, iter);

