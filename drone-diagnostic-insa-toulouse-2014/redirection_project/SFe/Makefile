CC = /opt/CodeSourcery/Sourcery_CodeBench_Lite_for_ARM_GNU_Linux/bin/arm-none-linux-gnueabi-gcc -march=armv7-a
CFLAGS=-W -Wall -pedantic
OPT=
EXEC=smartfoxE.elf

all: $(EXEC)
	make clean

smartfoxE.elf:  smartfoxE.o drone_com.o 
	$(CC) -o smartfoxE.elf smartfoxE.o drone_com.o $(OPT)

smartfoxE.o: smartfoxE.c 
	$(CC) -o smartfoxE.o -c smartfoxE.c

drone_com.o:com/drone_com.c
	$(CC) -o drone_com.o -c com/drone_com.c 

clean:
	rm -rf *.o

mrproper: clean
	rm -rf $(EXEC)
