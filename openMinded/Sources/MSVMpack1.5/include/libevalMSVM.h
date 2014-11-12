/*
	LibevalMSVM - Header file
*/

#ifndef _LIBEVALMSVM_H
#define _LIBEVALMSVM_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include "algebra.h"
#include "biblio.h"

#include "libMSVM.h"
#include "libtrainMSVM.h"

#define step 100
#define SET_SIZE 2000

/*
	Cache Structure for evaluation purposes
*/
struct Cache {
	double **H_alpha;
	double **H_tilde_alpha;
	double alpha_H_alpha;
	double alpha_H_tilde_alpha;	
	double sum_all_xi_opt;
	double primal;
	double primal_cheap;
	double dual;	
	int verbose;
};

// For thread synchronization
static pthread_mutex_t thread_classify_data_mutex = PTHREAD_MUTEX_INITIALIZER;

// data structures for threads
struct ThreadClassifyData { 
	int thread_id;
	struct Model *model;
	void **X;
	long *labels;
	long nb_data;	
	long **mat_conf;
	long *y;
	double **outputs;
};

// Main functions
double MSVM_eval(double *best_primal_upper_bound, double **gradient, double **H_alpha, double **H_tilde_alpha, struct Model *model, const int verbose, FILE *fp);
long MSVM_classify(void *x, const struct Model *model, double *real_outputs);
void MSVM_classify_set(long *labels, void **X, long *y, long nb_data, char *outputs_file, struct Model *model, int nprocs);

// Other functions included
void standardize_b(double *b_SVM, const long Q);
void *MSVM_classify_set_thread(void *th_data);
void MSVM_print_outputs(double **outputs, long *labels, long *y, long nb_data, long Q, char *outputs_file);
#endif
