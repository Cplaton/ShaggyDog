-- Table: "BasicSensors"

-- DROP TABLE "BasicSensors";

CREATE TABLE "BasicSensors"
(
  data_id integer NOT NULL DEFAULT 0,
  flight integer NOT NULL DEFAULT 0,
  "time" integer NOT NULL DEFAULT 0,
  altitude real NOT NULL DEFAULT 0.0,
  pitch real NOT NULL DEFAULT 0.0,
  roll real NOT NULL DEFAULT 0.0,
  vyaw real NOT NULL DEFAULT 0.0,
  vx real NOT NULL DEFAULT 0.0,
  vy real NOT NULL DEFAULT 0.0,
  vz real NOT NULL DEFAULT 0.0,
  ax real NOT NULL DEFAULT 0.0,
  ay real NOT NULL DEFAULT 0.0,
  az real NOT NULL DEFAULT 0.0,
  CONSTRAINT "BasicSensors_pk" PRIMARY KEY (data_id),
  CONSTRAINT "BasicSensors_flight_fk" FOREIGN KEY (flight)
      REFERENCES "Flight" (flight_id) MATCH SIMPLE
      ON UPDATE NO ACTION ON DELETE NO ACTION
)
WITH (
  OIDS=FALSE
);
ALTER TABLE "BasicSensors" OWNER TO zuser;

