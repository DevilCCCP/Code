-- Table: storage_current_cell

CREATE TABLE %username%.storage_current_cell
(
  _unit integer NOT NULL,
  current_cell integer,
  last_cell integer,
  CONSTRAINT storage_current_cell_pkey PRIMARY KEY (_unit),
  CONSTRAINT storage_current_cell_current_cell_fkey FOREIGN KEY (current_cell)
      REFERENCES %username%.storage_cell (_id) MATCH SIMPLE
      ON UPDATE NO ACTION ON DELETE SET NULL,
  CONSTRAINT storage_current_cell_last_cell_fkey FOREIGN KEY (last_cell)
      REFERENCES %username%.storage_cell (_id) MATCH SIMPLE
      ON UPDATE NO ACTION ON DELETE SET NULL
)
WITH (
  OIDS=FALSE
);
ALTER TABLE %username%.storage_current_cell
  OWNER TO %username%;
GRANT ALL ON TABLE %username%.storage_current_cell TO %username%;
---
INSERT INTO %username%.storage_current_cell (_unit, current_cell) VALUES (0, 1);
