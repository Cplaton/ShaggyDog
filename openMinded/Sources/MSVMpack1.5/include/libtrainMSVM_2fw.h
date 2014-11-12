/*
	Training M-SVM^2 with Franke-Wolfe algorithm -- Header file
*/

#ifndef _LIBTRAINMSVM_2fw_H
#define _LIBTRAINMSVM_2fw_H


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
long MSVM_2fw_train(struct Model *model, struct Data *training_set, long chunk_size, const double accuracy, int cache_memory, const int nprocs, char *alpha0_file, char *alpha_file, char *log_file); 

// Main computing function
void *MSVM2fw_train_thread(void *th_data);

/* Other functions included in libtrainMSVM_LLW.c */
void MSVM2fw_alloc_memory(struct TrainingCache *cache, const long Q, const long nb_data, const long chunk_size);
void MSVM2fw_free_memory(struct TrainingCache *cache);
void MSVM2fw_compute_H_delta(double **delta, struct TrainingCache *cache, const struct Model *model);
double MSVM2fw_compute_theta_opt(double **delta, struct TrainingCache *cache, const struct Model *model);
void MSVM2fw_compute_delta(double **delta, const struct TrainingCache *cache, const struct Model *model);
long MSVM2fw_compute_new_alpha(const double theta_opt, const struct TrainingCache *cache, struct Model *model);
int MSVM2fw_check_opt_sol(double **gradient, struct TrainingCache *cache, const struct Model *model);
int MSVM2fw_solve_lp(double **gradient, const struct TrainingCache *cache, const struct Model *model);
void MSVM2fw_init_gradient(const enum AlphaInitType alpha_init_type, double **gradient, double **H_alpha, double **H_tilde_alpha, const struct Model *model);
void MSVM2fw_update_gradient(double **gradient, double **H_alpha, double **H_tilde_alpha, struct TrainingCache *cache, const struct Model *model);
void *MSVM2fw_update_gradient_thread(void *th_data);
#ifdef _WIN32
void MSVM2fw_update_gradient_thread2(PTP_CALLBACK_INSTANCE instance,void *th_data);
#endif

#endif
