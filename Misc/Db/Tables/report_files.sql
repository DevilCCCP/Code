-- Table: report_files

-- DROP TABLE report_files;

CREATE TABLE report_files
(
  _id bigserial NOT NULL,
  _report bigint,
  _files bigint,
  CONSTRAINT report_files_pkey PRIMARY KEY (_id),
  CONSTRAINT report_files__report_fkey FOREIGN KEY (_report)
      REFERENCES report (_id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE,
  CONSTRAINT report_files__files_fkey FOREIGN KEY (_files)
      REFERENCES files (_id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE
)
WITH (
  OIDS=FALSE
);
ALTER TABLE report_files
  OWNER TO su;


CREATE INDEX report_files__report_idx
  ON report_files
  USING btree
  (_report);

CREATE INDEX report_files__files_idx
  ON report_files
  USING btree
  (_files);

