-- Function: objects_add_setting(integer, text, text)

-- DROP FUNCTION objects_add_setting(integer, text, text);

CREATE OR REPLACE FUNCTION objects_add_setting(otype_ integer, key_ text, value_ text)
  RETURNS void AS
$BODY$
BEGIN
  INSERT INTO object_settings(_object, key, value) (SELECT _id, key_, value_ FROM object WHERE _otype = otype_);
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;
ALTER FUNCTION objects_add_setting(integer, text, text)
  OWNER TO su;
--
