#
#	This is a simple example Makefile to show how to compile
#	and link with the MSVMpack library
#	

# MSVMpack root directory	<-- This should be changed in your own Makefiles
MSVMPACK_DIR= ../

# Header files directory
INCLUDEDIR= -I$(MSVMPACK_DIR)include/ 

# Static library directory
LIBDIR= -L$(MSVMPACK_DIR)lib/

# Required libraries
LIB=-lmsvm -lm -ldl -lpthread

# Make rule
example: example.c
	gcc -o example example.c $(INCLUDEDIR) $(LIBDIR) $(LIB)
	@echo "Successfully compiled example.c"
	
# -lmsvm	: link with MSVMpack static library (libmsvm.a)
# -lm		: math library
# -ldl		: required by lp_solve
# -lpthread	: POSIX threads library (for parallel implementation)
