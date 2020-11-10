-- Function: object_log_trunc_hours(integer, integer)

-- DROP FUNCTION object_log_trunc_hours(integer, integer);

CREATE OR REPLACE FUNCTION object_log_trunc_hours(
    object_ integer,
    period_hours_ integer)
  RETURNS void AS
$BODY$
DECLARE
  start_period_time_ timestamp with time zone;
  end_period_time_ timestamp with time zone;
BEGIN

  SELECT hours_end FROM object_log_info 
    WHERE _object=object_ INTO start_period_time_;
  end_period_time_ := date_trunc('hours', now() - interval '1 hour' * period_hours_);

  IF (start_period_time_ IS NOT NULL) THEN
   INSERT INTO object_log_hours(_object, period_hour, thread_name, work_name, 
     total_time, circles, work_time, longest_work, log_count)
   (SELECT _object, date_trunc('hours', period_start) AS period_hour, thread_name, work_name, 
     SUM(total_time)/12, SUM(circles)/12, SUM(work_time)/12, MAX(longest_work), COUNT(_id)
     FROM object_log 
     WHERE _object=object_ AND period_start >= start_period_time_ AND period_start < end_period_time_ 
     GROUP BY _object, period_hour, thread_name, work_name
     ORDER BY period_hour, thread_name, work_name
   );
   DELETE FROM object_log 
     WHERE _object=object_ AND period_start >= start_period_time_ AND period_start < end_period_time_;
   UPDATE object_log_info 
     SET hours_end=end_period_time_, last_trunc = now() 
     WHERE _object=object_;
  ELSE
   INSERT INTO object_log_hours(_object, period_hour, thread_name, work_name, 
     total_time, circles, work_time, longest_work, log_count)
   (SELECT _object, date_trunc('hours', period_start) AS period_hour, thread_name, work_name, 
     SUM(total_time)/12, SUM(circles)/12, SUM(work_time)/12, MAX(longest_work), COUNT(_id)
     FROM object_log 
     WHERE _object=object_ AND period_start < end_period_time_ 
     GROUP BY _object, period_hour, thread_name, work_name
     ORDER BY period_hour, thread_name, work_name
   );
   DELETE FROM object_log WHERE _object=object_ AND period_start < end_period_time_;
   INSERT INTO object_log_info(_object, hours_start, hours_end)
     (SELECT _object, MIN(period_hour), MAX(period_hour) FROM object_log_hours 
        WHERE _object=object_ GROUP BY _object
     );
   UPDATE object_log_info 
     SET hours_end=end_period_time_, last_trunc = now()
     WHERE _object=object_;
  END IF;

END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 1000;
ALTER FUNCTION object_log_trunc_hours(integer, integer)
  OWNER TO su;
