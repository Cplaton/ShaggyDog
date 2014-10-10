CC=gcc
CFLAGS=-W -Wall -pedantic
OPT= -lrt -pthread
EXEC=smartfox

all: $(EXEC)
	make clean

smartfox: command.o control_cmd.o navdata.o drone_com.o sfe_data_management.o diagnoser.o smartfox.o
	$(CC) -o smartfox smartfox.o control_cmd.o navdata.o command.o drone_com.o \
		 sfe_data_management.o diagnoser.o $(OPT)

smartfox.o: smartfox.c 
	$(CC) -o smartfox.o -c smartfox.c

control_cmd.o:droneLib/control_cmd.c
	$(CC) -o control_cmd.o -c droneLib/control_cmd.c 

navdata.o: droneLib/navdata.c
	$(CC) -o navdata.o -c droneLib/navdata.c

command.o: command/command.c
	$(CC) -o command.o -c command/command.c

drone_com.o: com/drone_com.c	
	$(CC) -o drone_com.o -c com/drone_com.c

sfe_data_management.o: sfe_data/sfe_data_management.c
	$(CC) -o sfe_data_management.o -c sfe_data/sfe_data_management.c

diagnoser.o: diagnoser/diagnoser.c
	$(CC) -o diagnoser.o -c diagnoser/diagnoser.c

clean:
	rm -rf *.o

mrproper: clean
	rm -rf $(EXEC)
