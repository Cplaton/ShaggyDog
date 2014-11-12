/*
	LibevalMSVM_CS - Header file
*/

#ifndef _LIBEVALSVMCS_H
#define _LIBEVALSVMCS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include "algebra.h"
#include "biblio.h"

#include "libMSVM.h"
#include "libevalMSVM.h"

// Main evaluation functions for the CS-MSVM
double MSVM_CS_eval(double *best_primal_upper_bound, double **H_alpha, struct Model *model, const int verbose, FILE *fp);
long CS_classify(void *vector, const struct Model *model, double *real_outputs);
double CS_function(long category, void *vector, const struct Model *model);

/* Other functions included in this program */
void CS_check_feasible_sol(const struct Model *model, char *alpha_file, int verbose);
void CS_compute_gradient(double **gradient, struct Cache *cache, const struct Model *model);
double CS_eval_training(double **gradient, long **mat_conf, const struct Model *model);
void CS_compute_obj_dual(struct Cache *cache, struct Model *model);
void CS_compute_obj_primal(double R_emp, struct Cache *cache, const struct Model *model);

#endif 
