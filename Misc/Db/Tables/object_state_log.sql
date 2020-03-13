-- Table: object_state_log

-- DROP TABLE object_state_log;

CREATE TABLE object_state_log
(
  _id bigserial NOT NULL,
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
