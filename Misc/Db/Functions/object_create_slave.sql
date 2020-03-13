-- Function: object_create_slave(integer, integer, text, text, text)

-- DROP FUNCTION object_create_slave(integer, integer, text, text, text);

CREATE OR REPLACE FUNCTION object_create_slave(omaster_ integer, otemplate_ integer, guid_ text, name_ text, descr_ text)
  RETURNS integer AS
$BODY$
DECLARE
  new_id_ integer;
BEGIN
  SELECT object_create(otemplate_, guid_, name_, descr_) INTO new_id_;
  INSERT INTO object_connection(_omaster, _oslave) VALUES (omaster_, new_id_);
  UPDATE object SET _parent=omaster_ WHERE _id=new_id_;
  RETURN new_id_;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;
ALTER FUNCTION object_create_slave(integer, integer, text, text, text)
  OWNER TO su;
--
