/*
	LibMSVM.h: general Header file including
		   common object definitions for MSVMpack
*/
#ifndef _LIBMSVM_H
#define _LIBMSVM_H

#ifdef _WIN32 // Windows 32 or 64 bit
# include <windows.h>
#else
# include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <pthread.h>
#ifdef __APPLE__
#include <sys/sysctl.h>		// for Mac OSX
#endif
#include "algebra.h"
#include "kernel.h"

// Useful macros
#define true 1
#define false 0
#define taille 500

#ifdef _WIN32 // Windows 32 or 64 bit
#define Small 1e-4
#else
#define small 1e-4
#endif

#define very_small 1e-6
#define small_enough 1e-8
#define tiny 1e-12

#define minimum(a,b) ((a)<=(b)?(a):(b))
#define maximum(a,b) ((a)>=(b)?(a):(b))

// Default parameters
#define DEFAULT_C 10.0;

/*
	Type (and precision) of data
*/
enum Datatype { DATATYPE_DOUBLE = 1, DATATYPE_FLOAT = 2, DATATYPE_INT = 3, DATATYPE_SHORT = 4, DATATYPE_BYTE = 5, DATATYPE_BIT = 6 };
//enum Datatype {DOUBLE=1, FLOAT=2, INT=3, SHORT=4, BYTE=5, BIT=6};


/*
	Model structure
*/
struct Model {
	float version;				// MSVMpack model version
	enum MSVM_type {WW=0,CS=1,
			LLW=2,MSVM2=3} type;	// Type of the M-SVM
	enum Algorithm {FrankWolfe=0,
			Rosen=1} algorithm;	// Optimization method
   
	long Q;                           // number of classes
	char *training_set_name;          // name of training set
	long nb_data;                     // number of SVs
	long dim_input;                   // dimension of x
	enum Datatype datatype;			  // type of data
	enum Kernel_type nature_kernel;   // type of kernel
	double *kernel_par;               // kernel function parameter
	double *C;                        // soft-margin parameters
	double training_error;			  // training error rate
    double ratio;                     // optimization accuracy
    long iter;                        // optimization iterations
    int crossvalidation;              // #fold in cross validation (0 otherwise)
	
	double **alpha;                // (nb_data x Q) matrix of alpha
	double *partial_average_alpha;
	double sum_all_alpha;        
	double *b_SVM;                 // vector b
	void **X;                      // support vectors
	long *y;                       // labels of the SVs
	double **normalization;        // (mean,std) used to normalize data (or NULL)
	double **W;                    // Weights of the linear model	

	// Format-specific data storage
	double **X_double;
	float **X_float;
	int **X_int;
	short int **X_short;
	unsigned char **X_byte;
	
	// Thread synchronization
	pthread_mutex_t mutex; 
};

/*
	Generic Data structure for a dataset
*/
struct Data {
	char *name;
	long nb_data;
	long dim;
	void **X;
	long *y;
	long Q; // number of classes in dataset
	double **X_double;
	float **X_float;
	int **X_int;
	short int **X_short;
	unsigned char **X_byte;
	enum Datatype datatype;
};

/* Model and Data handling functions */
struct Model *MSVM_make_model(enum MSVM_type type);
struct Model *MSVM_make_model_copy(struct Model *model);
void MSVM_delete_model(struct Model *model);
void MSVM_delete_model_with_data(struct Model *model);
long MSVM_init_model(struct Model *model, char *fichcom_file);
long MSVM_save_model(const struct Model *model, char *model_file);
long MSVM_save_model_sparse(const struct Model *model, char *model_file);
struct Model *MSVM_load_model(char *model_file);
void MSVM_model_set_C(double C, long Q, struct Model *model);

struct Data *MSVM_make_dataset(char *data_file, enum Datatype datatype);
void MSVM_delete_dataset(struct Data *dataset);
void allocate_dataset(struct Data *dataset, const long nb_data, const long dim_input) ;
double MSVM_normalize_data(struct Data *dataset, struct Model *model);
void copy_data_point(struct Data *src, long src_idx, struct Data *dest, long dest_idx);

/* General wrapper for kernel functions */
double ker(const enum Kernel_type type, const void *x1, const void *x2, const long dim, const double *par);
void set_default_kernel_par(struct Model *model);

/* File I/O functions */
long read_data(struct Data *dataset, enum Datatype datatype, char *data_file);
double read_alpha(struct Model *model, char *alpha_file);

/* Others */
void model_load_data(struct Model *model, const struct Data *training_set);
double normalize_data(struct Model *model, const int check_only);
double model_get_Xij(struct Model *model, const long i, const long j);
void Pause(char *message);
int check_argv(int argc, char *argv[], char *str);
int check_argv_eq(int argc, char *argv[], char *str);
enum MSVM_type str2model_type(char *str);
void remove_spaces(char *string, char *str_nospaces, const int maxchar);
int get_nprocessors(void);
unsigned long long get_physical_memory(void);

#endif

