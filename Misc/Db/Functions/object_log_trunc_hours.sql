-- Function: object_log_trunc_hours(integer, integer)

-- DROP FUNCTION object_log_trunc_hours(integer, integer);

CREATE OR REPLACE FUNCTION object_log_trunc_hours(
    object_ integer,
    period_hours_ integer)
  RETURNS void AS
$BODY$
DECLARE
  end_period_time_ timestamp with time zone;
BEGIN

  end_period_time_ := date_trunc('hours', now() - interval '1 hour' * period_hours_);
  INSERT INTO object_log_hours(_object, period_hour, thread_name, work_name, 
    total_time, circles, work_time, longest_work, log_count)
  (SELECT _object, date_trunc('hours', period_start) AS period_hour, thread_name, work_name, 
     SUM(total_time), SUM(circles), SUM(work_time), MAX(longest_work), COUNT(_id)
     FROM object_log 
     WHERE _object=object_ AND period_start < end_period_time_ 
     GROUP BY _object, period_hour, thread_name, work_name
     ORDER BY period_hour, thread_name, work_name
  );
  DELETE FROM object_log WHERE _object=object_ AND period_start < end_period_time_;

END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 1000;
ALTER FUNCTION object_log_trunc_hours(integer, integer)
  OWNER TO su;
