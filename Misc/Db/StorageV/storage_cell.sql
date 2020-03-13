-- Table: storage_cell

CREATE TABLE %username%.storage_cell
(
  _id serial NOT NULL,
  _unit integer NOT NULL,
  start_time timestamp with time zone,
  end_time timestamp with time zone,
  condition integer,
  CONSTRAINT %username%_storage_cell_pkey PRIMARY KEY (_id)
)
WITH (
  OIDS=FALSE
);
ALTER TABLE %username%.storage_cell
  OWNER TO %username%;

CREATE INDEX %username%_storage_cell__unit_start_time_idx
  ON %username%.storage_cell
  USING btree
  (_unit, start_time);

INSERT INTO %username%.storage_cell (_id, _unit) VALUES (1, 0);
