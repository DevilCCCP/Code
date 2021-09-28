-- Table: xxx_action

-- DROP TABLE xxx_action;

CREATE TABLE xxx_action
(
  _id bigserial NOT NULL,
  _job bigint NOT NULL,
  _oworker integer,
  expire timestamp with time zone NOT NULL,
  iter_from bigint NOT NULL,
  iter_to bigint NOT NULL,
  try integer NOT NULL DEFAULT 0,
  CONSTRAINT xxx_action_pkey PRIMARY KEY (_id),
  CONSTRAINT xxx_action__job_fkey FOREIGN KEY (_job)
      REFERENCES xxx_job (_id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE,
  CONSTRAINT xxx_action__oworker_fkey FOREIGN KEY (_oworker)
      REFERENCES object (_id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE
)
WITH (
  OIDS=FALSE
);
ALTER TABLE xxx_action
  OWNER TO su;

-- Index: xxx_action__oworker_idx

-- DROP INDEX xxx_action__oworker_idx;

CREATE INDEX xxx_action__oworker_idx
  ON xxx_action
  USING btree
  (_oworker);

-- Index: xxx_action_expire_idx

-- DROP INDEX xxx_action_expire_idx;

CREATE INDEX xxx_action_expire_idx
  ON xxx_action
  USING btree
  (expire);

