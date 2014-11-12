/*
	LibevalMSVM_WW - Header file
*/

#ifndef _LIBEVALSVMWW_H
#define _LIBEVALSVMWW_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include "algebra.h"
#include "biblio.h"

#include "libMSVM.h"
#include "libevalMSVM.h"

// Main evaluation functions for the WW-MSVM
double MSVM_WW_eval(double *best_primal_upper_bound, double **gradient, double **H_alpha, struct Model *model, const int verbose, FILE *fp);
long WW_classify(void *vector, const struct Model *model, double *real_outputs);
double WW_function(long category, void *vector, const struct Model *model);

/* Other functions included in this program */
void WW_optimize_b(double **gradient, struct Cache *cache, struct Model *model);
void WW_check_feasible_sol(const struct Model *model, char *alpha_file, int verbose);
void WW_check_sup_vect(const struct Model *model);
void WW_check_margin_vect(long **margin_vect, long **nb_margin_vect, const struct Model *model);
double WW_eval_training(double **gradient, long **mat_conf, const struct Model *model);
void WW_estimate_b(double **gradient, long **margin_vect, long **nb_margin_vect, const struct Cache *cache, struct Model *model);
void WW_compute_obj_dual(struct Cache *cache, struct Model *model);
void WW_compute_obj_primal(double R_emp, struct Cache *cache, const struct Model *model);

void WW_compute_W(struct Model *model);
void WW_compute_boundaries(const struct Model *model);

#endif 
