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
