-- Table: job_action

-- DROP TABLE job_action;

CREATE TABLE job_action
(
  _id bigserial NOT NULL,
  _job bigint NOT NULL,
  _oworker integer NOT NULL,
  expire timestamp with time zone NOT NULL,
  iter_from bigint NOT NULL,
  iter_to bigint NOT NULL,
  try integer NOT NULL DEFAULT 0,
  CONSTRAINT job_action_pkey PRIMARY KEY (_id),
  CONSTRAINT job_action__job_fkey FOREIGN KEY (_job)
      REFERENCES job (_id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE,
  CONSTRAINT job_action__oworker_fkey FOREIGN KEY (_oworker)
      REFERENCES object (_id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE
)
WITH (
  OIDS=FALSE
);
ALTER TABLE job_action
  OWNER TO su;

-- Index: job_action__oworker_idx

-- DROP INDEX job_action__oworker_idx;

CREATE INDEX job_action__oworker_idx
  ON job_action
  USING btree
  (_oworker);

-- Index: job_action_expire_idx

-- DROP INDEX job_action_expire_idx;

CREATE INDEX job_action_expire_idx
  ON job_action
  USING btree
  (expire);

