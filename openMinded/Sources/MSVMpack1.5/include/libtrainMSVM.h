/*
	LibtrainMSVM - Generic header file for training an MSVM
*/

#ifndef _LIBTRAINMSVM_H
#define _LIBTRAINMSVM_H

#ifdef _WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h> 
#include <math.h>
#include <ctype.h>
#include <pthread.h>
#include "algebra.h"

#include "libMSVM.h"

#define MSVM_TRAIN_MAXIT 1000000000L	// maximum numer of iterations
#define TRAIN_STEP 1000			// print status every TRAIN_STEP iterations
#define TRAIN_SMALL_STEP 100			
#define MBYTES 1048576			// how many bytes in 1 MB
#define GBYTE 1073741824		// how many bytes in 1 GB
#define MONITOR_PERIOD 60*20	// save model.tmp every MONITOR_PERIOD seconds 

/* 
	Cache structures
*/

// Structure for kernel cache
struct KernelCache {
	double *K;		// Flat 1-dim array containing all the kernel cache data
	double **rows_ptr;	// Array of pointers to rows inside K
	long *rows_idx;		// Table of conversion of indexes from general kernel matrix to cached matrix
	long *rows_idx_inv;	// Inverse table
	int *in_use;		// Keep track of how many threads are using a row of K
	long nrows;		// Number of kernel rows in cache
	long max_idx;
	int is_full;
	
	/* For local cache in cross validation
		- values are fetched from a master kernel cache 
		- and a map of indexes from local to master indexes
	*/
	struct KernelCache *master;	// master cache with allocated memory
	struct Model *master_model; // corresponding master model with complete data set
	long *map;					// for the i-th example of a subset, 
								//  map[i] = index of example in the full unpermuted data set

	// For thread synchronization
	pthread_mutex_t inuse_mutex;
	pthread_mutex_t *rowptr_mutex;
	pthread_cond_t *rowptr_cond;
};

// Main training thread cache
struct TrainingCache {

	// For all M-SVMs
	struct KernelCache *kc;
	long chunk_size;
	long *in_chunk;
	long *out_of_chunk;
	long *table_chunk;
	long *table_active;
	double **K;
	double **H_alpha;
	double **alpha_update;
	int *activeset;
	
	// For Frank-Wolfe algorithm
	double **H_delta;
	double **lp_sol;
	double gradient_diff;
	double *lp_rhs;
	
	// For Rosen's algorithm (M-SVM^2)
	long nb_variables;
	long nb_constraints;
	long nb_sat;
	double initial_norm_y_bar;
	double gradientT_y_bar;
	double **A0;
	double **A0T;
	double **A0A0T;
	double **A0A0T_1;
	double **A0A0T_ref;
	double **working_matrix1;
	double **working_matrix2;
	double **matrix_y_bar;
	double **vector_z_bar;
	double **H_y_bar;
	double *eigenvalues;
	double *constraints;

	// For CS chunk selection
	double *psi;
};


#include "libevalMSVM.h"

static unsigned short xi[3];	// seed for the random number generator used in select_random_chunk


int EVAL;	// triggered by the signal 'SIGINT' handler 
			// to call for the evaluation of the machine

// Global variables for synchronization of threads:
int STOP;	// triggered by user to stop training

extern pthread_mutex_t thread_data_mutex;
#ifdef _WIN32
extern pthread_cond_t      monitor_cond;
extern pthread_mutex_t     monitor_mutex;
#endif
// data structures for thread arguments
struct ThreadData { 	// Main thread for parallel kernel computations
	int thread_id;
	int nprocs;
	struct Model *model;
	struct KernelCache *kernelcache;
	long chunk_size;
	double accuracy;
	double **gradient;
	double **H_alpha;
	double **H_tilde_alpha;
	double *best_primal_upper_bound;
	int *activeset;	
	long *nb_SV;	
	double *lp_rhs;	
	char *alpha_file; 
	FILE *logfile_ptr;
};

struct ThreadGradient_data {	// Thread for gradient updates
	double **gradient;
	double **H_alpha;
	double **H_tilde_alpha;
	struct TrainingCache *cache;
	struct Model *model;
	long start_i;
	long end_i;
};

struct MonitorData { 		// Monitor data
	char *model_tmp_file;
	int period;
	struct Model * model;
};


// Main functions
long long MSVM_train(struct Model *model, struct Data *training_set, long chunk_size, const double accuracy, int cache_memory, int nprocs, char *alpha0_file, char *model_tmp_file, char *log_file); 
double **MSVM_train_cv(struct Model *model, struct Data *training_set, int folds, long chunk_size, const double accuracy, int cache_memory, int nprocs, char *log_file); 
long MSVM_init_train_comfile(struct Model *model, char *com_file, char *training_file, char *alpha0_file, char *alpha_file, char *log_file);
void init_alpha_b(struct Model *model, char *alpha_file);

/* Other functions included in libtrainMSVM.c */
void compute_K(struct TrainingCache *cache, const struct Model *model);
void release_K(struct TrainingCache *cache);
unsigned long long kernel_initialize_cache(unsigned long long cache_size, struct KernelCache *kc, const struct Model *model);
void kernel_free_cache(struct KernelCache *kc);
double *kernel_get_row(long index, struct KernelCache *kc, const struct Model *model);
void compute_kernel_row(const long index, const long cache_index, struct KernelCache *kc, const struct Model *model);
int switch_thread(const int nprocs, int *numthreads_grad, int *next_numthreads_grad, long *percentage, int *percentage_step, struct ThreadGradient_data *grad_data, int thread_id, long nb_data);
void select_random_chunk(struct TrainingCache *cache, const struct Model *model);
long *randperm(long N);

void *MSVM_monitor_model_thread(void *monitor_data);
void write_vector(double **vector, const long nb_data, const long Q, char *fichier);
void print_training_info(const long nb_SV, const struct Model *model);
// Other definitions
enum AlphaInitType {ALPHA_IS_ZERO=0, ALPHA_NOT_ZERO=1};

#endif
