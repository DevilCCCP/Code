-- Function: event_init(integer, integer)

-- DROP FUNCTION event_init(integer, integer);

CREATE OR REPLACE FUNCTION event_init(object_ integer, etype_ integer)
  RETURNS integer AS
$BODY$
DECLARE
  id_ integer;
BEGIN
  LOCK TABLE event;
  SELECT _id FROM event WHERE _object = object_ AND _etype = etype_ INTO id_;
  IF id_ IS NULL THEN
    INSERT INTO event (_object, _etype) VALUES(object_, etype_) RETURNING _id INTO id_;
  END IF;
  RETURN id_;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;
ALTER FUNCTION event_init(integer, integer)
  OWNER TO su;
