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
