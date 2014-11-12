@echo off

REM Add Mingw64\bin path to PATH

REM set PATH=C:\Program Files\mingw-builds\x64-4.8.1-posix-seh-rev5\mingw64\bin;%PATH%


REM This batch file compiles MSVMPack for windows
set VERSION=1.5

set gc=gcc
set BIN_DIR=bin

mkdir %BIN_DIR%

echo MSVMPack version %VERSION%

REM Default flags for GCC: use level 3 optimization

set SSE= -msse4.2
set DEBUG= -O3
set CFLAGS= -D _WIN32_WINNT=0x0600 -m32 -w -g %DEBUG% %SSE% -mfpmath=sse -DVERSION=%VERSION%

REM -m64

REM # Flags required by lp_solve
set LPSOLVELINK=
REM -ldl

set INCLUDES= -Iinclude -Ilp_solve_5.5 
REM -IPthread\
set LIBS= -Llib 

cd ..


rem goto suite

copy /Y lib\32\* lib\

REM Make object files

rem gcc -o test2 test2.c %CFLAGS% %INCLUDES% %LIBS% -lm -lpthread 

rem goto done

REM src/biblio.o: src/biblio.c
%gc% %CFLAGS% %INCLUDES% -c src/biblio.c -o src/biblio.o 
echo biblio.o...

REM src/algebra.o: src/algebra.c
%gc% %CFLAGS% %INCLUDES% -c src/algebra.c -o src/algebra.o
echo algebra.o...

REM src/libtrainMSVM_LLW.o: src/libtrainMSVM_LLW.c
%gc% %CFLAGS% %INCLUDES% -o src/libtrainMSVM_LLW.o -c src/libtrainMSVM_LLW.c
echo libtrainMSVM_LLW.o...
	
REM src/libevalMSVM_LLW.o: src/libevalMSVM_LLW.c
%gc% %CFLAGS% %INCLUDES% -c src/libevalMSVM_LLW.c -o src/libevalMSVM_LLW.o
echo libevalMSVM_LLW.o...

REM src/libtrainMSVM_2.o: src/libtrainMSVM_2.c
%gc% %CFLAGS% %INCLUDES% -o src/libtrainMSVM_2.o -c src/libtrainMSVM_2.c
echo libtrainMSVM_2.o...

REM src/libtrainMSVM_2fw.o: src/libtrainMSVM_2fw.c
%gc% %CFLAGS% %INCLUDES% -o src/libtrainMSVM_2fw.o -c src/libtrainMSVM_2fw.c
echo libtrainMSVM_2fw.o...

REM src/libevalMSVM_2.o: src/libevalMSVM_2.c
%gc% %CFLAGS% %INCLUDES% -c src/libevalMSVM_2.c -o src/libevalMSVM_2.o
echo libevalMSVM_2.o...

REM src/libtrainMSVM_WW.o: src/libtrainMSVM_WW.c 
%gc% %CFLAGS% %INCLUDES% -o src/libtrainMSVM_WW.o -c src/libtrainMSVM_WW.c
echo libtrainMSVM_WW.o...

REM src/libevalMSVM_WW.o: src/libevalMSVM_WW.c
%gc% %CFLAGS% %INCLUDES% -c src/libevalMSVM_WW.c -o src/libevalMSVM_WW.o
echo libevalMSVM_WW.o...
		
REM src/libtrainMSVM_CS.o: src/libtrainMSVM_CS.c
%gc% %CFLAGS% %INCLUDES% -o src/libtrainMSVM_CS.o -c src/libtrainMSVM_CS.c
echo libtrainMSVM_CS.o...
	
REM src/libevalMSVM_CS.o: src/libevalMSVM_CS.c
%gc% %CFLAGS% %INCLUDES% -c src/libevalMSVM_CS.c -o src/libevalMSVM_CS.o
echo libevalMSVM_CS.o...

REM src/libtrainMSVM.o: src/libtrainMSVM.c
%gc% %CFLAGS% %INCLUDES% -o src/libtrainMSVM.o -c src/libtrainMSVM.c
echo libtrainMSVM.o...
	
REM src/libevalMSVM.o: src/libevalMSVM.c
%gc% %CFLAGS% %INCLUDES% -c src/libevalMSVM.c -o src/libevalMSVM.o
echo libevalMSVM.o...

REM src/kernel.o: src/kernel.c src/custom_kernels.c
%gc% %CFLAGS% %INCLUDES% -o src/kernel.o -c src/kernel.c
echo kernel.o...

REM src/libMSVM.o: src/libMSVM.c src/custom_kernels.c
%gc% %CFLAGS% %INCLUDES% -o src/libMSVM.o -c src/libMSVM.c 
rem echo libMSVM.o...



REM # Make the MSVMpack static library 'libmsvm.a'
cd lib 
ar x liblpsolve55.a
cd ..
ar rcs lib/libmsvm.a src/libMSVM.o src/kernel.o src/libtrainMSVM.o src/libtrainMSVM_WW.o src/libevalMSVM.o src/libevalMSVM_WW.o src/libtrainMSVM_CS.o src/libevalMSVM_CS.o src/libtrainMSVM_LLW.o src/libevalMSVM_LLW.o src/libtrainMSVM_2.o src/libtrainMSVM_2fw.o src/libevalMSVM_2.o src/biblio.o src/algebra.o lib\*.o
del lib\*.o
del src\*.o

echo libmsvm.a...

:suite

REM # Make command-line tools
REM #
REM #	use '-Llib/ -lmsvm' to link with the MSVMpack library
REM #   
REM trainmsvm: src/trainMSVM.c lib/libmsvm.a

gcc  -o Windows/%BIN_DIR%/trainmsvm src/trainMSVM.c %CFLAGS% %INCLUDES% %LIBS% -lmsvm -lm %LPSOLVELINK% -lpthread -llpsolve55 
echo trainmsvm...
	
REM predmsvm: src/predMSVM.c lib/libmsvm.a
gcc  -o Windows/%BIN_DIR%/predmsvm src/predMSVM.c %CFLAGS% %INCLUDES% %LIBS% -lmsvm -lm -lpthread 
echo predmsvm...

rem -Wl,--stack,1073741824

cd Windows

