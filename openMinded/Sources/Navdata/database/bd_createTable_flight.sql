-- Table: "Flight"

-- DROP TABLE "Flight";

CREATE TABLE "Flight"
(
  flight_id integer NOT NULL,
  CONSTRAINT flight_pk PRIMARY KEY (flight_id)
)
WITH (
  OIDS=FALSE
);
ALTER TABLE "Flight" OWNER TO zuser;

