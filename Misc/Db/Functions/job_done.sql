-- Function: job_done(bigint, boolean)

-- DROP FUNCTION job_done(bigint, boolean);

CREATE OR REPLACE FUNCTION job_done(action_id_ bigint, success_ boolean)
  RETURNS void AS
$BODY$
DECLARE
  job_ bigint;
  iter_done_ bigint;
  done_total_ bigint;
  iter_end_ bigint;
BEGIN
  DELETE FROM job_action WHERE _id = action_id_ RETURNING _job, iter_to - iter_from + 1 INTO job_, iter_done_;
  IF success_ THEN
    UPDATE job SET done = done + iter_done_, active_time = now()
     WHERE _id = job_ RETURNING iter_end, done + fail INTO iter_end_, done_total_;
  ELSE
    UPDATE job SET fail = fail + iter_done_, active_time = now()
     WHERE _id = job_ RETURNING iter_end, done + fail INTO iter_end_, done_total_;
  END IF;
  IF iter_end_ > 0 AND done_total_ >= iter_end_ THEN
    UPDATE job SET is_active = false WHERE _id = job_;
  END IF;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;
ALTER FUNCTION job_done(bigint, boolean)
  OWNER TO su;
