-- Function: object_complete(integer, integer)

-- DROP FUNCTION object_complete(integer, integer);

CREATE OR REPLACE FUNCTION object_complete(object_ integer, omaster_type_ integer)
  RETURNS boolean AS
$BODY$
DECLARE
  omaster_ integer;
  oslave_type_ integer;
  status_ integer;
BEGIN
  SELECT status, _otype FROM object WHERE _id = object_ INTO status_, oslave_type_;
  IF status_ IS NULL THEN
    RAISE EXCEPTION 'object_complete: object not exists';
  ELSIF status_ = 0 THEN
    RETURN false;
  ELSE
    UPDATE object SET status = 0 WHERE _id = object_;
    SELECT o._id FROM object_connection c
     INNER JOIN object o ON o._id = c._omaster
     WHERE _oslave = object_ AND o._otype = omaster_type_ INTO omaster_;
    IF omaster_ IS NOT NULL THEN
      SELECT SUM(ABS(o.status)) FROM object o INNER JOIN object_connection c ON c._oslave = o._id
       WHERE c._omaster = omaster_ AND o._otype = oslave_type_ INTO status_;
      IF status_ = 0 THEN
        UPDATE object SET status = 0 WHERE _id = omaster_;
      END IF;
    END IF;
    RETURN true;
  END IF;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;
ALTER FUNCTION object_complete(integer, integer)
  OWNER TO su;
--
