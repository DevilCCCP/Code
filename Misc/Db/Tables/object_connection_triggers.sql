CREATE OR REPLACE FUNCTION object_connection_change_post() RETURNS trigger AS $BODY$
BEGIN
  IF NEW._omaster = NEW._oslave THEN
    RAISE EXCEPTION 'object_connection recursion not allowed';
  END IF;
  UPDATE object o SET revision = o.revision + 1 WHERE o._id = NEW._omaster;
  RETURN NEW;
END;
$BODY$ LANGUAGE plpgsql VOLATILE SECURITY DEFINER;
ALTER FUNCTION object_connection_change_post()
  OWNER TO su;
---
CREATE TRIGGER object_connection_change_post AFTER INSERT OR UPDATE ON object_connection
  FOR EACH ROW EXECUTE PROCEDURE object_connection_change_post();

---
CREATE OR REPLACE FUNCTION object_connection_delete_post() RETURNS trigger AS $BODY$
BEGIN
  UPDATE object o SET revision = o.revision + 1 WHERE o._id = OLD._omaster;
  RETURN OLD;
END;
$BODY$ LANGUAGE plpgsql VOLATILE SECURITY DEFINER;
ALTER FUNCTION object_connection_delete_post()
  OWNER TO su;
---
CREATE TRIGGER object_connection_delete_post AFTER DELETE ON object_connection
  FOR EACH ROW EXECUTE PROCEDURE object_connection_delete_post();
