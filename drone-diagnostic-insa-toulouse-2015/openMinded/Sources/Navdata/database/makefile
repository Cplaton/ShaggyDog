# Makefile for sample programs
# in Beginning PostgreSQL

# Edit the base directories for your
# PostgreSQL installation
INC=/usr/include/postgresql/
LIB=/usr/lib/postgresql/8.4/lib/

CFLAGS=-I$(INC)
LDLIBS=-L$(LIB) -lpq

bddManagement:  bd_management.c testBd.c
		gcc -o bd_management bd_management.c testBd.c $(CFLAGS) $(LDLIBS)



