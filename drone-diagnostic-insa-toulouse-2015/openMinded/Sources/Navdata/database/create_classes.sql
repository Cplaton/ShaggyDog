CREATE TABLE "Classes"
(
   classe_id integer NOT NULL DEFAULT 0, 
   classe_descr text NOT NULL DEFAULT, 
   CONSTRAINT "Classes_PK" PRIMARY KEY (classe_id)
) 
WITH (
  OIDS = FALSE
)
;
