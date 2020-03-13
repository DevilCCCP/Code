-- Function: event_stat_init(integer, integer)

-- DROP FUNCTION event_stat_init(integer, integer);

CREATE OR REPLACE FUNCTION event_stat_init(object_ integer, etype_ integer)
  RETURNS integer AS
$BODY$
DECLARE
  event_ integer;
  event_stat_ integer;
BEGIN
  SELECT _id FROM event WHERE _object = object_ AND _etype = etype_ INTO event_;
  IF event_ IS NULL THEN
    INSERT INTO event (_object, _etype) VALUES(object_, etype_) RETURNING _id INTO event_;
  END IF;
  SELECT _id FROM event_stat WHERE _event = event_ INTO event_stat_;
  IF event_stat_ IS NULL THEN
    INSERT INTO event_stat (_event) VALUES(event_) RETURNING _id INTO event_stat_;
  END IF;
  RETURN event_stat_;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;
ALTER FUNCTION event_stat_init(integer, integer)
  OWNER TO su;
