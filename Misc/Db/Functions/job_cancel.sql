-- Function: xxx_job_cancel(bigint)

-- DROP FUNCTION xxx_job_cancel(bigint);

CREATE OR REPLACE FUNCTION xxx_job_cancel(action_id_ bigint)
  RETURNS void AS
$BODY$
BEGIN
  UPDATE xxx_action 
    SET _oworker = NULL, try = try - 1, expire = now() - '00:00:01'::interval
  WHERE _id = action_id_;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;
ALTER FUNCTION xxx_job_cancel(bigint)
  OWNER TO su;
