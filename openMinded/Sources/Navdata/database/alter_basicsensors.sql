ALTER TABLE "BasicSensors" ADD COLUMN classe integer;
ALTER TABLE "BasicSensors" ADD CONSTRAINT "BasicSensors_classe_FK" FOREIGN KEY (classe) REFERENCES "Classes" (classe_id) ON UPDATE NO ACTION ON DELETE NO ACTION;

