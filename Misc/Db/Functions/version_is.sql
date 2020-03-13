-- Function: version_is(text)

-- DROP FUNCTION version_is(text);

CREATE OR REPLACE FUNCTION version_is(name_ text)
  RETURNS integer AS
$BODY$
DECLARE
  ver_ integer;
BEGIN
  SELECT value::integer FROM variables WHERE key=name_||'Ver' AND _object IS NULL INTO ver_;
  IF ver_ IS NULL THEN
    INSERT INTO variables (_object, key, value) VALUES (NULL, name_||'Ver', 0);
    RETURN 0;
  ELSE
    RETURN ver_;
  END IF;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;

ALTER FUNCTION version_is(text)
  OWNER TO su;

