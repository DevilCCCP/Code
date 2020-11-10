-- Function: object_log_clean_hours(integer, integer)

-- DROP FUNCTION object_log_clean_hours(integer, integer);

CREATE OR REPLACE FUNCTION object_log_clean_hours(
    object_ integer,
    period_hours_ integer)
  RETURNS void AS
$BODY$
DECLARE
  start_period_time_ timestamp with time zone;
  end_period_time_ timestamp with time zone;
BEGIN

  SELECT hours_start FROM object_log_info 
    WHERE _object=object_ INTO start_period_time_;
  end_period_time_ := date_trunc('hours', now() - interval '1 hour' * period_hours_);

  IF (start_period_time_ IS NOT NULL) THEN
   DELETE FROM object_log_hours 
     WHERE _object=object_ AND period_hour >= start_period_time_ AND period_hour < end_period_time_;
   UPDATE object_log_info SET hours_start=end_period_time_, last_clean = now()
     WHERE _object=object_;
  END IF;

END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 1000;
ALTER FUNCTION object_log_clean_hours(integer, integer)
  OWNER TO su;
