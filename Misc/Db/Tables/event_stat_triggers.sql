CREATE OR REPLACE FUNCTION event_stat_update_post() RETURNS trigger AS $BODY$
DECLARE
  triggered_hour_ timestamp with time zone;
  id_ bigint;
BEGIN
  triggered_hour_ := date_trunc('hours', NEW.triggered_time);
  SELECT _id FROM event_stat_hours WHERE triggered_hour = triggered_hour_ AND _event = NEW._event INTO id_;
  IF (id_ IS NULL) THEN
    INSERT INTO event_stat_hours (_event, triggered_hour, value, period)
      VALUES (NEW._event, triggered_hour_, NEW.value, NEW.period);
  ELSE
    UPDATE event_stat_hours SET value = value + NEW.value, period = period + NEW.period 
      WHERE _id = id_;
  END IF;
  RETURN NEW;
END;
$BODY$ LANGUAGE plpgsql VOLATILE SECURITY DEFINER;
ALTER FUNCTION event_stat_update_post()
  OWNER TO su;
---
CREATE TRIGGER event_stat_update_post AFTER UPDATE ON event_stat
  FOR EACH ROW EXECUTE PROCEDURE event_stat_update_post();
