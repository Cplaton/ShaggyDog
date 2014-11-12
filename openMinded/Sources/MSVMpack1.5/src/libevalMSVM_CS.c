/* Copyright 2001, 2002, 2003 Yann Guermeur and Andre Elisseeff               */

/* This program is free software; you can redistribute it and/or modify       */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 2 of the License, or          */
/* (at your option) any later version.                                        */

/* This program is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of             */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              */
/* GNU General Public License for more details.                               */

/* You should have received a copy of the GNU General Public License          */
/* along with this program; if not, write to the Free Software                */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA  */

/*----------------------------------------------------------------------------*/
/*  Name           : libevalMSVM_CS.c                                         */
/*  Version        : 2.0                                                      */
/*  Creation       : 04/30/00                                                 */
/*  Last update    : 01/20/11                                                 */
/*  Subject        : Implementation of the M-SVM of Crammer and Singer        */
/*  Module         : Evaluation functions                                     */
/*  Author         : Fabien Lauer and Yann Guermeur Yann.Guermeur@loria.fr    */
/*----------------------------------------------------------------------------*/


#include "libevalMSVM_CS.h"


/*
	Main evaluation function for CS: compute the ratio (dual / upper bound)
*/
double MSVM_CS_eval(double *best_primal_upper_bound, double **H_alpha, struct Model *model, const int verbose, FILE *fp) {

	const long Q = model->Q;
	double R_emp, ratio;
	
	struct Cache cache;
	cache.H_alpha = H_alpha; 
	cache.verbose = verbose;
		
	long **mat_conf = matrix_l(Q,Q);
	long i,training_errors;
	
	CS_check_feasible_sol(model,NULL,verbose);
  
	R_emp = CS_eval_training(cache.H_alpha,mat_conf,model);

	// Compute training error
	training_errors = model->nb_data;
	for(i=1;i<=Q;i++)
		training_errors -= mat_conf[i][i];
	
	model->training_error = (double)training_errors/(double)model->nb_data;

	// Compute the ratio dual/upper bound (stopping criterion)

	CS_compute_obj_dual(&cache,model);
	CS_compute_obj_primal(R_emp,&cache,model);

	// Make sure the upper bound does not increase
	if(cache.primal > *best_primal_upper_bound)
		cache.primal = *best_primal_upper_bound;
	else
		*best_primal_upper_bound = cache.primal;
				
	ratio = cache.dual / cache.primal;
		
	if(verbose) {
		printf("\nDifference between the primal and the dual: %lf -> %lf", cache.primal, cache.dual);
		printf("\n(ratio %5.2lf \%%)\n", 100.0 * ratio);
	
		printf("\n*** Training performance\n");
		display_stats(Q, mat_conf);
		display_full_stats(Q, mat_conf);
	
	}

	// Log info
	if(fp != NULL)
		fprintf(fp, "%lf %lf \n",cache.dual,cache.primal);
	

	// Free memory
	free(mat_conf[1]); free(mat_conf);
	return ratio;
	
}

/*
	output = CS_function( k, x, model)
	
	Compute the real-valued output of model for the k-th category on pattern x
*/
double CS_function(long category, void *vector, const struct Model *model)
{
	const long nb_data = model->nb_data;
	const long Q = model->Q;	
	const enum Kernel_type nature_kernel = model->nature_kernel;
	const double *kernel_par = model->kernel_par;
	long dim_input = model->dim_input;

	long i,k;

	double partial, result = 0.0;

	for(i=1; i<=nb_data; i++)
	  {
	  if(model->y[i] == category)
	    {
	    partial = 0.0;
	    for(k=1; k<=Q; k++)
	      if(k != category)
		      partial += model->alpha[i][k];
	    }
	  else
	    partial = - model->alpha[i][category];
	  
	  if(partial != 0.0)
	    result += partial * ker(nature_kernel, model->X[i], vector, dim_input, kernel_par);
	  }

	return result;
}

/*
	label = CS_classify(x, model, real_outputs)
	
	Classify x and output the label. 
	  
	If real_outputs is not NULL, also compute the Q real-valued outputs
	 of the model and store them in (real_outputs[1]...real_outpust[Q]).
	 
	Note: this function is faster than calling Q times CS_function 		
*/
long CS_classify(void *vector, const struct Model *model, double *real_outputs)
{
	const long nb_data = model->nb_data;
	const long Q = model->Q;	
	const enum Kernel_type nature_kernel = model->nature_kernel;
	const double *kernel_par = model->kernel_par;
	long dim_input = model->dim_input;

	long i,k;
	
	long label = 1;
	
	double maxoutput,partial, kernel;
	
	int isSV, use_real_outputs = true;
	if(real_outputs==NULL) {
		real_outputs = (double *)malloc(sizeof(double) * (Q+1));				
		use_real_outputs = false; 
	}

	for(k=1;k<=Q;k++) 
		real_outputs[k] = 0.0;	// No bias term in CS model

	for(i=1; i<=nb_data; i++)
	  {  
	    partial = 0.0;
	    isSV = false;
	    for(k=1; k<=Q; k++) {
	      partial += model->alpha[i][k];
	      if(k != model->y[i] && model->alpha[i][k] != 0.0)
	      	isSV = true;
	    }
	    if(isSV) {
		    kernel = ker(nature_kernel, model->X[i], vector, dim_input, kernel_par);
		    for (k=1; k<=Q; k++) {
		    	    if(model->y[i] == k)
			    	real_outputs[k] += (partial - model->alpha[i][k]) * kernel;
			    else
			    	real_outputs[k] -= model->alpha[i][k] * kernel;
	 	    }
	    }
	  }
		
	maxoutput = real_outputs[1];

	for(k=2; k<=Q; k++)
		if(real_outputs[k] > maxoutput)
		{
			maxoutput = real_outputs[k];
			label = k;
		}
		
	if(!use_real_outputs) 
		free(real_outputs);

	return label;
}

void CS_check_feasible_sol(const struct Model *model, char *alpha_file,int verbose)
{
	char command[taille];
	const long Q = model->Q;
	const long nb_data = model->nb_data;
	const double *C = model->C;
	double partial;
	int rc;
	
	long i,k;	

	for(i=1; i<=nb_data; i++)
	  for(k=1; k<=Q; k++)
	    if((model->alpha[i][k] < 0.0) || (model->alpha[i][k] > C[model->y[i]]))
	       {
	       printf("\nNo feasible solution: alpha[%ld][%ld] = %1.12lf  >  %lf\n\n",
		      i, k, model->alpha[i][k], C[model->y[i]]);
	       exit(0);
	       }

		     
	for(i=1; i<=nb_data; i++)
	  {
	  partial = 0.0;
	  for(k=1; k<=Q; k++)
	    partial += model->alpha[i][k];
	  if(fabs(partial - C[model->y[i]]) > very_small)
	    {
	    printf("\nLarge deviation of the equality constraint: sum_alpha = %1.10lf ~= %lf = C\n",partial,C[model->y[i]]);
	    
	    exit(0);
	    }
	  }
	

	if(alpha_file != NULL) {
		sprintf(command, "cp %s Save_alpha/.", alpha_file);
		rc = system(command);
	}
}

/*
	Compute the training error
*/
double CS_eval_training(double **H_alpha, long **mat_conf, const struct Model *model)
{
	long i,k,l,y_i;
	const long Q = model->Q;
	const long nb_data = model->nb_data;

	long ind_max = 0;
	double xi_ik, maxi = HUGE_VAL;
	
	double R_emp = 0.0;

	if(mat_conf != NULL) {
		for(k=1; k<=Q; k++)
		  for(l=1; l<=Q; l++)
		    mat_conf[k][l] = 0;
	}
	
	for(i=1; i<=nb_data; i++)  {
	  maxi = 0.0;
	  ind_max = 0;
	  y_i = model->y[i];

	  for(k=1; k<=Q; k++)
	    if(k != y_i)
	      {
	      xi_ik = 1.0 - H_alpha[i][k];
	      if(xi_ik < 0.0)
	      	xi_ik = 0.0;
	      if(xi_ik > maxi)
		{
		maxi = xi_ik;
		ind_max = k;
		}
     	      }
     	      
     	  R_emp += model->C[y_i] * maxi;
	  if(mat_conf != NULL) {
		  if(maxi > 1.0)
		    mat_conf[y_i][ind_max]++;
		  else
		    mat_conf[y_i][y_i]++;
	  }
	}

  	return R_emp;
}

void CS_compute_W(struct Model *model)
{
	const long Q = model->Q;
	const long nb_data = model->nb_data;
	const long dim_input = model->dim_input;
	long i,j,k,l;
	double partial;
	
	if(model->W == NULL)
		model->W = matrix(dim_input, Q);
		
	for(j=1; j<=dim_input; j++)
	  for(k=1; k<=Q; k++)
	    model->W[j][k] = 0.0;

	for(k=1; k<=Q; k++)
	  for(i=1; i<=nb_data; i++)
	    {
	    if((model->y[i]) == k)
	      {
	      partial = 0.0;
	      for(l=1; l<=Q; l++)
		partial += model->alpha[i][l];
	      for(j=1; j<=dim_input; j++)
		model->W[j][k] += partial * model_get_Xij(model,i,j);
	      }
	    else
	      for(j=1; j<=dim_input; j++)
		model->W[j][k] -= model->alpha[i][k] * model_get_Xij(model,i,j);
	    }	
}

void CS_compute_obj_dual(struct Cache *cache, struct Model *model)

{
	long i,k;
	const long Q = model->Q;
	const long nb_data = model->nb_data;
	
	double alpha_H_alpha = 0.0;
	double sum_Ci_minus_alpha_iyi = 0.0;
	
	for(i=1; i<=nb_data; i++) {
	  for(k=1; k<=Q; k++) {
	    alpha_H_alpha += cache->H_alpha[i][k] * model->alpha[i][k];
	    
	 }
	 
	  sum_Ci_minus_alpha_iyi +=  model->C[model->y[i]] - model->alpha[i][model->y[i]];
	}
	
	cache->alpha_H_alpha = alpha_H_alpha;
	cache->dual = -0.5*alpha_H_alpha + sum_Ci_minus_alpha_iyi;
}

void CS_compute_obj_primal(double R_emp, struct Cache *cache, const struct Model *model)
{
 
	cache->primal = 0.5 * cache->alpha_H_alpha + R_emp;
	if(cache->primal < cache->dual)
	  {
	  printf("\nInconsistency in the values of the objective functions...\n primal = %lf + %lf = %lf  <  %lf (dual)\n", 0.5 * cache->alpha_H_alpha, R_emp , cache->primal, cache->dual);
	/*  exit(0); */
	  }
	
}

