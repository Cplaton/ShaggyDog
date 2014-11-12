/*
	LibevalMSVM_2 - Header file
*/

#ifndef _LIBEVALMSVM2_H
#define _LIBEVALMSVM2_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include "algebra.h"
#include "biblio.h"

#include "libMSVM.h"
#include "libevalMSVM.h"

// Evaluation functions for the MSVM2
double MSVM_2_eval(double *best_primal_upper_bound, double **gradient, double **H_alpha, double **H_tilde_alpha, struct Model *model, const int verbose, FILE *fp);
long MSVM_2_classify(void *vector, const struct Model *model, double *real_outputs);
double MSVM_2_function(long category, void *vector, const struct Model *model);

/* Other functions included in this program */
int MSVM_2_check_feasible_sol(const struct Model *model, char *alpha_file);
void MSVM_2_compute_gradient(double **gradient, struct Cache *cache, const struct Model *model);
void MSVM_2_compute_xi_opt(double **xi_opt, struct Cache *cache, const struct Model *model);
void MSVM_2_check_xi_opt(double **xi_opt, double **gradient, const struct Cache *cache, const struct Model *model);
void MSVM_2_correct_xi_opt(double **xi_opt, struct Cache *cache, const struct Model *model, const int MaxIter);
void MSVM_2_optimize_b(double **xi_opt, struct Cache *cache, struct Model *model);
double MSVM_2_compute_primal_obj(double **xi_opt, const struct Cache *cache, const struct Model *model);
void MSVM_2_minimal_xi(const int keepok,double **xi_opt, const struct Cache *cache, const struct Model *model);
void MSVM_2_estimate_b(double **gradient, const struct Cache *cache, struct Model *model);
void MSVM_2_compute_obj_dual(struct Cache *cache, const struct Model *model);
void MSVM_2_compute_obj_primal(struct Cache *cache, double **xi_opt, const struct Model *model);
long MSVM_2_eval_training(long **mat_conf, const struct Cache *cache, const struct Model *model);

#endif
