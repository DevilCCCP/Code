-- Function: storage_resize(integer)

-- DROP FUNCTION storage_resize(integer);

CREATE OR REPLACE FUNCTION %username%.get_current_cell(unit_ integer)
  RETURNS integer AS $BODY$
DECLARE
  id integer;
  new_cell integer;
  high_cell integer;
BEGIN
  LOCK TABLE %username%.storage_current_cell;
  SELECT _unit, current_cell FROM %username%.storage_current_cell WHERE _unit = unit_ INTO id, new_cell;
  IF new_cell IS NULL THEN
    SELECT current_cell, last_cell FROM %username%.storage_current_cell WHERE _unit = 0 INTO new_cell, high_cell;
    IF new_cell >= high_cell THEN
      UPDATE %username%.storage_current_cell SET current_cell = 1 WHERE _unit = 0;
    ELSE
      UPDATE %username%.storage_current_cell SET current_cell = new_cell + 1 WHERE _unit = 0;
    END IF;

    IF id IS NULL THEN
      INSERT INTO %username%.storage_current_cell (_unit, current_cell) VALUES (unit_, new_cell);
    ELSE
      UPDATE %username%.storage_current_cell SET current_cell = new_cell WHERE _unit = unit_;
    END IF;
    UPDATE %username%.storage_cell SET _unit = unit_, start_time = NULL, end_time = NULL, condition = 0 WHERE _id = new_cell;
  END IF;
  RETURN new_cell;
END;
$BODY$  LANGUAGE plpgsql VOLATILE
  COST 100;
ALTER FUNCTION %username%.get_current_cell(integer)
  OWNER TO %username%;
