-- AUTO generated script -- 

-- object_type -- 
-- Table: object_type

-- DROP TABLE object_type;

CREATE TABLE object_type
(
  _id serial NOT NULL,
  _odefault integer,
  name text NOT NULL,
  descr text,
  CONSTRAINT object_type_pkey PRIMARY KEY (_id)
)
WITH (
  OIDS=FALSE
);
ALTER TABLE object_type
  OWNER TO su;

-- Index: object_type_name_idx

-- DROP INDEX object_type_name_idx;

CREATE INDEX object_type_name_idx
  ON object_type
  USING btree
  (name COLLATE pg_catalog."default");

GRANT ALL ON TABLE object_type TO su;
GRANT SELECT ON TABLE object_type TO usr;

-- Constraint: object_type_name_key

-- ALTER TABLE object_type DROP CONSTRAINT object_type_name_key;

ALTER TABLE object_type
  ADD CONSTRAINT object_type_name_key UNIQUE(name);


-- object -- 
-- Table: object

-- DROP TABLE object CASCADE;

CREATE TABLE object
(
  _id serial NOT NULL,
  _otype integer NOT NULL,
  _parent integer,
  guid text NOT NULL,
  name text NOT NULL,
  descr text,
  version text,
  revision integer DEFAULT 1,
  uri text,
  status integer DEFAULT 0,
  CONSTRAINT object_pkey PRIMARY KEY (_id),
  CONSTRAINT object__otype_fkey FOREIGN KEY (_otype)
      REFERENCES object_type (_id) MATCH SIMPLE
      ON UPDATE NO ACTION ON DELETE NO ACTION,
  CONSTRAINT object__parent_fkey FOREIGN KEY (_parent)
      REFERENCES object (_id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE,
  CONSTRAINT object__otype_guid_key UNIQUE (_otype, guid)
)
WITH (
  OIDS=FALSE
);
ALTER TABLE object
  OWNER TO su;

-- Index: object__otype_idx

-- DROP INDEX object__otype_idx;

CREATE INDEX object__otype_idx
  ON object
  USING btree
  (_otype);

-- Index: object_guid_idx

-- DROP INDEX object_guid_idx;

CREATE INDEX object_guid_idx
  ON object
  USING btree
  (guid COLLATE pg_catalog."default");

ALTER TABLE object_type ADD
CONSTRAINT object_type__odefault_fkey FOREIGN KEY (_odefault)
      REFERENCES object (_id) MATCH SIMPLE
      ON UPDATE NO ACTION ON DELETE NO ACTION;

GRANT ALL ON TABLE object TO su;
GRANT SELECT ON TABLE object TO usr;

-- object_connection -- 
-- Table: object_connection

-- DROP TABLE object_connection;

CREATE TABLE object_connection
(
  _id serial NOT NULL,
  _omaster integer,
  _oslave integer,
  type integer,
  CONSTRAINT object_connection_pkey PRIMARY KEY (_id),
  CONSTRAINT object_connection__omaster_fkey FOREIGN KEY (_omaster)
      REFERENCES object (_id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE,
  CONSTRAINT object_connection__oslave_fkey FOREIGN KEY (_oslave)
      REFERENCES object (_id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE
)
WITH (
  OIDS=FALSE
);
ALTER TABLE object_connection
  OWNER TO su;

-- Index: object_connection__omaster_idx

-- DROP INDEX object_connection__omaster_idx;

CREATE INDEX object_connection__omaster_idx
  ON object_connection
  USING btree
  (_omaster);

-- Index: object_connection__oslave_idx

-- DROP INDEX object_connection__oslave_idx;

CREATE INDEX object_connection__oslave_idx
  ON object_connection
  USING btree
  (_oslave);

GRANT ALL ON TABLE object_connection TO su;
GRANT SELECT ON TABLE object_connection TO usr;

-- object_triggers -- 
CREATE OR REPLACE FUNCTION object_change_post() RETURNS trigger AS $BODY$
BEGIN
  UPDATE object o SET revision = o.revision + 1
    FROM object_connection c
    WHERE o._id = c._omaster AND c._oslave = NEW._id;
  RETURN NEW;
END;
$BODY$ LANGUAGE plpgsql VOLATILE SECURITY DEFINER;
ALTER FUNCTION object_change_post()
  OWNER TO su;
---
CREATE TRIGGER object_change_post AFTER INSERT OR UPDATE ON object
  FOR EACH ROW EXECUTE PROCEDURE object_change_post();

-- object_connection_triggers -- 
CREATE OR REPLACE FUNCTION object_connection_change_post() RETURNS trigger AS $BODY$
BEGIN
  IF NEW._omaster = NEW._oslave THEN
    RAISE EXCEPTION 'object_connection recursion not allowed';
  END IF;
  UPDATE object o SET revision = o.revision + 1 WHERE o._id = NEW._omaster;
  RETURN NEW;
END;
$BODY$ LANGUAGE plpgsql VOLATILE SECURITY DEFINER;
ALTER FUNCTION object_connection_change_post()
  OWNER TO su;
---
CREATE TRIGGER object_connection_change_post AFTER INSERT OR UPDATE ON object_connection
  FOR EACH ROW EXECUTE PROCEDURE object_connection_change_post();

---
CREATE OR REPLACE FUNCTION object_connection_delete_post() RETURNS trigger AS $BODY$
BEGIN
  UPDATE object o SET revision = o.revision + 1 WHERE o._id = OLD._omaster;
  RETURN OLD;
END;
$BODY$ LANGUAGE plpgsql VOLATILE SECURITY DEFINER;
ALTER FUNCTION object_connection_delete_post()
  OWNER TO su;
---
CREATE TRIGGER object_connection_delete_post AFTER DELETE ON object_connection
  FOR EACH ROW EXECUTE PROCEDURE object_connection_delete_post();

-- object_settings_type -- 
-- Table: object_settings_type

-- DROP TABLE object_settings_type;

CREATE TABLE object_settings_type
(
  _id serial NOT NULL,
  _otype integer,
  key text,
  name text,
  descr text,
  type text,
  min_value text,
  max_value text,
  CONSTRAINT object_settings_type_pkey PRIMARY KEY (_id),
  CONSTRAINT object_settings_type__otype_fkey FOREIGN KEY (_otype)
      REFERENCES object_type (_id) MATCH SIMPLE
      ON UPDATE NO ACTION ON DELETE NO ACTION
)
WITH (
  OIDS=FALSE
);
ALTER TABLE object_settings_type
  OWNER TO su;

-- Index: object_settings_type__otype_key_idx

-- DROP INDEX object_settings_type__otype_key_idx;

CREATE INDEX object_settings_type__otype_key_idx
  ON object_settings_type
  USING btree
  (_otype, key COLLATE pg_catalog."default");

GRANT ALL ON TABLE object_settings_type TO su;
GRANT SELECT ON TABLE object_settings_type TO usr;

-- object_settings -- 
-- Table: object_settings

-- DROP TABLE object_settings;

CREATE TABLE object_settings
(
  _id serial NOT NULL,
  _object integer NOT NULL,
  key text NOT NULL,
  value text,
  CONSTRAINT object_settings_pkey PRIMARY KEY (_id),
  CONSTRAINT object_settings__object_fkey FOREIGN KEY (_object)
      REFERENCES object (_id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE
)
WITH (
  OIDS=FALSE
);
ALTER TABLE object_settings
  OWNER TO su;

-- Index: object_settings__object_idx
-- DROP INDEX object_settings__object_idx;
CREATE INDEX object_settings__object_idx
  ON object_settings
  USING btree
  (_object, key);

GRANT ALL ON TABLE object_settings TO su;
GRANT SELECT ON TABLE object_settings TO usr;

-- object_settings_triggers -- 
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

-- object_state_type -- 
-- Table: object_state_type

-- DROP TABLE object_state_type;

CREATE TABLE object_state_type
(
  _id serial NOT NULL,
  name text NOT NULL,
  descr text,
  CONSTRAINT object_state_type_pkey PRIMARY KEY (_id),
  CONSTRAINT object_state_type_name_key UNIQUE (name)
)
WITH (
  OIDS=FALSE
);
ALTER TABLE object_state_type
  OWNER TO su;

-- Index: object_state_type_name_idx

-- DROP INDEX object_state_type_name_idx;

CREATE INDEX object_state_type_name_idx
  ON object_state_type
  USING btree
  (name COLLATE pg_catalog."default");

GRANT ALL ON TABLE object_state_type TO su;
GRANT SELECT ON TABLE object_state_type TO usr;

-- Constraint: object_state_type_name_key

-- ALTER TABLE object_state_type DROP CONSTRAINT object_state_type_name_key;

-- ALTER TABLE object_state_type
--  ADD CONSTRAINT object_state_type_name_key UNIQUE(name);

-- object_state_values -- 
-- Table: object_state_values

-- DROP TABLE object_state_values;

CREATE TABLE object_state_values
(
  _id serial NOT NULL,
  _ostype integer NOT NULL,
  state integer NOT NULL,
  descr text NOT NULL,
  color text NOT NULL,
  CONSTRAINT object_state_values_pkey PRIMARY KEY (_id),
  CONSTRAINT object_state_values__ostype_fkey FOREIGN KEY (_ostype)
      REFERENCES object_state_type (_id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE
)
WITH (
  OIDS=FALSE
);
ALTER TABLE object_state_values
  OWNER TO su;

-- Index: object_state_values_name_idx

-- DROP INDEX object_state_values_name_idx;

CREATE INDEX object_state_values_name_idx
  ON object_state_values
  USING btree
  (_ostype, state);

GRANT ALL ON TABLE object_state_values TO su;
GRANT SELECT ON TABLE object_state_values TO usr;

-- object_state -- 
-- Table: object_state

-- DROP TABLE object_state CASCADE;

CREATE TABLE object_state
(
  _id serial NOT NULL,
  _object integer NOT NULL,
  _ostype integer NOT NULL,
  state integer DEFAULT 0,
  change_time timestamp with time zone NOT NULL,
  CONSTRAINT object_state_pkey PRIMARY KEY (_id),
  CONSTRAINT object_state__object_fkey FOREIGN KEY (_object)
      REFERENCES object (_id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE,
  CONSTRAINT object_state__ostype_fkey FOREIGN KEY (_ostype)
      REFERENCES object_state_type (_id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE
)
WITH (
  OIDS=FALSE
);
ALTER TABLE object_state
  OWNER TO su;

-- Index: object_state__object__ostype_idx

-- DROP INDEX object_state__object__ostype_idx;

CREATE INDEX object_state__object__ostype_idx
  ON object_state
  USING btree
  (_object, _ostype);

GRANT ALL ON TABLE object_state TO su;
GRANT SELECT, UPDATE ON TABLE object_state TO usr;

-- object_state_log -- 
-- Table: object_state_log

-- DROP TABLE object_state_log;

CREATE TABLE object_state_log
(
  _id serial NOT NULL,
  _ostate integer NOT NULL,
  old_state integer NOT NULL,
  new_state integer NOT NULL,
  change_time timestamp with time zone NOT NULL,
  CONSTRAINT object_state_log_pkey PRIMARY KEY (_id),
  CONSTRAINT object_state_log__ostate_fkey FOREIGN KEY (_ostate)
      REFERENCES object_state (_id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE
)
WITH (
  OIDS=FALSE
);
ALTER TABLE object_state_log
  OWNER TO su;

-- Index: object_state_log__ostate_change_time_idx

-- DROP INDEX object_state_log__ostate_change_time_idx;

CREATE INDEX object_state_log__ostate_change_time_idx
  ON object_state_log
  USING btree
  (_ostate, change_time);


-- Index: object_state_log_change_time_idx

-- DROP INDEX object_state_log_change_time_idx;

CREATE INDEX object_state_log_change_time_idx
  ON object_state_log
  USING btree
  (change_time);

GRANT ALL ON TABLE object_state_log TO su;
GRANT SELECT, UPDATE, INSERT ON TABLE object_state_log TO usr;

-- object_state_triggers -- 
--DROP TRIGGER IF EXISTS object_state_update_post ON object_state;
--DROP FUNCTION object_state_update_post();
---
CREATE OR REPLACE FUNCTION object_state_update_post() RETURNS trigger AS $BODY$
BEGIN
  IF (OLD.state <> NEW.state) THEN
    INSERT INTO object_state_log (_ostate, old_state, new_state, change_time)
     VALUES (NEW._id, OLD.state, NEW.state, NEW.change_time);
  END IF;
  RETURN NEW;
END;
$BODY$ LANGUAGE plpgsql VOLATILE SECURITY DEFINER;
ALTER FUNCTION object_state_update_post()
  OWNER TO su;
--GRANT EXECUTE ON FUNCTION object_state_update_post(integer, integer, integer, integer) TO usr;
---
CREATE TRIGGER object_state_update_post AFTER UPDATE ON object_state
  FOR EACH ROW EXECUTE PROCEDURE object_state_update_post();
---
/*---
DROP TRIGGER IF EXISTS object_state_add_post ON object_state;
DROP FUNCTION object_state_add_post();
---
CREATE OR REPLACE FUNCTION object_state_add_post() RETURNS trigger AS $BODY$
BEGIN
  INSERT INTO object_state_log (_ostate, old_state, new_state, change_time)
   VALUES (NEW._id, NEW.state, NEW.state, NOW());
  RETURN NEW;
END;
$BODY$ LANGUAGE plpgsql VOLATILE SECURITY DEFINER;
ALTER FUNCTION object_state_add_post()
  OWNER TO su;
---
CREATE TRIGGER object_state_add_post AFTER INSERT ON object_state
  FOR EACH ROW EXECUTE PROCEDURE object_state_add_post();
---
*/

-- event_type -- 
-- Table: event_type

-- DROP TABLE event_type;

CREATE TABLE event_type
(
  _id serial NOT NULL,
  name text NOT NULL,
  descr text NOT NULL,
  icon text,
  flag integer NOT NULL DEFAULT 1,
  CONSTRAINT event_type_pkey PRIMARY KEY (_id)
)
WITH (
  OIDS=FALSE
);
ALTER TABLE event_type
  OWNER TO su;

-- Index: event_type_name_idx

-- DROP INDEX event_type_name_idx;

CREATE INDEX event_type_name_idx
  ON event_type
  USING btree
  (name COLLATE pg_catalog."default");

-- Constraint: event_type_name_key

-- ALTER TABLE event_type DROP CONSTRAINT event_type_name_key;

ALTER TABLE event_type
  ADD CONSTRAINT event_type_name_key UNIQUE(name);

-- event -- 
-- Table: event

-- DROP TABLE event;

CREATE TABLE event
(
  _id serial NOT NULL,
  _object integer NOT NULL,
  _etype integer NOT NULL,
  CONSTRAINT event_pkey PRIMARY KEY (_id),
  CONSTRAINT event__etype_fkey FOREIGN KEY (_etype)
      REFERENCES event_type (_id) MATCH SIMPLE
      ON UPDATE NO ACTION ON DELETE NO ACTION,
  CONSTRAINT event__object_fkey FOREIGN KEY (_object)
      REFERENCES object (_id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE
)
WITH (
  OIDS=FALSE
);
ALTER TABLE event
  OWNER TO su;

-- Index: event__object_idx

-- DROP INDEX event__object_idx;

CREATE INDEX event__object_idx
  ON event
  USING btree
  (_object);


-- event_log -- 
-- Table: event_log

-- DROP TABLE event_log;

CREATE TABLE event_log
(
  _id bigserial NOT NULL,
  _event integer NOT NULL,
  triggered_time timestamp with time zone NOT NULL,
  value real NOT NULL DEFAULT 1,
  info text,
  CONSTRAINT event_log_pkey PRIMARY KEY (_id),
  CONSTRAINT event_log__event_fkey FOREIGN KEY (_event)
      REFERENCES event (_id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE
)
WITH (
  OIDS=FALSE
);
ALTER TABLE event_log
  OWNER TO su;

-- Index: event_log__event_triggered_time_idx

-- DROP INDEX event_log__event_triggered_time_idx;

CREATE INDEX event_log__event_triggered_time_idx
  ON event_log
  USING btree
  (_event, triggered_time);

-- Index: event_log_triggered_time_idx

-- DROP INDEX event_log_triggered_time_idx;

CREATE INDEX event_log_triggered_time_idx
  ON event_log
  USING btree
  (triggered_time);


-- event_log_hours -- 
-- Table: event_log_hours

-- DROP TABLE event_log_hours;

CREATE TABLE event_log_hours
(
  _id bigserial NOT NULL,
  _event integer NOT NULL,
  triggered_hour timestamp with time zone NOT NULL,
  value real NOT NULL DEFAULT 0,
  CONSTRAINT event_log_hours_pkey PRIMARY KEY (_id),
  CONSTRAINT event_log_hours__event_fkey FOREIGN KEY (_event)
      REFERENCES event (_id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE
)
WITH (
  OIDS=FALSE
);
ALTER TABLE event_log_hours
  OWNER TO su;

-- Index: event_log_hours__event_triggered_hour_idx

-- DROP INDEX event_log_hours__event_triggered_hour_idx;

CREATE INDEX event_log_hours__event_triggered_hour_idx
  ON event_log_hours
  USING btree
  (_event, triggered_hour);

-- Index: event_log_hours_triggered_hour_idx

-- DROP INDEX event_log_hours_triggered_hour_idx;

CREATE INDEX event_log_hours_triggered_hour_idx
  ON event_log_hours
  USING btree
  (triggered_hour);


-- event_log_triggers -- 
CREATE OR REPLACE FUNCTION event_log_insert_post() RETURNS trigger AS $BODY$
DECLARE
  triggered_hour_ timestamp with time zone;
  id_ bigint;
BEGIN
  triggered_hour_ := date_trunc('hours', NEW.triggered_time);
  SELECT _id FROM event_log_hours WHERE triggered_hour = triggered_hour_ AND _event = NEW._event INTO id_;
  IF (id_ IS NULL) THEN
    INSERT INTO event_log_hours (_event, triggered_hour, value)
      VALUES (NEW._event, triggered_hour_, NEW.value);
  ELSE
    UPDATE event_log_hours SET value = value + NEW.value WHERE _id = id_;
  END IF;
  RETURN NEW;
END;
$BODY$ LANGUAGE plpgsql VOLATILE SECURITY DEFINER;
ALTER FUNCTION event_log_insert_post()
  OWNER TO su;
---
CREATE TRIGGER event_log_insert_post AFTER INSERT ON event_log
  FOR EACH ROW EXECUTE PROCEDURE event_log_insert_post();

-- event_stat -- 
-- Table: event_stat

-- DROP TABLE event_stat;

CREATE TABLE event_stat
(
  _id serial NOT NULL,
  _event integer NOT NULL,
  triggered_time timestamp with time zone NOT NULL DEFAULT now(),
  value real NOT NULL DEFAULT 0,
  period integer NOT NULL DEFAULT 0,
  CONSTRAINT event_stat_pkey PRIMARY KEY (_id),
  CONSTRAINT event_stat__event_fkey FOREIGN KEY (_event)
      REFERENCES event (_id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE,
  CONSTRAINT event_stat__event_key UNIQUE (_event)
)
WITH (
  OIDS=FALSE
);
ALTER TABLE event_stat
  OWNER TO su;

-- Index: event_stat__event_triggered_time_idx

-- DROP INDEX event_stat__event_triggered_time_idx;

CREATE INDEX event_stat__event_triggered_time_idx
  ON event_stat
  USING btree
  (_event, triggered_time);

-- Index: event_stat_triggered_time_idx

-- DROP INDEX event_stat_triggered_time_idx;

CREATE INDEX event_stat_triggered_time_idx
  ON event_stat
  USING btree
  (triggered_time);


-- event_stat_hours -- 
-- Table: event_stat_hours

-- DROP TABLE event_stat_hours;

CREATE TABLE event_stat_hours
(
  _id bigserial NOT NULL,
  _event integer NOT NULL,
  triggered_hour timestamp with time zone NOT NULL,
  value real NOT NULL DEFAULT 0,
  period integer NOT NULL DEFAULT 0,
  CONSTRAINT event_stat_hours_pkey PRIMARY KEY (_id),
  CONSTRAINT event_stat_hours__event_fkey FOREIGN KEY (_event)
      REFERENCES event (_id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE
)
WITH (
  OIDS=FALSE
);
ALTER TABLE event_stat_hours
  OWNER TO su;

-- Index: event_stat_hours__event_triggered_hour_idx

-- DROP INDEX event_stat_hours__event_triggered_hour_idx;

CREATE INDEX event_stat_hours__event_triggered_hour_idx
  ON event_stat_hours
  USING btree
  (_event, triggered_hour);

-- Index: event_stat_hours_triggered_hour_idx

-- DROP INDEX event_stat_hours_triggered_hour_idx;

CREATE INDEX event_stat_hours_triggered_hour_idx
  ON event_stat_hours
  USING btree
  (triggered_hour);


-- event_stat_triggers -- 
CREATE OR REPLACE FUNCTION event_stat_update_post() RETURNS trigger AS $BODY$
DECLARE
  triggered_hour_ timestamp with time zone;
  id_ bigint;
BEGIN
  triggered_hour_ := date_trunc('hours', NEW.triggered_time);
  SELECT _id FROM event_stat_hours WHERE triggered_hour = triggered_hour_ AND _event = NEW._event INTO id_;
  IF (id_ IS NULL) THEN
    INSERT INTO event_stat_hours (_event, triggered_hour, value, period)
      VALUES (NEW._event, triggered_hour_, NEW.value, NEW.period);
  ELSE
    UPDATE event_stat_hours SET value = value + NEW.value, period = period + NEW.period 
      WHERE _id = id_;
  END IF;
  RETURN NEW;
END;
$BODY$ LANGUAGE plpgsql VOLATILE SECURITY DEFINER;
ALTER FUNCTION event_stat_update_post()
  OWNER TO su;
---
CREATE TRIGGER event_stat_update_post AFTER UPDATE ON event_stat
  FOR EACH ROW EXECUTE PROCEDURE event_stat_update_post();

-- variables -- 
-- Table: variables

-- DROP TABLE variables;

CREATE TABLE variables
(
  _id bigserial NOT NULL,
  _object integer,
  key text,
  value text,
  CONSTRAINT variables_pkey PRIMARY KEY (_id),
  CONSTRAINT variables__object_fkey FOREIGN KEY (_object)
      REFERENCES object (_id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE,
  CONSTRAINT variables__object_key_key UNIQUE (_object, key)
)
WITH (
  OIDS=FALSE
);
ALTER TABLE variables
  OWNER TO su;
GRANT ALL ON TABLE variables TO su;
GRANT SELECT ON TABLE variables TO usr;

-- Index: variables__object_key_idx

-- DROP INDEX variables__object_key_idx;

CREATE INDEX variables__object_key_idx
  ON variables
  USING btree
  (_object, key COLLATE pg_catalog."default");

-- arm_monitors -- 
-- Table: arm_monitors

-- DROP TABLE arm_monitors CASCADE;

CREATE TABLE arm_monitors
(
  _id serial NOT NULL,
  _object integer,
  name text NOT NULL,
  descr text NOT NULL,
  num integer NOT NULL,
  width integer NOT NULL,
  height integer NOT NULL,
  size point DEFAULT '(0,0)'::point,
  used boolean DEFAULT FALSE,
  CONSTRAINT arm_monitors_pkey PRIMARY KEY (_id),
  CONSTRAINT arm_monitors__object_fkey FOREIGN KEY (_object)
      REFERENCES object (_id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE
)
WITH (
  OIDS=FALSE
);
ALTER TABLE arm_monitors
  OWNER TO su;

-- Index: arm_monitors__object_idx

-- DROP INDEX arm_monitors__object_idx;

CREATE INDEX arm_monitors__object_idx
  ON arm_monitors
  USING btree
  (_object);

GRANT ALL ON TABLE arm_monitors TO su;
GRANT SELECT ON TABLE arm_monitors TO usr;

-- arm_monitor_layouts -- 
-- Table: arm_monitor_layouts

-- DROP TABLE arm_monitor_layouts;

CREATE TABLE arm_monitor_layouts
(
  _id serial NOT NULL,
  _amonitor integer NOT NULL,
  place box NOT NULL,
  flag int DEFAULT 263,
  CONSTRAINT arm_monitor_layouts_pkey PRIMARY KEY (_id),
  CONSTRAINT arm_monitor_layouts__amonitor_fkey FOREIGN KEY (_amonitor)
      REFERENCES arm_monitors (_id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE
)
WITH (
  OIDS=FALSE
);
ALTER TABLE arm_monitor_layouts
  OWNER TO su;

-- Index: arm_monitor_layouts__amonitor_idx

-- DROP INDEX arm_monitor_layouts__amonitor_idx;

CREATE INDEX arm_monitor_layouts__amonitor_idx
  ON arm_monitor_layouts
  USING btree
  (_amonitor);

GRANT ALL ON TABLE arm_monitor_layouts TO su;
GRANT SELECT ON TABLE arm_monitor_layouts TO usr;

-- arm_monitor_lay_cameras -- 
-- Table: arm_monitor_lay_cameras

-- DROP TABLE arm_monitor_lay_cameras;

CREATE TABLE arm_monitor_lay_cameras
(
  _id serial NOT NULL,
  _amlayout integer NOT NULL,
  _camera integer NOT NULL,
  CONSTRAINT arm_monitor_lay_cameras_pkey PRIMARY KEY (_id),
  CONSTRAINT arm_monitor_lay_cameras__amlayout_fkey FOREIGN KEY (_amlayout)
      REFERENCES arm_monitor_layouts (_id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE,
  CONSTRAINT arm_monitor_lay_cameras__camera_fkey FOREIGN KEY (_camera)
      REFERENCES object (_id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE
)
WITH (
  OIDS=FALSE
);
ALTER TABLE arm_monitor_lay_cameras
  OWNER TO su;

-- Index: arm_monitor_lay_cameras__amlayout_idx

-- DROP INDEX arm_monitor_lay_cameras__amlayout_idx;

CREATE INDEX arm_monitor_lay_cameras__amlayout_idx
  ON arm_monitor_lay_cameras
  USING btree
  (_amlayout);

GRANT ALL ON TABLE arm_monitor_lay_cameras TO su;
GRANT SELECT ON TABLE arm_monitor_lay_cameras TO usr;
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
-- AUTO generated script -- 

-- object_states_ru -- 
--------------------
--
-- !!! If edit: Update ObjectState.h !!!
--
--------------------
--DELETE FROM object_state;
DELETE FROM object_state_values;
DELETE FROM object_state_type;
--------------------
INSERT INTO object_state_type(_id, name, descr) VALUES (1, 'power', 'Питание');

INSERT INTO object_state_values(_ostype, state, descr, color) VALUES (1, 0, 'Выключено', 'gray');
INSERT INTO object_state_values(_ostype, state, descr, color) VALUES (1, 1, 'Включено', 'green');
INSERT INTO object_state_values(_ostype, state, descr, color) VALUES (1, 2, 'Спящий режим', 'gray');

INSERT INTO object_state_type(_id, name, descr) VALUES (2, 'service', 'Сервис');

INSERT INTO object_state_values(_ostype, state, descr, color) VALUES (2, -2, 'Проблемы', 'red');
INSERT INTO object_state_values(_ostype, state, descr, color) VALUES (2, -1, 'Замечания', 'orange');
INSERT INTO object_state_values(_ostype, state, descr, color) VALUES (2, 0, 'Выключено', 'gray');
INSERT INTO object_state_values(_ostype, state, descr, color) VALUES (2, 1, 'Порядок', 'green');

INSERT INTO object_state_type(_id, name, descr) VALUES (3, 'connect', 'Соединение');

INSERT INTO object_state_values(_ostype, state, descr, color) VALUES (3, -1, 'Недоступно', 'red');
INSERT INTO object_state_values(_ostype, state, descr, color) VALUES (3, 0, 'Выключено', 'gray');
INSERT INTO object_state_values(_ostype, state, descr, color) VALUES (3, 1, 'Доступно', 'green');

-- objects -- 
--------------------
DELETE FROM object;
DELETE FROM object_type;
--------------------
INSERT INTO object_type (_id, name, descr) VALUES (0, 'tmp', 'Шаблон');
INSERT INTO object_type (_id, name, descr) VALUES (1, 'srv', 'Сервер');
INSERT INTO object_type (_id, name, descr) VALUES (20, 'cam', 'Камера');
INSERT INTO object_type (_id, name, descr) VALUES (3, 'rep', 'Хранилище');
INSERT INTO object_type (_id, name, descr) VALUES (4, 'svc', 'Сервис');
INSERT INTO object_type (_id, name, descr) VALUES (5, 'arm', 'АРМ оператора');
INSERT INTO object_type (_id, name, descr) VALUES (50, 'sch', 'Расписание');

INSERT INTO object_type (_id, name, descr) VALUES (31, 'va1', 'Видеоаналитика движения v1');

INSERT INTO object_type (_id, name, descr) VALUES (18, 'usr', 'Пользователь');
INSERT INTO object_type (_id, name, descr) VALUES (19, 'upd', 'Точка обновления');
SELECT setval('object_type__id_seq', 100);
--------------------
INSERT INTO object(_id, _otype, guid, name, descr, revision, uri, status) VALUES (0, 0, 'tmp', 'Шаблоны', 'Корневой объект для всех шаблонов', 0, '', 0);
INSERT INTO object(_id, _otype, guid, name, descr, revision, uri, status) VALUES (1, 1, 'srv', 'Сервер', 'Сервер по умолчанию', 0, '', 0);
INSERT INTO object(_id, _otype, guid, name, descr, revision, uri, status) VALUES (21, 20, 'cam', 'Камера', 'Камера по умолчанию', 0, 'tcp::', 0);
INSERT INTO object(_id, _otype, guid, name, descr, revision, uri, status) VALUES (22, 20, 'usb', 'USB веб-камера', 'Веб-камера по умолчанию', 0, 'tcp::', 0);
INSERT INTO object(_id, _otype, guid, name, descr, revision, uri, status) VALUES (23, 20, 'file', 'Видеофайл', 'Видеофайл по умолчанию', 0, 'tcp::', 0);
INSERT INTO object(_id, _otype, guid, name, descr, revision, uri, status) VALUES (3, 3, 'rep', 'Хранилище', 'Хранилище по умолчанию', 0, '', 1);
INSERT INTO object(_id, _otype, guid, name, descr, revision, uri, status) VALUES (5, 5, 'arm', 'АРМ оператора', 'АРМ оператора по умолчанию', 0, '', 0);
INSERT INTO object(_id, _otype, guid, name, descr, revision, uri, status) VALUES (31, 31, 'va1', 'Аналитика движения v1', 'Анализ видеопотока для выявления двужущихся объектов и анализа их траекторий', 0, '', 0);
INSERT INTO object(_id, _otype, guid, name, descr, revision, uri, status) VALUES (18, 18, 'usr', 'Пользователь', 'Пользователь кластера', 0, '', 0);
INSERT INTO object(_id, _otype, guid, name, descr, revision, uri, status) VALUES (19, 19, 'upd', 'Точка обновления', 'Точка обновления по-умолчанию', 0, '', 0);
INSERT INTO object(_id, _otype, guid, name, descr, revision, uri, status) VALUES (51, 50, 'sch1', 'Ежедневное расписание', 'Расписание, одинаковое каждый день', 0, '', 0);
INSERT INTO object(_id, _otype, guid, name, descr, revision, uri, status) VALUES (52, 50, 'sch2', 'Еженедельное расписание', 'Расписание, одинаковое каждую неделю', 0, '', 0);

INSERT INTO object(_id, _otype, guid, name, descr, revision, uri, status) VALUES (28, 18, 'root', 'root', 'Администратор кластера', 0, '', 0);
SELECT setval('object__id_seq', 100);
--------------------
INSERT INTO object_connection(_omaster, _oslave, type) VALUES (0, 1, 0);
INSERT INTO object_connection(_omaster, _oslave, type) VALUES (0, 5, 0);
INSERT INTO object_connection(_omaster, _oslave, type) VALUES (0, 18, 0);
INSERT INTO object_connection(_omaster, _oslave, type) VALUES (0, 19, 0);

INSERT INTO object_connection(_omaster, _oslave, type) VALUES (0, 51, 0);
INSERT INTO object_connection(_omaster, _oslave, type) VALUES (0, 52, 0);

INSERT INTO object_connection(_omaster, _oslave, type) VALUES (0, 21, 0);
INSERT INTO object_connection(_omaster, _oslave, type) VALUES (0, 22, 0);
INSERT INTO object_connection(_omaster, _oslave, type) VALUES (0, 23, 0);
INSERT INTO object_connection(_omaster, _oslave, type) VALUES (0, 3, 0);
INSERT INTO object_connection(_omaster, _oslave, type) VALUES (0, 31, 0);

-- object_settings -- 
--------------------
DELETE FROM object_settings;
DELETE FROM object_settings_type;
--------------------

-- (obj_type, def_obj) --
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (0, 'Id', 'Ид', 'Идентификатор объекта', 'int', '', '');
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (0, 'Name', 'Имя', 'Имя объекта', 'string', '', '');
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (0, 'Descr', 'Описание', 'Описание объекта', 'string', '', '');
--- Server --- (1, 1)
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (1, 'IP', 'IP адрес', 'IP адрес, под которым компоненты сервера будут доступны в системе', 'inet_address', '', '');
INSERT INTO object_settings(_object, key, value) VALUES (1, 'IP', '');
--INSERT INTO object_settings(_object, key, value) VALUES (1, 'AutoConnect', '1');
--- Store --- (3, 3)
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (3, 'Path', 'Путь', 'Путь к объекту файловой системы, это может быть как файл, так и ссылка на диск', 'path', '', '');
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (3, 'CellSize', 'Кластер', 'Размер ячейки хранилища', 'size', '16777216', '16777216');
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (3, 'PageSize', 'Страница', 'Минимальный размер данных, для чтения/записи хранилища', 'size', '1048576', '1048576');
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (3, 'Capacity', 'Размер хранилища', 'Размер хранилища в кластерах (как правило имеет смысл указывать только максимальное значение)', 'size', '200', 'max');
INSERT INTO object_settings(_object, key, value) VALUES (3, 'Path', 'd:/video.bin');
INSERT INTO object_settings(_object, key, value) VALUES (3, 'CellSize', '16777216');
INSERT INTO object_settings(_object, key, value) VALUES (3, 'PageSize', '1048576');
INSERT INTO object_settings(_object, key, value) VALUES (3, 'Capacity', '200');
--- Camera --- (20, 2x)
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (20, 'Uri', 'Uri', 'Адрес видео-источника', 'uri', '', '');
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (20, 'Login', 'Логин', 'Имя учётной записи для авторизации на видео-источнике', 'string', '', '');
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (20, 'Password', 'Пароль', 'Пароль учётной записи для авторизации на видео-источнике', 'password', '', '');
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (20, 'Module', 'Модуль', 'Модуль захвата RTSP', 'bool', 'live555', 'ffmpeg');
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (20, 'Transport', 'Транспорт', 'Транспорт передачи данных', 'bool', 'TCP', 'UDP');
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (20, 'Resolution', 'Разрешение', 'Разрешение USB камеры', 'resolution', '320x240', '1920×1080');
INSERT INTO object_settings(_object, key, value) VALUES (21, 'Uri', 'rtsp://');
INSERT INTO object_settings(_object, key, value) VALUES (21, 'Login', 'admin');
INSERT INTO object_settings(_object, key, value) VALUES (21, 'Password', 'admin');
INSERT INTO object_settings(_object, key, value) VALUES (21, 'Module', '0');
INSERT INTO object_settings(_object, key, value) VALUES (21, 'Transport', '0');
INSERT INTO object_settings(_object, key, value) VALUES (22, 'Uri', 'usb://0');
INSERT INTO object_settings(_object, key, value) VALUES (22, 'Resolution', '320x240');
INSERT INTO object_settings(_object, key, value) VALUES (23, 'Uri', 'file://');
--- Anal --- (3x, 3x)
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (31, 'Standby', 'Режим ожидания', 'Переход в режим ожидания при недостаточной освещённости', 'bool', 'Нет', 'Да');
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (31, 'Macro', 'Макрокамера', 'Аналитика макрокамеры, т.е. камера снимает объекты очень близко', 'bool', 'Нет', 'Да');
INSERT INTO object_settings(_object, key, value) VALUES (31, 'Standby', '1');
INSERT INTO object_settings(_object, key, value) VALUES (31, 'Macro', '1');

--- Arm --- (5, 5)
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (5, 'ScaleBest', 'Сохранять пропорции', 'Сохранять пропорции при выводе видео с камеры', 'bool', 'Нет', 'Да');
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (5, 'ShowMouse', 'Показывать мышь', 'Показывать мышь над окном вывода с камеры', 'bool', 'Нет', 'Да');
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (5, 'AutoHideMouse', 'Прятать мышь', 'Автоматически прятать мышь через короткий промежуток её не активности', 'bool', 'Нет', 'Да');
INSERT INTO object_settings(_object, key, value) VALUES (5, 'ScaleBest', '1');
INSERT INTO object_settings(_object, key, value) VALUES (5, 'ShowMouse', '1');
INSERT INTO object_settings(_object, key, value) VALUES (5, 'AutoHideMouse', '1');

--- Schedule --- (50, 51-52)
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (50, 'Dayly', 'Ежедневно', 'Ежедневный период работы', 'time_period', '', '');
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (50, 'Weekly0', 'Понедельники', 'Еженедельный период работы в понедельник', 'time_period', '', '');
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (50, 'Weekly1', 'Вторник', 'Еженедельный период работы во вторник', 'time_period', '', '');
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (50, 'Weekly2', 'Среда', 'Еженедельный период работы в среду', 'time_period', '', '');
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (50, 'Weekly3', 'Четверг', 'Еженедельный период работы в четверг', 'time_period', '', '');
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (50, 'Weekly4', 'Пятница', 'Еженедельный период работы в пятницу', 'time_period', '', '');
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (50, 'Weekly5', 'Суббота', 'Еженедельный период работы в субботу', 'time_period', '', '');
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (50, 'Weekly6', 'Воскресенье', 'Еженедельный период работы в воскресенье', 'time_period', '', '');
INSERT INTO object_settings(_object, key, value) VALUES (51, 'Dayly', '8:00-23:00');

INSERT INTO object_settings(_object, key, value) VALUES (52, 'Weekly0', '9:00-20:00');
INSERT INTO object_settings(_object, key, value) VALUES (52, 'Weekly1', '9:00-20:00');
INSERT INTO object_settings(_object, key, value) VALUES (52, 'Weekly2', '9:00-20:00');
INSERT INTO object_settings(_object, key, value) VALUES (52, 'Weekly3', '9:00-20:00');
INSERT INTO object_settings(_object, key, value) VALUES (52, 'Weekly4', '9:00-20:00');
INSERT INTO object_settings(_object, key, value) VALUES (52, 'Weekly5', '9:00-20:00');
INSERT INTO object_settings(_object, key, value) VALUES (52, 'Weekly6', '');

--- User --- (18, 18|28)
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (18, 'UserLogin', 'Логин', 'Логин пользователя', 'string', '', '');
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (18, 'UserPassword', 'Пароль', 'Пароль к логину пользователя', 'string', '', '');
INSERT INTO object_settings(_object, key, value) VALUES (18, 'UserLogin', 'root');
INSERT INTO object_settings(_object, key, value) VALUES (18, 'UserPassword', 'root');

INSERT INTO object_settings(_object, key, value) VALUES (28, 'UserLogin', 'root');
INSERT INTO object_settings(_object, key, value) VALUES (28, 'UserPassword', 'root');
