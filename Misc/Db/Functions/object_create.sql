-- Function: object_create(integer, text, text, text)

-- DROP FUNCTION object_create(integer, text, text, text);

CREATE OR REPLACE FUNCTION object_create(otemplate_ integer, guid_ text, name_ text, descr_ text)
  RETURNS integer AS
$BODY$
DECLARE
  otype_ integer;
  object_ integer;
  uri_ text;
  status_ integer;
BEGIN
  SELECT _otype, uri, status FROM object WHERE otemplate_ = _id INTO otype_, uri_, status_;
  IF otype_ IS NULL THEN
    RAISE EXCEPTION 'object_create: template not defined';
  END IF;
  INSERT INTO object(_otype, guid, name, descr, uri, status) VALUES (otype_, guid_, name_, descr_, uri_, status_) RETURNING _id INTO object_;

  INSERT INTO object_settings(_object, key, value) 
    (SELECT object_, key, value FROM object_settings WHERE _object = otemplate_ ORDER BY _id);
  RETURN object_;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;
ALTER FUNCTION object_create(integer, text, text, text)
  OWNER TO su;
--
