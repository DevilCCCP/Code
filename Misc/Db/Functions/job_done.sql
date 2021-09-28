-- Function: xxx_job_done(bigint, boolean)

-- DROP FUNCTION xxx_job_done(bigint, boolean);

CREATE OR REPLACE FUNCTION xxx_job_done(action_id_ bigint, success_ boolean)
  RETURNS void AS
$BODY$
DECLARE
  job_ bigint;
  iter_done_ bigint;
  done_total_ bigint;
  iter_end_ bigint;
BEGIN
  DELETE FROM xxx_action WHERE _id = action_id_ RETURNING _job, iter_to - iter_from + 1 INTO job_, iter_done_;
  IF success_ THEN
    UPDATE xxx_job SET done = done + iter_done_, active_time = now()
     WHERE _id = job_ RETURNING iter_end, done + fail INTO iter_end_, done_total_;
  ELSE
    UPDATE xxx_job SET fail = fail + iter_done_, active_time = now()
     WHERE _id = job_ RETURNING iter_end, done + fail INTO iter_end_, done_total_;
  END IF;
  IF iter_end_ > 0 AND done_total_ >= iter_end_ THEN
    UPDATE xxx_job SET is_active = false WHERE _id = job_;
  END IF;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;
ALTER FUNCTION xxx_job_done(bigint, boolean)
  OWNER TO su;
