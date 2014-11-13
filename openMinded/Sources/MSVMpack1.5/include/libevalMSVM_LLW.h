/*
	LibevalMSVM_LLW - Header file
*/

#ifndef _LIBEVALSVMLLW_H
#define _LIBEVALSVMLLW_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include "algebra.h"
#include "biblio.h"

#include "libMSVM.h"
#include "libevalMSVM.h"

// Main evaluation functions for the LLW-MSVM
double MSVM_LLW_eval(double *best_primal_upper_bound, double **gradient, double **H_alpha, struct Model *model, const int verbose, FILE *fp);
long LLW_classify(void *vector, const struct Model *model, double *real_outputs);
double LLW_function(long category, void *vector, const struct Model *model);

/* Other functions included in this program */
void LLW_check_feasible_sol(const struct Model *model, char *alpha_file, int verbose);
void LLW_compute_gradient(double **gradient, struct Cache *cache, const struct Model *model);
void LLW_check_margin_vect(long **margin_vect, long *nb_margin_vect, const struct Model *model);
double LLW_eval_training(double **gradient, long **mat_conf, const struct Cache *cache, const struct Model *model);
void LLW_estimate_b(double **gradient, long **margin_vect, long *nb_margin_vect, const struct Cache *cache, struct Model *model);
void LLW_compute_obj_dual(struct Cache *cache, struct Model *model);
void LLW_compute_obj_primal(double R_emp, struct Cache *cache, const struct Model *model);

#endif 
