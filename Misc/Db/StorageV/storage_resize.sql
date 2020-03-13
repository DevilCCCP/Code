-- Function: storage_resize(integer)

-- DROP FUNCTION storage_resize(integer);

CREATE OR REPLACE FUNCTION %username%.storage_resize(new_size_ integer)
  RETURNS void AS
$BODY$
DECLARE
  i integer;
  del_count integer;
BEGIN
  DELETE FROM %username%.storage_cell WHERE _id > new_size_;
  FOR i IN REVERSE new_size_..1 LOOP
  BEGIN
    INSERT INTO %username%.storage_cell (_id, _unit) VALUES (i, 0);
    EXCEPTION WHEN unique_violation THEN
    EXIT;
  END;
  END LOOP;
  
  UPDATE %username%.storage_current_cell SET last_cell = new_size_
    ,current_cell = CASE WHEN current_cell <= new_size_ THEN current_cell ELSE 1 END 
    WHERE _unit = 0;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;
ALTER FUNCTION %username%.storage_resize(integer)
  OWNER TO %username%;
