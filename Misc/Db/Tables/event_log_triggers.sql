CREATE OR REPLACE FUNCTION event_log_insert_post() RETURNS trigger AS $BODY$
DECLARE
  triggered_hour_ timestamp with time zone;
  id_ bigint;
BEGIN
  triggered_hour_ := date_trunc('hours', NEW.triggered_time);
  SELECT _id FROM event_log_hours WHERE triggered_hour = triggered_hour_ AND _event = NEW._event INTO id_;
  IF (id_ IS NULL) THEN
    INSERT INTO event_log_hours (_event, triggered_hour, value)
      VALUES (NEW._event, triggered_hour_, NEW.value);
  ELSE
    UPDATE event_log_hours SET value = value + NEW.value WHERE _id = id_;
  END IF;
  RETURN NEW;
END;
$BODY$ LANGUAGE plpgsql VOLATILE SECURITY DEFINER;
ALTER FUNCTION event_log_insert_post()
  OWNER TO su;
---
CREATE TRIGGER event_log_insert_post AFTER INSERT ON event_log
  FOR EACH ROW EXECUTE PROCEDURE event_log_insert_post();
