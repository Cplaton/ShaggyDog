/*
	Training Weston and Watkins MSVM -- Header file
*/


#ifndef _LIBTRAINMSVMWW_H
#define _LIBTRAINMSVMWW_H



#include <stdio.h>
#include <stdlib.h> 
#include <math.h>
#include <ctype.h>
#include "algebra.h"

#include "libMSVM.h"
#include "libtrainMSVM.h"

// For lp_solve API
#include "lp_lib.h"

// Weston and Watkins M-SVM training function
long MSVM_WW_train(struct Model *model, struct Data *training_set, long chunk_size, const double accuracy, int cache_memory, const int nprocs, char *alpha0_file, char *alpha_file, char *log_file); 

// Main computing function
void *WW_train_thread(void *th_data);

/* Other functions included in libtrainMSVM_WW.c */
void WW_alloc_memory(struct TrainingCache *cache, const long Q, const long nb_data, const long chunk_size);
void WW_free_memory(struct TrainingCache *cache);
int WW_compute_gradient_chunk(double **gradient, struct TrainingCache *cache, const struct Model *model);
void WW_compute_H_delta(double **delta, struct TrainingCache *cache, const struct Model *model);
double WW_compute_theta_opt(double **delta, struct TrainingCache *cache, const struct Model *model);
void WW_compute_delta(double **delta, const struct TrainingCache *cache, const struct Model *model);
long WW_compute_new_alpha(const double theta_opt, const struct TrainingCache *cache, struct Model *model);
int WW_check_opt_sol(double **gradient, struct TrainingCache *cache, const struct Model *model);
int WW_solve_lp(double **gradient, const struct TrainingCache *cache, const struct Model *model);

void WW_init_gradient(const enum AlphaInitType alpha_init_type, double **gradient, double **H_alpha, const struct Model *model);
int WW_check_gradient_chunk(double **gradient, struct TrainingCache *cache, const struct Model *model);
void *WW_update_gradient_thread(void *th_data);
void WW_update_gradient(double **gradient, double **H_alpha, struct TrainingCache *cache, const struct Model *model);
#ifdef _WIN32
void WW_update_gradient_thread2(PTP_CALLBACK_INSTANCE instance,void *th_data);
#endif

#endif
