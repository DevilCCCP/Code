-- AUTO generated script -- 

-- object_complete -- 
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

-- object_create -- 
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

-- object_create_slave -- 
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
--

-- object_state_init -- 
-- Function: object_state_init(integer, integer, integer, integer);

-- DROP FUNCTION object_state_init(integer, integer, integer, integer);

CREATE OR REPLACE FUNCTION object_state_init(object_ integer, ostype_ integer, old_state_ integer, new_state_ integer)
  RETURNS integer AS
$BODY$
DECLARE
  id_ integer;
  real_state_ integer;
  change_time_ timestamp with time zone;
BEGIN
  LOCK TABLE object_state;
  SELECT _id, state FROM object_state WHERE _object = object_ AND _ostype = ostype_ INTO id_, real_state_;
  change_time_ = NOW();
  IF id_ IS NULL THEN
    INSERT INTO object_state (_object, _ostype, state, change_time) VALUES(object_, ostype_, new_state_, change_time_) RETURNING _id INTO id_;
    INSERT INTO object_state_log (_ostate, old_state, new_state, change_time)
     VALUES (id_, old_state_, new_state_, change_time_);
  ELSE
    IF real_state_ <> old_state_ THEN
      UPDATE object_state SET state = old_state_ WHERE _id = id_;
    END IF;
    UPDATE object_state SET state = new_state_, change_time = change_time_ WHERE _id = id_;
  END IF;
  RETURN id_;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;
ALTER FUNCTION object_state_init(integer, integer, integer, integer)
  OWNER TO su;
--
GRANT EXECUTE ON FUNCTION object_state_init(integer, integer, integer, integer) TO usr;
--

-- objects_add_setting -- 
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

-- version_is -- 
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

-- version_set -- 
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

-- event_init -- 
-- Function: event_init(integer, integer)

-- DROP FUNCTION event_init(integer, integer);

CREATE OR REPLACE FUNCTION event_init(object_ integer, etype_ integer)
  RETURNS integer AS
$BODY$
DECLARE
  id_ integer;
BEGIN
  LOCK TABLE event;
  SELECT _id FROM event WHERE _object = object_ AND _etype = etype_ INTO id_;
  IF id_ IS NULL THEN
    INSERT INTO event (_object, _etype) VALUES(object_, etype_) RETURNING _id INTO id_;
  END IF;
  RETURN id_;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;
ALTER FUNCTION event_init(integer, integer)
  OWNER TO su;

-- event_stat_init -- 
-- Function: event_stat_init(integer, integer)

-- DROP FUNCTION event_stat_init(integer, integer);

CREATE OR REPLACE FUNCTION event_stat_init(object_ integer, etype_ integer)
  RETURNS integer AS
$BODY$
DECLARE
  event_ integer;
  event_stat_ integer;
BEGIN
  SELECT _id FROM event WHERE _object = object_ AND _etype = etype_ INTO event_;
  IF event_ IS NULL THEN
    INSERT INTO event (_object, _etype) VALUES(object_, etype_) RETURNING _id INTO event_;
  END IF;
  SELECT _id FROM event_stat WHERE _event = event_ INTO event_stat_;
  IF event_stat_ IS NULL THEN
    INSERT INTO event_stat (_event) VALUES(event_) RETURNING _id INTO event_stat_;
  END IF;
  RETURN event_stat_;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;
ALTER FUNCTION event_stat_init(integer, integer)
  OWNER TO su;
