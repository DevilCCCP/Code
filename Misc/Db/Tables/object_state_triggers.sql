--DROP TRIGGER IF EXISTS object_state_update_post ON object_state;
--DROP FUNCTION object_state_update_post();
---
CREATE OR REPLACE FUNCTION object_state_update_post() RETURNS trigger AS $BODY$
BEGIN
  IF (OLD.state <> NEW.state) THEN
    INSERT INTO object_state_log (_ostate, old_state, new_state, change_time)
     VALUES (NEW._id, OLD.state, NEW.state, NEW.change_time);
  END IF;
  RETURN NEW;
END;
$BODY$ LANGUAGE plpgsql VOLATILE SECURITY DEFINER;
ALTER FUNCTION object_state_update_post()
  OWNER TO su;
--GRANT EXECUTE ON FUNCTION object_state_update_post(integer, integer, integer, integer) TO usr;
---
CREATE TRIGGER object_state_update_post AFTER UPDATE ON object_state
  FOR EACH ROW EXECUTE PROCEDURE object_state_update_post();
---
/*---
DROP TRIGGER IF EXISTS object_state_add_post ON object_state;
DROP FUNCTION object_state_add_post();
---
CREATE OR REPLACE FUNCTION object_state_add_post() RETURNS trigger AS $BODY$
BEGIN
  INSERT INTO object_state_log (_ostate, old_state, new_state, change_time)
   VALUES (NEW._id, NEW.state, NEW.state, NOW());
  RETURN NEW;
END;
$BODY$ LANGUAGE plpgsql VOLATILE SECURITY DEFINER;
ALTER FUNCTION object_state_add_post()
  OWNER TO su;
---
CREATE TRIGGER object_state_add_post AFTER INSERT ON object_state
  FOR EACH ROW EXECUTE PROCEDURE object_state_add_post();
---
*/
