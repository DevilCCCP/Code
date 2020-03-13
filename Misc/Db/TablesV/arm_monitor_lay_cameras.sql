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
