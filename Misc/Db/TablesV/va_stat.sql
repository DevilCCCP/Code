-- Table: va_stat

-- DROP TABLE va_stat;

CREATE TABLE va_stat
(
  _id serial NOT NULL,
  _object integer NOT NULL,
  _vstype integer NOT NULL,
  CONSTRAINT va_stat_pkey PRIMARY KEY (_id)
)
WITH (
  OIDS=FALSE
);
ALTER TABLE va_stat
  OWNER TO su;


-- Index: va_stat__vstype__object_idx

-- DROP INDEX va_stat__vstype__object_idx;

CREATE INDEX va_stat__vstype__object_idx
  ON va_stat
  USING btree
  (_object, _vstype);

