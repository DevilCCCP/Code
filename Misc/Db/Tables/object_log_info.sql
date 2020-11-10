-- Table: object_log_info

-- DROP TABLE IF EXISTS object_log_info;

CREATE TABLE object_log_info
(
  _id bigserial,
  _object integer NOT NULL,
  hours_start timestamp with time zone NOT NULL,
  hours_end timestamp with time zone NOT NULL,
  last_trunc timestamp with time zone,
  last_clean timestamp with time zone,
  CONSTRAINT object_log_info_pkey PRIMARY KEY (_id),
  CONSTRAINT object_log_info__object_fkey FOREIGN KEY (_object)
      REFERENCES object (_id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE,
  CONSTRAINT object_log_info__object_key UNIQUE (_object)
)
WITH (
  OIDS=FALSE
);
ALTER TABLE object_log_info
  OWNER TO su;
GRANT ALL ON TABLE object_log_info TO su;
GRANT SELECT, UPDATE ON TABLE object_log_info TO usr;

-- Index: object_log_info__object_idx

-- DROP INDEX object_log_info__object_idx;

CREATE INDEX object_log_info__object_idx
  ON object_log_info
  USING btree
  (_object);

-- Index: public.object_log_info_last_clean_idx

-- DROP INDEX public.object_log_info_last_clean_idx;

CREATE INDEX object_log_info_last_clean_idx
  ON public.object_log_info
  USING btree
  (last_clean);

-- Index: public.object_log_info_last_trunc_idx

-- DROP INDEX public.object_log_info_last_trunc_idx;

CREATE INDEX object_log_info_last_trunc_idx
  ON public.object_log_info
  USING btree
  (last_trunc);