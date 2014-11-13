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
/*  Name           : libevalMSVM_LLW.c                                        */
/*  Version        : 2.0                                                      */
/*  Creation       : 04/30/00                                                 */
/*  Last update    : 01/20/11                                                 */
/*  Subject        : Implementation of the M-SVM of Lee, Lin and Wahba        */
/*  Module         : evaluation functions                                     */
/*  Author         : Fabien Lauer and Yann Guermeur Yann.Guermeur@loria.fr    */
/*----------------------------------------------------------------------------*/


#include "libevalMSVM_LLW.h"


/*
	Main evaluation function for LLW: compute the ratio (dual / upper bound)
*/
double MSVM_LLW_eval(double *best_primal_upper_bound, double **gradient, double **H_alpha, struct Model *model, const int verbose, FILE *fp) {

	const long Q = model->Q;
	const long nb_data = model->nb_data;

	double R_emp, ratio;
	
	struct Cache cache;
	cache.H_alpha = H_alpha; 
	cache.verbose = verbose;
			
	long **mat_conf = matrix_l(Q,Q);
	long i,training_errors;
	
	long **margin_vect = matrix_l(nb_data, Q);
	long *nb_margin_vect = (long*)malloc(sizeof(long) * (Q+1));
	
	LLW_check_feasible_sol(model,NULL,verbose);

	LLW_check_margin_vect(margin_vect,nb_margin_vect,model);		
  	
	LLW_estimate_b(gradient, margin_vect, nb_margin_vect, &cache, model);

	R_emp = LLW_eval_training(gradient,mat_conf,&cache,model);

	// Compute training error
	training_errors = model->nb_data;
	for(i=1;i<=Q;i++)
		training_errors -= mat_conf[i][i];
	
	model->training_error = (double)training_errors/(double)model->nb_data;

	// Compute the ratio dual/upper bound (stopping criterion)

	LLW_compute_obj_dual(&cache,model);
	LLW_compute_obj_primal(R_emp,&cache,model);
	
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
	free(margin_vect[1]); free(margin_vect);
	free(nb_margin_vect);	
	return ratio;
	
}


/*
	output = LLW_function( k, x, model)
	
	Compute the real-valued output of model (without bias term)
	for the k-th category on pattern x
*/
double LLW_function(long category, void *vector, const struct Model *model)
{
	const long nb_data = model->nb_data;
	const long Q = model->Q;	
	const double Qd = (double)Q;
	const enum Kernel_type nature_kernel = model->nature_kernel;
	const double *kernel_par = model->kernel_par;
	long dim_input = model->dim_input;

	long i,k;

	double partial, result = 0.0;

	for(i=1; i<=nb_data; i++)
	  {
	    partial = 0.0;
	    for(k=1; k<=Q; k++)
	      partial += model->alpha[i][k];
	    
	    partial /= Qd;
	    partial -= model->alpha[i][category];
	  
	  if(partial != 0.0)
	    result += partial * ker(nature_kernel, model->X[i], vector, dim_input, kernel_par);
	  }

	return result;
}

/*
	label = LLW_classify(x, model, real_outputs)
	
	Classify x and output the label. 
	  
	If real_outputs is not NULL, also compute the Q real-valued outputs
	 of the model and store them in (real_outputs[1]...real_outpust[Q]).
	 
	Note: this function is faster than calling Q times LLW_function 		
*/
long LLW_classify(void *vector, const struct Model *model, double *real_outputs)
{
	const long nb_data = model->nb_data;
	const long Q = model->Q;	
	const double Qd = (double)Q;
	const enum Kernel_type nature_kernel = model->nature_kernel;
	const double *kernel_par = model->kernel_par;
	long dim_input = model->dim_input;

	long i,k;
	
	long label = 1;
	
	double maxoutput,partial, kernel;

	int use_real_outputs = true;
	if(real_outputs==NULL) {
		real_outputs = (double *)malloc(sizeof(double) * (Q+1));				
		use_real_outputs = false; 
	}

	for(k=1;k<=Q;k++) 
		real_outputs[k] = model->b_SVM[k];		

	for(i=1; i<=nb_data; i++)
	  {
	    partial = 0.0;
	    for(k=1; k<=Q; k++)
	      partial += model->alpha[i][k];
	    
	    if(partial != 0.0) {
		    partial /= Qd;
		    
		    kernel = ker(nature_kernel, model->X[i], vector, dim_input, kernel_par);
		    
		    for (k=1; k<=Q; k++) 
			    real_outputs[k] += (partial - model->alpha[i][k]) * kernel;
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
void LLW_check_feasible_sol(const struct Model *model, char *alpha_file,int verbose)
{
	char command[taille];
	const long Q = model->Q;
	const long nb_data = model->nb_data;
	const double *C = model->C;
	double norm;
	int rc;
	
	long i,k;	
	double *constraints, sum_all_alpha = 0.0;

	for(i=1; i<=nb_data; i++)
	  for(k=1; k<=Q; k++)
	    if((model->alpha[i][k] < 0.0) || (model->alpha[i][k] > C[model->y[i]]))
	       {
	       printf("\nNo feasible solution: alpha[%ld][%ld] = %1.12lf  >  %lf\n\n",
		      i, k, model->alpha[i][k], C[model->y[i]]);
	       exit(0);
	       }
	     
	constraints = (double *) calloc(Q+1, sizeof(double));


	for(i=1; i<=nb_data; i++)
	  for(k=1; k<=Q; k++)
	    sum_all_alpha += model->alpha[i][k];

	for(k=1; k<=Q; k++)
	  constraints[k] = sum_all_alpha;

	for(i=1; i<=nb_data; i++)
	  for(k=1; k<=Q; k++)
	    constraints[k] -= Q * model->alpha[i][k];  

	for(k=1; k<=Q; k++)
	  constraints[k] /= Q;

	
	if(verbose) {
		printf("\nSatisfaction of the equality constraints:\n\n");

		for(k=1; k<=Q; k++)
		  printf("%11.8lf\n", constraints[k]);
	}
	
	norm = 0.0;

	for(k=1; k<=Q; k++)
	  norm += constraints[k] * constraints[k];

	norm = sqrt(norm);

	if(norm >= 1e-4)
	  {
	  printf("\nLarge deviation of the equality constraints (%lf)...\n",norm);
	  exit(0);
	  }

	if(alpha_file != NULL) {
		sprintf(command, "cp %s Save_alpha/.", alpha_file);
		rc = system(command);
	}
	
	free(constraints);

}

void LLW_check_margin_vect(long **margin_vect, long *nb_margin_vect, const struct Model *model)

{
	long i,k,l,y_i;
	const long Q = model->Q;
	const long nb_data = model->nb_data;
	const double *C = model->C;
	
	int over=false;
	double *average = (double*)malloc(sizeof(double) * (Q+1));

	double *radius = (double*)malloc(sizeof(double) * (Q+1));
	const double radius_step = 1000.0;

	for(i=1; i<=nb_data; i++)
	  for(k=1; k<=Q; k++)
	    margin_vect[i][k] = 0;

	for(k=1; k<=Q; k++) {
	  average[k] = C[k] / 2.0;;	 
	  radius[k] = C[k] / radius_step; 	  
 	}

	while(over == false) {

	  for(k=1; k<=Q; k++)
	    nb_margin_vect[k] = 0;

	  for(i=1; i<=nb_data; i++)
	    {
	    y_i = model->y[i];
	    for(k=1; k<=Q; k++)
	      if((k != y_i) && fabs(model->alpha[i][k] - average[model->y[i]]) <= radius[k])
		{
		margin_vect[i][k] = 1;
		nb_margin_vect[k]++;
		}
	    }

	  over = true;

	 for(k=1; k<=Q; k++)
	    if(nb_margin_vect[k] == 0)
	      {
	      over = false;
	      if((average[k] - radius[k]) > (average[k] / 5000.0))
		radius[k] += average[k] / radius_step;
	      else
		radius[k] = (radius[k] + average[k]) / 2.0;
	      }


	}

	free(radius);
	free(average);
}

/*
	Compute the training error
*/
double LLW_eval_training(double **gradient, long **mat_conf, const struct Cache *cache, const struct Model *model)
{
	long i,k,l,y_i;
	const long Q = model->Q;
	const long nb_data = model->nb_data;

	long ind_max = 0;
	double maximum, xi_ik;
	double *output = (double *)malloc((Q+1) * sizeof(double));
	double R_emp = 0.0;

	if(mat_conf != NULL) {
		for(k=1; k<=Q; k++)
		  for(l=1; l<=Q; l++)
		    mat_conf[k][l] = 0;
	}
	
	for(i=1; i<=nb_data; i++)  {
	  y_i = model->y[i];
	  output[y_i] = 0.0;
	  
	  for(k=1; k<=Q; k++)
	    if(k != y_i)
	      {
	      output[k] = -cache->H_alpha[i][k] + model->b_SVM[k];
	      output[y_i] -= output[k];
	      xi_ik = -gradient[i][k] + model->b_SVM[k];
	      if(xi_ik > 0.0)
	      	 R_emp += model->C[y_i] * xi_ik;
	      }

	  ind_max = 1;
	  maximum = output[1];

	  for(k=2; k<=Q; k++)
	    if(output[k] > maximum)
	      {
	      maximum = output[k];
	      ind_max = k;
	      }
	      
	  if(mat_conf != NULL) 
	     mat_conf[y_i][ind_max]++;
	}  

	free(output);
	return R_emp;
}

void LLW_estimate_b(double **gradient, long **margin_vect, long *nb_margin_vect, const struct Cache *cache, struct Model *model)
{

	long i,k;
	const long Q = model->Q;
	const long nb_data = model->nb_data;

	double sum = 0.0, max_alpha = 0.0;

	for(k=1; k<=Q; k++)
	   model->b_SVM[k] = 0.0;
	  
	for(i=1; i<=nb_data; i++)
	  for(k=1; k<=Q; k++)
	    if(margin_vect[i][k] == 1)
	      model->b_SVM[k] += gradient[i][k];

	if(cache->verbose)
		printf("\nVector b before centering\n\n");

	for(k=1; k<=Q; k++)
	  {
	  if(nb_margin_vect[k] > 0)
	     model->b_SVM[k] /= (double) nb_margin_vect[k];
	  else
	     printf("*** in LLW_estimate_b: nb_sum_vect[%ld] = 0\n", k);
	     
	  sum += model->b_SVM[k];
	  if(cache->verbose)
	  	printf(" b_%1ld = %lf\n", k, model->b_SVM[k]);
	  }

	sum /= Q;

	max_alpha = 0.0;
	for(i=1; i<=nb_data; i++)
	  for(k=1; k<=Q; k++)
		if(model->alpha[i][k] > max_alpha) 
			max_alpha = model->alpha[i][k];
			
	if(max_alpha > 0.0)
	  for(k=1; k<=Q; k++)
	    model->b_SVM[k] -= sum;
	else
	  for(k=1; k<=Q; k++)
	    model->b_SVM[k] = 0.0;

}

void LLW_compute_obj_dual(struct Cache *cache, struct Model *model)

{
	long i,k;
	const long Q = model->Q;
	const long nb_data = model->nb_data;
	
	double alpha_H_alpha = 0;
	double L_form_dual = 0;
	
	for(i=1; i<=nb_data; i++)
	  for(k=1; k<=Q; k++) {
	    alpha_H_alpha += cache->H_alpha[i][k] * model->alpha[i][k];
	    L_form_dual += model->alpha[i][k];
	  }
	  
	model->sum_all_alpha = L_form_dual;

	cache->alpha_H_alpha = alpha_H_alpha;
	cache->dual = -0.5*alpha_H_alpha + (L_form_dual / ((double)Q - 1.0));
}

void LLW_compute_obj_primal(double R_emp, struct Cache *cache, const struct Model *model)
{
 
	cache->primal = 0.5 * cache->alpha_H_alpha +  R_emp;
	if(cache->primal < cache->dual)
	  {
	  printf("\nInconsistency in the values of the objective functions...\n primal = %lf + %lf = %lf  <  %lf (dual)\n", 0.5 * cache->alpha_H_alpha, R_emp , cache->primal, cache->dual);
	/*  exit(0); */
	  }
	
}

