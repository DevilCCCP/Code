CREATE OR REPLACE FUNCTION object_settings_change_post() RETURNS trigger AS $BODY$
BEGIN
  UPDATE object SET revision = revision + 1 WHERE _id = NEW._object;
  RETURN NEW;
END;
$BODY$ LANGUAGE plpgsql VOLATILE SECURITY DEFINER;
ALTER FUNCTION object_settings_change_post()
  OWNER TO su;
---
CREATE TRIGGER object_settings_change_post AFTER INSERT OR UPDATE ON object_settings
  FOR EACH ROW EXECUTE PROCEDURE object_settings_change_post();
