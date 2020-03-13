-- Table: va_stat_hours

-- DROP TABLE va_stat_hours;

CREATE TABLE va_stat_hours
(
  _id bigserial NOT NULL,
  _vstat integer NOT NULL,
  _fimage bigint NOT NULL,
  hour timestamp with time zone NOT NULL,
  CONSTRAINT va_stat_hours_pkey PRIMARY KEY (_id),
  CONSTRAINT va_stat_hours__vstat_fkey FOREIGN KEY (_vstat)
      REFERENCES va_stat (_id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE,
  CONSTRAINT va_stat_hours__fimage_fkey FOREIGN KEY (_fimage)
      REFERENCES files (_id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE NO ACTION
)
WITH (
  OIDS=FALSE
);
ALTER TABLE va_stat_hours
  OWNER TO su;

-- Index: va_stat_hours__object_hour_idx

-- DROP INDEX va_stat_hours__object_hour_idx;

CREATE INDEX va_stat_hours__object_hour_idx
  ON va_stat_hours
  USING btree
  (_vstat, hour);

-- Index: va_stat_hours_hour_idx

-- DROP INDEX va_stat_hours_hour_idx;

CREATE INDEX va_stat_hours_hour_idx
  ON va_stat_hours
  USING btree
  (hour);

