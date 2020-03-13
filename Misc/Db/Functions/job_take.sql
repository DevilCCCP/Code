-- Function: job_take(integer, integer, bigint, bigint)

-- DROP FUNCTION job_take(integer, integer, bigint, bigint);

CREATE OR REPLACE FUNCTION job_take(IN object_ integer, IN seconds_ integer, IN iteration_count_ bigint, IN max_iteration_ bigint
                                    , OUT action_id_ bigint, OUT job_id_ bigint, OUT iter_from_ bigint, OUT iter_to_ bigint, OUT try_ integer)
  RETURNS record AS
$BODY$
BEGIN
  LOCK TABLE job;
  IF max_iteration_ > 0 THEN
    SELECT _id, iter, iter_end FROM job
      WHERE is_active AND iter < max_iteration_
      ORDER BY priority DESC, iter LIMIT 1
      INTO job_id_, iter_from_, iter_to_;
  ELSE
    SELECT _id, iter, iter_end FROM job
      WHERE is_active
      ORDER BY priority DESC, iter LIMIT 1
      INTO job_id_, iter_from_, iter_to_;
  END IF;

  IF job_id_ IS NOT NULL THEN
    iter_from_ := iter_from_ + 1;
    IF iter_to_ > 0 AND max_iteration_ > 0 THEN
      iter_to_ := LEAST(LEAST(iter_from_ + iteration_count_ - 1, iter_to_), max_iteration_);
    ELSIF iter_to_ > 0 THEN
      iter_to_ := LEAST(iter_from_ + iteration_count_ - 1, iter_to_);
    ELSIF max_iteration_ > 0 THEN
      iter_to_ := LEAST(iter_from_ + iteration_count_ - 1, max_iteration_);
    END IF;
    UPDATE job SET iter = iter_to_
      WHERE _id=job_id_;
    INSERT INTO job_action(_job, _oworker, iter_from, iter_to, expire)
      VALUES (job_id_, object_, iter_from_, iter_to_, now() + seconds_ * '00:00:01'::interval)
      RETURNING _id INTO action_id_;
    try_ := 0;
  END IF;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;
ALTER FUNCTION job_take(integer, integer, bigint, bigint)
  OWNER TO su;
