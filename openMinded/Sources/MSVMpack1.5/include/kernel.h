/*
	Types of kernel functions
*/
#ifndef _KERNEL_FCT_H_
#define _KERNEL_FCT_H_


/*
	Default values for kernel parameters
*/
#define DEFAULT_RBF_KERNEL_PAR sqrt(5.0 * (model->dim_input))
#define DEFAULT_POLY_KERNEL_PAR 2
#define DEFAULT_CUSTOM_KERNEL_1_PAR 0
#define DEFAULT_CUSTOM_KERNEL_2_PAR 0
#define DEFAULT_CUSTOM_KERNEL_3_PAR 0

/*
	Names of custom kernel functions
*/
#define CUSTOM_KERNEL_1_NAME "custom 1"
#define CUSTOM_KERNEL_2_NAME "custom 2"
#define CUSTOM_KERNEL_3_NAME "custom 3"

/*
	Kernel types
*/
enum Kernel_type {	// kernel functions for DOUBLE datatype
		  LINEAR=1,	// linear
	          RBF=2,	// Gaussian RBF
	          POLY_H=3,	// Homogeneous polynomial
	          POLY=4,	// Non-homogeneous polynomial
	          CUSTOM1=5,	// Custom kernels
	          CUSTOM2=6,
	          CUSTOM3=7,
	          	// same for FLOAT datatype
	          LINEAR_FLOAT=11,
	          RBF_FLOAT=12,	
	          POLY_H_FLOAT=13,	
	          POLY_FLOAT=14,
	          CUSTOM1_FLOAT=15,	
	          CUSTOM2_FLOAT=16,
	          CUSTOM3_FLOAT=17,	          
		          // kernels for INT datatype
	          LINEAR_INT=21,
	          RBF_INT=22,	
	          POLY_H_INT=23,	
	          POLY_INT=24,
	          CUSTOM1_INT=25,	
	          CUSTOM2_INT=26,
	          CUSTOM3_INT=27,	          
		          // kernels for SHORT INT datatype		          
  	          LINEAR_SHORT=31,
	          RBF_SHORT=32,
	          POLY_H_SHORT=33,	
	          POLY_SHORT=34,
	          CUSTOM1_SHORT=35,	
	          CUSTOM2_SHORT=36,
	          CUSTOM3_SHORT=37,	          	          
		          // kernels for BYTE datatype
	          LINEAR_BYTE=41,
	          RBF_BYTE=42,	
	          POLY_H_BYTE=43,	
	          POLY_BYTE=44,
	          CUSTOM1_BYTE=45,	
	          CUSTOM2_BYTE=46,
	          CUSTOM3_BYTE=47,	          
		          // kernels for BIT datatype
	          LINEAR_BIT=51,
	          RBF_BIT=52,	
	          POLY_H_BIT=53,	
	          POLY_BIT=54,
	          CUSTOM1_BIT=55,	
	          CUSTOM2_BIT=56,
	          CUSTOM3_BIT=57,	          
	          };


/*
	Kernel functions

	Suffix indicates the format of data the function takes as input
*/

double dot_double(const double *a, const double *b, const long dim);
double dot_float(const float *a, const float *b, const long dim);
double dot_int(const int *a , const int *b , const long dim);
double dot_short_int(const short int *a, const short int *b, const long dim);
double dot_byte(const unsigned char *a, const unsigned char *b, const long dim);
double dot_bit(const int *a, const int *b, const long dim);

double rbf_double(const double *a, const double *b, const long dim, const double sigma);
double rbf_float(const float *a, const float *b, const long dim, const double sigma);
double rbf_int(const int *a, const int *b, const long dim, const double sigma);
double rbf_short_int(const short int *a, const short int *b, const long dim, const double sigma);
double rbf_byte(const unsigned char *a, const unsigned char *b, const long dim, const double sigma);
double rbf_bit(const int *a, const int *b, const long dim, const double sigma);

double polynomial_homo_double(const double *a , const double *b , const long dim, const int deg);
double polynomial_homo_float(const float *a , const float *b , const long dim, const int deg);
double polynomial_homo_int(const int *a , const int *b , const long dim, const int deg);
double polynomial_homo_short_int(const short int *a , const short int *b , const long dim, const int deg);
double polynomial_homo_byte(const unsigned char *a , const unsigned char *b , const long dim, const int deg);
double polynomial_homo_bit(const int *a, const int *b, const long dim, const int deg);

double polynomial_double(const double *a , const double *b , const long dim, const int deg);
double polynomial_float(const float *a , const float *b , const long dim, const int deg);
double polynomial_int(const int *a , const int *b , const long dim, const int deg);
double polynomial_short_int(const short int *a , const short int *b , const long dim, const int deg);
double polynomial_byte(const unsigned char *a , const unsigned char *b , const long dim, const int deg);
double polynomial_bit(const int *a, const int *b, const long dim, const int deg);

double custom_kernel_1(const double *x1 , const double *x2 , const long dim, const double *kernel_par);
double custom_kernel_2(const double *x1 , const double *x2 , const long dim, const double *kernel_par);
double custom_kernel_3(const double *x1 , const double *x2 , const long dim, const double *kernel_par);
double custom_kernel_1_float(const float *x1 , const float *x2 , const long dim, const double *kernel_par);
double custom_kernel_2_float(const float *x1 , const float *x2 , const long dim, const double *kernel_par);
double custom_kernel_3_float(const float *x1 , const float *x2 , const long dim, const double *kernel_par);
double custom_kernel_1_int(const int *x1 , const int *x2 , const long dim, const double *kernel_par);
double custom_kernel_2_int(const int *x1 , const int *x2 , const long dim, const double *kernel_par);
double custom_kernel_3_int(const int *x1 , const int *x2 , const long dim, const double *kernel_par);
double custom_kernel_1_short_int(const short int *x1 , const short int *x2 , const long dim, const double *kernel_par);
double custom_kernel_2_short_int(const short int *x1 , const short int *x2 , const long dim, const double *kernel_par);
double custom_kernel_3_short_int(const short int *x1 , const short int *x2 , const long dim, const double *kernel_par);
double custom_kernel_1_byte(const unsigned char *x1 , const unsigned char *x2 , const long dim, const double *kernel_par);
double custom_kernel_2_byte(const unsigned char *x1 , const unsigned char *x2 , const long dim, const double *kernel_par);
double custom_kernel_3_byte(const unsigned char *x1 , const unsigned char *x2 , const long dim, const double *kernel_par);
double custom_kernel_1_bit(const int *x1 , const int *x2 , const long dim, const double *kernel_par);
double custom_kernel_2_bit(const int *x1 , const int *x2 , const long dim, const double *kernel_par);
double custom_kernel_3_bit(const int *x1 , const int *x2 , const long dim, const double *kernel_par);

// Custom kernel functions can be defined in custom_kernels.c

#endif
