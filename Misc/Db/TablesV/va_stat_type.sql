-- Table: va_stat_type

-- DROP TABLE va_stat_type;

CREATE TABLE va_stat_type
(
  _id serial NOT NULL,
  abbr text NOT NULL,
  name text NOT NULL,
  descr text NOT NULL,
  CONSTRAINT va_stat_type_pkey PRIMARY KEY (_id),
  CONSTRAINT va_stat_type_abbr_key UNIQUE (abbr)
)
WITH (
  OIDS=FALSE
);
ALTER TABLE va_stat_type
  OWNER TO su;

-- Index: va_stat_type_abbr_idx

-- DROP INDEX va_stat_type_abbr_idx;

CREATE INDEX va_stat_type_abbr_idx
  ON va_stat_type
  USING btree
  (abbr COLLATE pg_catalog."default");

