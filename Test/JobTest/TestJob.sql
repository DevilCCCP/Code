-- Table: test_job

-- DROP TABLE test_job;

CREATE TABLE test_job
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

  circles integer,
  CONSTRAINT test_job_pkey PRIMARY KEY (_id)
)
WITH (
  OIDS=FALSE
);
ALTER TABLE test_job
  OWNER TO su;

-- Index: test_job_is_active_priority_iter_idx

-- DROP INDEX test_job_is_active_priority_iter_idx;

CREATE INDEX test_job_is_active_priority_iter_idx
  ON test_job
  USING btree
  (is_active DESC, priority DESC, iter);

-- Index: test_job_priority_iter_idx

-- DROP INDEX test_job_priority_iter_idx;

CREATE INDEX test_job_priority_iter_idx
  ON test_job
  USING btree
  (priority DESC, iter);

----------------------------------------------------------
-- Table: test_action

-- DROP TABLE test_action;

CREATE TABLE test_action
(
  _id bigserial NOT NULL,
  _job bigint NOT NULL,
  _oworker integer,
  expire timestamp with time zone NOT NULL,
  iter_from bigint NOT NULL,
  iter_to bigint NOT NULL,
  try integer NOT NULL DEFAULT 0,
  CONSTRAINT test_action_pkey PRIMARY KEY (_id),
  CONSTRAINT test_action__job_fkey FOREIGN KEY (_job)
      REFERENCES test_job (_id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE,
  CONSTRAINT test_action__oworker_fkey FOREIGN KEY (_oworker)
      REFERENCES object (_id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE
)
WITH (
  OIDS=FALSE
);
ALTER TABLE test_action
  OWNER TO su;

-- Index: test_action__oworker_idx

-- DROP INDEX test_action__oworker_idx;

CREATE INDEX test_action__oworker_idx
  ON test_action
  USING btree
  (_oworker);

-- Index: test_action_expire_idx

-- DROP INDEX test_action_expire_idx;

CREATE INDEX test_action_expire_idx
  ON test_action
  USING btree
  (expire);

----------------------------------------------------------
-- Function: test_job_init(integer, integer)

-- DROP FUNCTION test_job_init(integer, integer);

CREATE OR REPLACE FUNCTION test_job_init(IN object_ integer, IN seconds_ integer
                                         , OUT action_id_ bigint, OUT job_id_ bigint
                                         , OUT iter_from_ bigint, OUT iter_to_ bigint
                                         , OUT try_ integer)
  RETURNS record AS
$BODY$
BEGIN
  SELECT _id FROM test_action WHERE _oworker = object_ LIMIT 1 INTO action_id_ FOR NO KEY UPDATE;
  IF action_id_ IS NULL THEN
    SELECT _id FROM test_action WHERE expire < now() LIMIT 1 INTO action_id_ FOR NO KEY UPDATE;
  END IF;
  IF action_id_ IS NOT NULL THEN
    UPDATE test_action SET _oworker = object_, try = try + 1, expire = now() + seconds_ * '00:00:01'::interval
      WHERE _id = action_id_ RETURNING _job, iter_from, iter_to, try INTO job_id_, iter_from_, iter_to_, try_;
  END IF;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;
ALTER FUNCTION test_job_init(integer, integer)
  OWNER TO su;
----------------------------------------------------------
-- Function: test_job_take(integer, integer, bigint, bigint)

-- DROP FUNCTION test_job_take(integer, integer, bigint, bigint);

CREATE OR REPLACE FUNCTION test_job_take(IN object_ integer, IN seconds_ integer
                                    , IN iteration_count_ bigint, IN last_iteration_ bigint
                                    , OUT action_id_ bigint, OUT job_id_ bigint
                                    , OUT iter_from_ bigint, OUT iter_to_ bigint
                                    , OUT try_ integer)
  RETURNS record AS
$BODY$
BEGIN
  IF last_iteration_ >= 0 THEN
    SELECT _id, iter, iter_end FROM test_job
      WHERE is_active = TRUE AND iter < last_iteration_
      ORDER BY priority DESC, iter LIMIT 1
      INTO job_id_, iter_from_, iter_to_
      FOR NO KEY UPDATE;
  ELSE
    SELECT _id, iter, iter_end FROM test_job
      WHERE is_active = TRUE
      ORDER BY priority DESC, iter LIMIT 1
      INTO job_id_, iter_from_, iter_to_
      FOR NO KEY UPDATE;
  END IF;

  IF job_id_ IS NOT NULL THEN
    iter_from_ := iter_from_ + 1;
    IF iter_to_ > 0 AND last_iteration_ >= 0 THEN
      iter_to_ := LEAST(LEAST(iter_from_ + iteration_count_ - 1, iter_to_), last_iteration_);
    ELSIF iter_to_ > 0 THEN
      iter_to_ := LEAST(iter_from_ + iteration_count_ - 1, iter_to_);
    ELSIF last_iteration_ > 0 THEN
      iter_to_ := LEAST(iter_from_ + iteration_count_ - 1, last_iteration_);
    END IF;
    try_ := 1;

    IF iter_to_ >= iter_from_ THEN
      UPDATE test_job SET iter = iter_to_
        WHERE _id = job_id_;
      INSERT INTO test_action(_job, _oworker, iter_from, iter_to, expire, try)
        VALUES (job_id_, object_, iter_from_, iter_to_, now() + seconds_ * '00:00:01'::interval, try_)
        RETURNING _id INTO action_id_;
    ELSE
      job_id_ := 0;
    END IF;
  END IF;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;
ALTER FUNCTION test_job_take(integer, integer, bigint, bigint)
  OWNER TO su;
----------------------------------------------------------
-- Function: test_job_done(bigint, boolean)

-- DROP FUNCTION test_job_done(bigint, boolean);

CREATE OR REPLACE FUNCTION test_job_done(action_id_ bigint, success_ boolean)
  RETURNS void AS
$BODY$
DECLARE
  job_ bigint;
  iter_done_ bigint;
  done_total_ bigint;
  iter_end_ bigint;
BEGIN
  DELETE FROM test_action WHERE _id = action_id_ RETURNING _job, iter_to - iter_from + 1 INTO job_, iter_done_;
  IF success_ THEN
    UPDATE test_job SET done = done + iter_done_, active_time = now()
     WHERE _id = job_ RETURNING iter_end, done + fail INTO iter_end_, done_total_;
  ELSE
    UPDATE test_job SET fail = fail + iter_done_, active_time = now()
     WHERE _id = job_ RETURNING iter_end, done + fail INTO iter_end_, done_total_;
  END IF;
  IF iter_end_ > 0 AND done_total_ >= iter_end_ THEN
    UPDATE test_job SET is_active = false WHERE _id = job_;
  END IF;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;
ALTER FUNCTION test_job_done(bigint, boolean)
  OWNER TO su;
----------------------------------------------------------
-- Function: test_job_cancel(bigint)

-- DROP FUNCTION test_job_cancel(bigint);

CREATE OR REPLACE FUNCTION test_job_cancel(action_id_ bigint)
  RETURNS void AS
$BODY$
BEGIN
  UPDATE test_action
    SET _oworker = NULL, try = try - 1, expire = now() - '00:00:01'::interval
  WHERE _id = action_id_;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;
ALTER FUNCTION test_job_cancel(bigint)
  OWNER TO su;
