CREATE OR REPLACE FUNCTION object_change_post() RETURNS trigger AS $BODY$
BEGIN
  UPDATE object o SET revision = o.revision + 1
    FROM object_connection c
    WHERE o._id = c._omaster AND c._oslave = NEW._id;
  RETURN NEW;
END;
$BODY$ LANGUAGE plpgsql VOLATILE SECURITY DEFINER;
ALTER FUNCTION object_change_post()
  OWNER TO su;
---
CREATE TRIGGER object_change_post AFTER INSERT OR UPDATE ON object
  FOR EACH ROW EXECUTE PROCEDURE object_change_post();
