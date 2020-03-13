-- Function: version_set(text, integer)

-- DROP FUNCTION version_set(text, integer);

CREATE OR REPLACE FUNCTION version_set(name_ text, ver_ integer)
  RETURNS void AS
$BODY$
BEGIN
  UPDATE variables SET value = ver_::text WHERE key=name_||'Ver' AND _object IS NULL;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;

ALTER FUNCTION version_set(text, integer)
  OWNER TO su;
