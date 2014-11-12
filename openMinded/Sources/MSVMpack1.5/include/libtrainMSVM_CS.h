/*
	Training Crammer and Singer MSVM -- Header file
*/

#ifndef _LIBTRAINMSVMCS_H
#define _LIBTRAINMSVMCS_H


#include <stdio.h>
#include <stdlib.h> 
#include <math.h>
#include <ctype.h>
#include "algebra.h"

#include "libMSVM.h"
#include "libtrainMSVM.h"

// For lp_solve API
#include "lp_lib.h"
	
// Crammer and Singer M-SVM training function
long MSVM_CS_train(struct Model *model, struct Data *training_set, long chunk_size, const double accuracy, int cache_memory, const int nprocs, char *alpha0_file, char *alpha_file, char *log_file); 

// Main computing function
void *CS_train_thread(void *th_data);

/* Other functions included in libtrainMSVM_CS.c */
void CS_alloc_memory(struct TrainingCache *cache, const long Q, const long nb_data, const long chunk_size);
void CS_free_memory(struct TrainingCache *cache);
long CS_compute_table_chunk(struct TrainingCache *cache, const struct Model *model);
void CS_init_gradient(const enum AlphaInitType alpha_init_type, double **gradient, double **H_alpha, const struct Model *model);
void *CS_update_gradient_thread(void *th_data);
void CS_update_gradient(double **gradient, double **H_alpha, struct TrainingCache *cache, const struct Model *model);
void CS_compute_H_delta(double **delta, struct TrainingCache *cache, const struct Model *model);
double CS_compute_theta_opt(double **delta, struct TrainingCache *cache, const struct Model *model);
void CS_compute_delta(double **delta, const struct TrainingCache *cache, const struct Model *model);
long CS_compute_new_alpha(const double theta_opt, const struct TrainingCache *cache, struct Model *model);
int CS_check_opt_sol(double **gradient, struct TrainingCache *cache, const struct Model *model);
int CS_solve_lp(double **gradient, const struct TrainingCache *cache, const struct Model *model);
void CS_init_alpha(struct Model *model, char *alpha_file);
double CS_workingset_selection(double **Fmatrix, struct TrainingCache *cache, const struct Model *model);
int CS_switch_thread(const int nprocs, int *numthreads_grad, int *next_numthreads_grad, long *percentage, int *percentage_step, struct ThreadGradient_data *grad_data, int thread_id, long nb_data);
#ifdef _WIN32
void CS_update_gradient_thread2(PTP_CALLBACK_INSTANCE instance, void *th_data);
#endif

#endif
