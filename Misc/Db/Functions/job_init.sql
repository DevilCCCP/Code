----------------------------------------------------------
-- Function: xxx_job_init(integer, integer)

-- DROP FUNCTION xxx_job_init(integer, integer);

CREATE OR REPLACE FUNCTION xxx_job_init(IN object_ integer, IN seconds_ integer
                             , OUT action_id_ bigint, OUT job_id_ bigint
                             , OUT iter_from_ bigint, OUT iter_to_ bigint
                             , OUT try_ integer)
  RETURNS record AS
$BODY$
BEGIN
  SELECT _id FROM xxx_action WHERE _oworker = object_ LIMIT 1 INTO action_id_ FOR NO KEY UPDATE;
  IF action_id_ IS NULL THEN
    SELECT _id FROM xxx_action WHERE expire < now() LIMIT 1 INTO action_id_ FOR NO KEY UPDATE;
  END IF;
  IF action_id_ IS NOT NULL THEN
    UPDATE xxx_action SET _oworker = object_, try = try + 1, expire = now() + seconds_ * '00:00:01'::interval
      WHERE _id = action_id_ RETURNING _job, iter_from, iter_to, try INTO job_id_, iter_from_, iter_to_, try_;
  END IF;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;
ALTER FUNCTION xxx_job_init(integer, integer)
  OWNER TO su;
