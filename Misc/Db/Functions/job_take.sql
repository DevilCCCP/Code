----------------------------------------------------------
-- Function: xxx_job_take(integer, integer, bigint, bigint)

-- DROP FUNCTION xxx_job_take(integer, integer, bigint, bigint);

CREATE OR REPLACE FUNCTION xxx_job_take(IN object_ integer, IN seconds_ integer
                             , IN iteration_count_ bigint, IN last_iteration_ bigint
                             , OUT action_id_ bigint, OUT job_id_ bigint
                             , OUT iter_from_ bigint, OUT iter_to_ bigint
                             , OUT try_ integer)
  RETURNS record AS
$BODY$
BEGIN
  IF last_iteration_ >= 0 THEN
    SELECT _id, iter, iter_end FROM xxx_job
      WHERE is_active = TRUE AND iter < last_iteration_
      ORDER BY priority DESC, iter LIMIT 1
      INTO job_id_, iter_from_, iter_to_
      FOR NO KEY UPDATE;
  ELSE
    SELECT _id, iter, iter_end FROM xxx_job
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
      UPDATE xxx_job SET iter = iter_to_
        WHERE _id = job_id_;
      INSERT INTO xxx_action(_job, _oworker, iter_from, iter_to, expire, try)
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
ALTER FUNCTION xxx_job_take(integer, integer, bigint, bigint)
  OWNER TO su;
