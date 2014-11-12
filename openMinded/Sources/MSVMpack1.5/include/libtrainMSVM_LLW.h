/*
	Training Lee, Lin and Wahba MSVM -- Header file
*/

#ifndef _LIBTRAINMSVMLLW_H
#define _LIBTRAINMSVMLLW_H


#include <stdio.h>
#include <stdlib.h> 
#include <math.h>
#include <ctype.h>
#include "algebra.h"

#include "libMSVM.h"
#include "libtrainMSVM.h"

// For lp_solve API
#include "lp_lib.h"

// Lee, Lin and Wahba M-SVM training function
long MSVM_LLW_train(struct Model *model, struct Data *training_set, long chunk_size, const double accuracy, int cache_memory, const int nprocs, char *alpha0_file, char *alpha_file, char *log_file); 

// Main computing function
void *LLW_train_thread(void *th_data);

/* Other functions included in libtrainMSVM_LLW.c */
void LLW_alloc_memory(struct TrainingCache *cache, const long Q, const long nb_data, const long chunk_size);
void LLW_free_memory(struct TrainingCache *cache);
long LLW_compute_table_chunk(struct TrainingCache *cache, const struct Model *model);
void LLW_compute_H_delta(double **delta, struct TrainingCache *cache, const struct Model *model);
double LLW_compute_theta_opt(double **delta, struct TrainingCache *cache, const struct Model *model);
void LLW_compute_delta(double **delta, const struct TrainingCache *cache, const struct Model *model);
long LLW_compute_new_alpha(const double theta_opt, const struct TrainingCache *cache, struct Model *model);
int LLW_check_opt_sol(double **gradient, struct TrainingCache *cache, const struct Model *model);
int LLW_solve_lp(double **gradient, const struct TrainingCache *cache, const struct Model *model);
void LLW_init_gradient(const enum AlphaInitType alpha_init_type, double **gradient, double **H_alpha, const struct Model *model);
void LLW_update_gradient(double **gradient, double **H_alpha, struct TrainingCache *cache, const struct Model *model);
void *LLW_update_gradient_thread(void *th_data);
#ifdef _WIN32
void LLW_update_gradient_thread2(PTP_CALLBACK_INSTANCE instance,void *th_data);
#endif

#endif
