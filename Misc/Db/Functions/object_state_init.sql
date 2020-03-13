-- Function: object_state_init(integer, integer, integer, integer);

-- DROP FUNCTION object_state_init(integer, integer, integer, integer);

CREATE OR REPLACE FUNCTION object_state_init(object_ integer, ostype_ integer, old_state_ integer, new_state_ integer)
  RETURNS integer AS
$BODY$
DECLARE
  id_ integer;
  real_state_ integer;
  change_time_ timestamp with time zone;
BEGIN
  LOCK TABLE object_state;
  SELECT _id, state FROM object_state WHERE _object = object_ AND _ostype = ostype_ INTO id_, real_state_;
  change_time_ = NOW();
  IF id_ IS NULL THEN
    INSERT INTO object_state (_object, _ostype, state, change_time) VALUES(object_, ostype_, new_state_, change_time_) RETURNING _id INTO id_;
    INSERT INTO object_state_log (_ostate, old_state, new_state, change_time)
     VALUES (id_, old_state_, new_state_, change_time_);
  ELSE
    IF real_state_ <> old_state_ THEN
      UPDATE object_state SET state = old_state_ WHERE _id = id_;
    END IF;
    UPDATE object_state SET state = new_state_, change_time = change_time_ WHERE _id = id_;
  END IF;
  RETURN id_;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;
ALTER FUNCTION object_state_init(integer, integer, integer, integer)
  OWNER TO su;
--
GRANT EXECUTE ON FUNCTION object_state_init(integer, integer, integer, integer) TO usr;
--
