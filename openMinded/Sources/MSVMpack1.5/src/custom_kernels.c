/*
	Custom kernel functions for MSVMpack
	
	This file contains templates for 3 custom kernel functions. 
	Edit this file to implement new kernels and use them by calling
	
	trainmain -k 5 -p kernel_par (for custom kernel 1) 
	trainmain -k 6 -p kernel_par (for custom kernel 2) 
	trainmain -k 7 -p kernel_par (for custom kernel 3) 
			
	Each custom kernel function must return the value k(x1,x2)
	and takes the following inputs:
	  x1, x2	: two vectors 
	  dim		: dimension of the vectors
	  kernel_par	: vector of parameters of the kernel function 
	  
	Remember to use indexes from 1 to dim to access vector components,
	e.g., to compute the inner product k(x1,x2) = < x1, x2 >, write

		for(i = 1; i<=dim; i++)
		  kernel_value += x1[i] * x2[i];
		  
	Kernel parameters are stored in an array of DOUBLE of size (1 + #parameters):
	
		kernel_par[0] = #parameters     
		kernel_par[1] ... kernel_par[(int)kernel_par[0]] = parameter values 
		
	If the function takes a single parameter, its default value can be defined in 'include/kernel.h'.
	
	Names of custom kernel functions can also be defined in 'include/kernel.h'.  
*/	 
#include <stdio.h>
#include <stdlib.h>

double custom_kernel_1(const double *x1 , const double *x2 , const long dim, const double *kernel_par) {
	double kernel_value = 0.0;
	
	/* Add code to compute custom kernel function here
		and remove the next 4 lines. 
	*/
	printf("Error: custom kernel 1 is undefined.\n");
	printf("   Open MSVMpack/src/custom_kernels.c and edit the \n");
	printf("   function custom_kernel_1() to implement your new kernel.\n");
	exit(1);
	
	return kernel_value;
}

double custom_kernel_2(const double *x1 , const double *x2 , const long dim, const double *kernel_par) {
	double kernel_value = 0.0;

	/* Add code to compute custom kernel function here
		and remove the next 4 lines. 
	*/
	printf("Error: custom kernel 2 is undefined.\n");
	printf("   Open MSVMpack/src/custom_kernels.c and edit the \n");
	printf("   function custom_kernel_2() to implement your new kernel.\n");
	exit(1);
	
	return kernel_value;
}

double custom_kernel_3(const double *x1 , const double *x2 , const long dim, const double *kernel_par) {
	double kernel_value = 0.0;

	/* Add code to compute custom kernel function here
		and remove the next 4 lines. 
	*/
	printf("Error: custom kernel 3 is undefined.\n");
	printf("   Open MSVMpack/src/custom_kernels.c and edit the \n");
	printf("   function custom_kernel_3() to implement your new kernel.\n");
	exit(1);
	
	return kernel_value;
}





/*
	The following functions can be used to compute similar kernels
	with FLOAT data type
	
	Note that the same default values for kernel_par as defined above
	apply to these functions.
*/

double custom_kernel_1_float(const float *x1 , const float *x2 , const long dim, const double *kernel_par) {
	double kernel_value = 0.0;

	/* Add code to compute custom kernel function here
		and remove the next 4 lines. 
	*/
	printf("Error: custom kernel 1 is undefined for FLOAT data type.\n");
	printf("   Open MSVMpack/src/custom_kernels.c and edit the \n");
	printf("   function custom_kernel_1_float() to implement your new kernel.\n");
	exit(1);
	
	return kernel_value;
}

double custom_kernel_2_float(const float *x1 , const float *x2 , const long dim, const double *kernel_par) {
	double kernel_value = 0.0;

	/* Add code to compute custom kernel function here
		and remove the next 4 lines. 
	*/
	printf("Error: custom kernel 2 is undefined for FLOAT data type.\n");
	printf("   Open MSVMpack/src/custom_kernels.c and edit the \n");
	printf("   function custom_kernel_2_float() to implement your new kernel.\n");
	exit(1);
	
	return kernel_value;
}

double custom_kernel_3_float(const float *x1 , const float *x2 , const long dim, const double *kernel_par) {
	double kernel_value = 0.0;

	/* Add code to compute custom kernel function here
		and remove the next 4 lines. 
	*/
	printf("Error: custom kernel 3 is undefined for FLOAT data type.\n");
	printf("   Open MSVMpack/src/custom_kernels.c and edit the \n");
	printf("   function custom_kernel_3_float() to implement your new kernel.\n");
	exit(1);
	
	return kernel_value;
}






/*
	Custom kernel functions for INT data type
*/

double custom_kernel_1_int(const int *x1 , const int *x2 , const long dim, const double *kernel_par) {
	double kernel_value = 0.0;

	/* Add code to compute custom kernel function here
		and remove the next 4 lines. 
	*/
	printf("Error: custom kernel 1 is undefined for INT data type.\n");
	printf("   Open MSVMpack/src/custom_kernels.c and edit the \n");
	printf("   function custom_kernel_1_int() to implement your new kernel.\n");
	exit(1);
	
	return kernel_value;
}

double custom_kernel_2_int(const int *x1 , const int *x2 , const long dim, const double *kernel_par) {
	double kernel_value = 0.0;

	/* Add code to compute custom kernel function here
		and remove the next 4 lines. 
	*/
	printf("Error: custom kernel 2 is undefined for INT data type.\n");
	printf("   Open MSVMpack/src/custom_kernels.c and edit the \n");
	printf("   function custom_kernel_2_int() to implement your new kernel.\n");
	exit(1);
	
	return kernel_value;
}

double custom_kernel_3_int(const int *x1 , const int *x2 , const long dim, const double *kernel_par) {
	double kernel_value = 0.0;

	/* Add code to compute custom kernel function here
		and remove the next 4 lines. 
	*/
	printf("Error: custom kernel 3 is undefined for INT data type.\n");
	printf("   Open MSVMpack/src/custom_kernels.c and edit the \n");
	printf("   function custom_kernel_3_int() to implement your new kernel.\n");
	exit(1);
	
	return kernel_value;
}






/*
	Custom kernel functions for SHORT INT data type
*/

double custom_kernel_1_short_int(const short int *x1 , const short int *x2 , const long dim, const double *kernel_par) {
	double kernel_value = 0.0;

	/* Add code to compute custom kernel function here
		and remove the next 4 lines. 
	*/
	printf("Error: custom kernel 1 is undefined for SHORT INT data type.\n");
	printf("   Open MSVMpack/src/custom_kernels.c and edit the \n");
	printf("   function custom_kernel_1_short_int() to implement your new kernel.\n");
	exit(1);
	
	return kernel_value;
}

double custom_kernel_2_short_int(const short int *x1 , const short int *x2 , const long dim, const double *kernel_par) {
	double kernel_value = 0.0;

	/* Add code to compute custom kernel function here
		and remove the next 4 lines. 
	*/
	printf("Error: custom kernel 2 is undefined for SHORT INT data type.\n");
	printf("   Open MSVMpack/src/custom_kernels.c and edit the \n");
	printf("   function custom_kernel_2_short_int() to implement your new kernel.\n");
	exit(1);
	
	return kernel_value;
}

double custom_kernel_3_short_int(const short int *x1 , const short int *x2 , const long dim, const double *kernel_par) {
	double kernel_value = 0.0;

	/* Add code to compute custom kernel function here
		and remove the next 4 lines. 
	*/
	printf("Error: custom kernel 3 is undefined for SHORT INT data type.\n");
	printf("   Open MSVMpack/src/custom_kernels.c and edit the \n");
	printf("   function custom_kernel_3_short_int() to implement your new kernel.\n");
	exit(1);
	
	return kernel_value;
}






/*
	Custom kernel functions for BYTE data type
*/

double custom_kernel_1_byte(const unsigned char *x1 , const unsigned char *x2 , const long dim, const double *kernel_par) {
	double kernel_value = 0.0;

	/* Add code to compute custom kernel function here
		and remove the next 4 lines. 
	*/
	printf("Error: custom kernel 1 is undefined for BYTE data type.\n");
	printf("   Open MSVMpack/src/custom_kernels.c and edit the \n");
	printf("   function custom_kernel_1_byte() to implement your new kernel.\n");
	exit(1);
	
	return kernel_value;
}

double custom_kernel_2_byte(const unsigned char *x1 , const unsigned char *x2 , const long dim, const double *kernel_par) {
	double kernel_value = 0.0;

	/* Add code to compute custom kernel function here
		and remove the next 4 lines. 
	*/
	printf("Error: custom kernel 2 is undefined for BYTE data type.\n");
	printf("   Open MSVMpack/src/custom_kernels.c and edit the \n");
	printf("   function custom_kernel_2_byte() to implement your new kernel.\n");
	exit(1);
	
	return kernel_value;
}

double custom_kernel_3_byte(const unsigned char *x1 , const unsigned char *x2 , const long dim, const double *kernel_par) {
	double kernel_value = 0.0;

	/* Add code to compute custom kernel function here
		and remove the next 4 lines. 
	*/
	printf("Error: custom kernel 3 is undefined for BYTE data type.\n");
	printf("   Open MSVMpack/src/custom_kernels.c and edit the \n");
	printf("   function custom_kernel_3_byte() to implement your new kernel.\n");
	exit(1);
	
	return kernel_value;
}




/*
	Custom kernel functions for BIT data type
	
	Data format: 
		- each x1[i] is an INT containing 32 bits
		- dim is the number of bits in a data vector, so the dimension of x1 is
			true_dim = (dim/32) + (dim%32 != 0)
		- if dim%32 != 0, the last component x1[true_dim] is completed with zeros		
*/

double custom_kernel_1_bit(const int *x1 , const int *x2 , const long dim, const double *kernel_par) {
	double kernel_value = 0.0;

	/* Add code to compute custom kernel function here
		and remove the next 4 lines. 
	*/
	printf("Error: custom kernel 1 is undefined for BIT data type.\n");
	printf("   Open MSVMpack/src/custom_kernels.c and edit the \n");
	printf("   function custom_kernel_1_bit() to implement your new kernel.\n");
	exit(1);
	
	return kernel_value;
}

double custom_kernel_2_bit(const int *x1 , const int *x2 , const long dim, const double *kernel_par) {
	double kernel_value = 0.0;

	/* Add code to compute custom kernel function here
		and remove the next 4 lines. 
	*/
	printf("Error: custom kernel 2 is undefined for BIT data type.\n");
	printf("   Open MSVMpack/src/custom_kernels.c and edit the \n");
	printf("   function custom_kernel_2_bit() to implement your new kernel.\n");
	exit(1);
	
	return kernel_value;
}

double custom_kernel_3_bit(const int *x1 , const int *x2 , const long dim, const double *kernel_par) {
	double kernel_value = 0.0;

	/* Add code to compute custom kernel function here
		and remove the next 4 lines. 
	*/
	printf("Error: custom kernel 3 is undefined for BIT data type.\n");
	printf("   Open MSVMpack/src/custom_kernels.c and edit the \n");
	printf("   function custom_kernel_3_bit() to implement your new kernel.\n");
	exit(1);
	
	return kernel_value;
}

