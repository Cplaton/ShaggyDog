/*
	Training an MSVM^2 -- Header file
*/

#ifndef _LIBTRAINMSVM2_H
#define _LIBTRAINMSVM2_H

#include <stdio.h>
#include <stdlib.h> 
#include <math.h>
#include <ctype.h>
#include "algebra.h"

#include "libMSVM.h"
#include "libtrainMSVM.h"

// MSVM2 training function
long MSVM_2_train(struct Model *model, struct Data *training_set, long chunk_size, const double accuracy, int cache_memory, const int nprocs, char *alpha0_file, char *alpha_file, char *log_file); 

// Main computing thread
void *MSVM_2_train_thread(void *th_data);

/* Other functions included in libtrainMSVM_2.c */
void MSVM_2_alloc_memory(struct TrainingCache *cache, const long Q, const long nb_data, const long chunk_size);
void MSVM_2_free_memory(struct TrainingCache *cache);
void MSVM_2_init_gradient(const enum AlphaInitType alpha_init_type, double **gradient, double **H_alpha, double **H_tilde_alpha, const struct Model *model);
void MSVM_2_update_gradient(double **gradient, double **H_alpha, double **H_tilde_alpha, struct TrainingCache *cache, const struct Model *model);
void *MSVM2_update_gradient_thread(void *th_data);
void MSVM_2_compute_gradient_chunk(double **vector_gradient, double **gradient, struct TrainingCache *cache, const struct Model *model);
void MSVM_2_compute_A0(struct TrainingCache *cache, const struct Model *model);
void MSVM_2_compute_vector_u(double **vector_u, double **vector_gradient, struct TrainingCache *cache, const struct Model *model);
int MSVM_2_check_vector_u(double **vector_u, double **vector_gradient, struct TrainingCache *cache, const struct Model *model);
void MSVM_2_compute_vector_y_bar(double **vector_y_bar, double **vector_u, double **vector_gradient, struct TrainingCache *cache, const struct Model *model);
int MSVM_2_check_vector_y_bar(double **vector_y_bar, double **vector_u, double **vector_gradient, struct TrainingCache *cache, const struct Model *model);
void MSVM_2_simplify_A0(double **vector_u, struct TrainingCache *cache, const struct Model *model);
double MSVM_2_compute_theta_max(const struct TrainingCache *cache, const struct Model *model);
void MSVM_2_compute_H_y_bar(struct TrainingCache *cache, const struct Model *model);
double MSVM_2_compute_theta_opt(const double theta_max, struct TrainingCache *cache, const struct Model *model);
long MSVM_2_compute_new_alpha(const double theta_opt, const struct TrainingCache *cache, struct Model *model);
int MSVM_2_check_feasible_sol_train(struct Model *model,  struct TrainingCache *cache);
int MSVM_2_check_feasible_sol_init(struct Model *model);
void MSVM_2_study_unfeasible_sol(const int descent, double **vector_u, double **vector_y_bar, const struct TrainingCache *cache, const struct Model *model);
double MSVM_2_compute_objective_function(const struct Model *model, double former_val_obj_function);
void MSVM_2_compute_pseudo_inverse(struct TrainingCache *cache);
#ifdef _WIN32
void MSVM2_update_gradient_thread2(PTP_CALLBACK_INSTANCE instance, void *th_data);
#endif

#endif
