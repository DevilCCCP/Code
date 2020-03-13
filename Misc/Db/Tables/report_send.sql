-- Table: report_send

-- DROP TABLE report_send;

CREATE TABLE report_send
(
  _id bigserial NOT NULL,
  _oto integer NOT NULL,
  _last_report integer NOT NULL,
  send_time timestamp with time zone,
  CONSTRAINT report_send_pkey PRIMARY KEY (_id),
  CONSTRAINT report_send__oto_fkey FOREIGN KEY (_oto)
      REFERENCES object (_id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE,
  CONSTRAINT report_send__last_report_fkey FOREIGN KEY (_last_report)
      REFERENCES report (_id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE SET NULL,
  CONSTRAINT report_send__oto_key UNIQUE (_oto)
)
WITH (
  OIDS=FALSE
);
ALTER TABLE report_send
  OWNER TO su;


-- Index: report_send__oto_idx

-- DROP INDEX report_send__oto_idx;

CREATE INDEX report_send__oto_idx
  ON report_send
  USING btree
  (_oto);

