-- Table: report

-- DROP TABLE report;

CREATE TABLE report
(
  _id bigserial NOT NULL,
  _object integer NOT NULL,
  type integer,
  period_begin timestamp with time zone,
  period_end timestamp with time zone,
  data bytea,
  CONSTRAINT report_pkey PRIMARY KEY (_id),
  CONSTRAINT report__object_fkey FOREIGN KEY (_object)
      REFERENCES object (_id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE
)
WITH (
  OIDS=FALSE
);
ALTER TABLE report
  OWNER TO su;

-- Index: report__object_idx

-- DROP INDEX report__object_idx;

CREATE INDEX report__object_idx
  ON report
  USING btree
  (_object);

-- Index: report__object_type_period_begin_idx

-- DROP INDEX report__object_type_period_begin_idx;

CREATE INDEX report__object_type_period_begin_idx
  ON report
  USING btree
  (_object, type, period_begin);

