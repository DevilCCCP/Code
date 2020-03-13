-- Table: va_stat_days

-- DROP TABLE va_stat_days;

CREATE TABLE va_stat_days
(
  _id bigserial NOT NULL,
  _vstat integer NOT NULL,
  _fimage bigint NOT NULL,
  day timestamp with time zone NOT NULL,
  CONSTRAINT va_stat_days_pkey PRIMARY KEY (_id),
  CONSTRAINT va_stat_days__vstat_fkey FOREIGN KEY (_vstat)
      REFERENCES va_stat (_id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE,
  CONSTRAINT va_stat_days__fimage_fkey FOREIGN KEY (_fimage)
      REFERENCES files (_id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE
)
WITH (
  OIDS=FALSE
);
ALTER TABLE va_stat_days
  OWNER TO su;

-- Index: va_stat_days__object_day_idx

-- DROP INDEX va_stat_days__object_day_idx;

CREATE INDEX va_stat_days__object_day_idx
  ON va_stat_days
  USING btree
  (_vstat, day);

-- Index: va_stat_days_day_idx

-- DROP INDEX va_stat_days_day_idx;

CREATE INDEX va_stat_days_day_idx
  ON va_stat_days
  USING btree
  (day);

