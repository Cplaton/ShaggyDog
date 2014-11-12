/* Copyright 2008-2010 Yann Guermeur                                       */

/* This program is free software; you can redistribute it and/or modify     */
/* it under the terms of the GNU General Public License as published by     */
/* the Free Software Foundation; either version 2 of the License, or        */
/* (at your option) any later version.                                      */

/* This program is distributed in the hope that it will be useful,          */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of           */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            */
/* GNU General Public License for more details.                             */

/* You should have received a copy of the GNU General Public License        */
/* along with this program; if not, write to the Free Software              */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA*/

/*--------------------------------------------------------------------------*/
/*  Name           : libevalMSVM_2.c                                        */
/*  Version        : 2.0                                                    */
/*  Creation       : 06/20/08                                               */
/*  Last update    : 01/20/11                                               */
/*  Subject        : Implementation of the M-SVM^2                          */
/*  Module         : Evaluation functions                                   */
/*  Author         : Fabien Lauer and Yann Guermeur Yann.Guermeur@loria.fr  */
/*--------------------------------------------------------------------------*/

#include "libevalMSVM_2.h"

/*
	Main evaluation function for the M-SVM^2:
	compute the ratio (dual / upper bound)
	
*/
double MSVM_2_eval(double *best_primal_upper_bound, double **gradient, double **H_alpha, double **H_tilde_alpha, struct Model *model, const int verbose, FILE *fp) {
	long i,k;
	const long Q = model->Q;
	const long nb_data = model->nb_data;

	double ratio;
	static double best_primal_cheap_upper_bound = HUGE_VAL;
	
	struct Cache cache;
	cache.H_alpha  = H_alpha;
	cache.H_tilde_alpha = H_tilde_alpha;
	cache.verbose = verbose;
	
	double **xi_opt = matrix(nb_data, Q);
	double **xi_save;
	static int maxiter_xi = 200;
		
//	MSVM_2_compute_gradient(gradient, &cache, model);	// will use -gradient for maximization of dual

	MSVM_2_estimate_b(gradient, &cache, model);		// here

	MSVM_2_compute_xi_opt(xi_opt, &cache, model);

	MSVM_2_check_xi_opt(xi_opt,gradient,&cache,model);	// and here

	MSVM_2_compute_obj_dual(&cache,model);
	MSVM_2_compute_obj_primal(&cache,xi_opt,model); // this is not a true value of the primal 
						 // should only be used for testing purposes

	if(fp != NULL)
		fprintf(fp, "%lf %lf ",cache.dual,cache.primal);

	/*
		---- Choose upper bound method here:
	*/
	static int upperbound = 2;	// start with fast method 

	switch(upperbound) {

		case 1 :					// simple gradient projection over xi
			cache.primal = *best_primal_upper_bound;
			
			MSVM_2_correct_xi_opt(xi_opt,&cache,model,maxiter_xi);
			
			// Make sure the upper bound does not increase
			if(cache.primal > *best_primal_upper_bound)
				cache.primal = *best_primal_upper_bound;
			else
				*best_primal_upper_bound = cache.primal;
			if(cache.primal_cheap > best_primal_cheap_upper_bound)
				cache.primal_cheap = best_primal_cheap_upper_bound;
			else
				best_primal_cheap_upper_bound = cache.primal_cheap;
				
			// Write to log file
			if(fp != NULL)
				fprintf(fp, "%lf %lf ",cache.primal_cheap,cache.primal);
			break;
		case 2 :
			MSVM_2_minimal_xi(1,xi_opt,&cache,model);
			cache.primal = MSVM_2_compute_primal_obj(xi_opt, &cache, model);
			
			// Make sure the upper bound does not increase
			if(cache.primal > *best_primal_upper_bound)
				cache.primal = *best_primal_upper_bound;
			else
				*best_primal_upper_bound = cache.primal;
			cache.primal_cheap = cache.primal;

			// Write to log file
			if(fp != NULL)
				fprintf(fp, "%lf %lf ",cache.primal_cheap,cache.primal);
				
			
			break;
			
		case 4 :					// grid search over b + optimization over xi
			MSVM_2_correct_xi_opt(xi_opt,&cache,model,maxiter_xi);
			if(fp != NULL)
				fprintf(fp, "%lf %lf ",cache.primal_cheap,cache.primal);
			MSVM_2_optimize_b(xi_opt, &cache, model);		

			if(fp != NULL)
				fprintf(fp, "%lf %lf ",cache.primal_cheap,cache.primal);
			break;
		case 5 :					// minimum xi
			/* Save the current xi_opt*/
			xi_save = matrix(nb_data,Q);

			for(i=1;i<=nb_data;i++)
				for(k=1;k<=Q;k++)
					xi_save[i][k] = xi_opt[i][k];

			MSVM_2_minimal_xi(1,xi_opt,&cache,model);
			cache.primal_cheap = MSVM_2_compute_primal_obj(xi_opt,&cache,model);

			MSVM_2_minimal_xi(0,xi_opt,&cache,model);
			cache.primal = MSVM_2_compute_primal_obj(xi_opt,&cache,model);
			/* Restore original xi_opt */
			for(i=1;i<=nb_data;i++)
				for(k=1;k<=Q;k++)
					xi_opt[i][k] = xi_save[i][k];

			if(fp != NULL)
				fprintf(fp, "%lf %lf ",cache.primal_cheap,cache.primal);
			MSVM_2_optimize_b(xi_opt, &cache, model);		

			if(fp != NULL)
				fprintf(fp, "%lf %lf ",cache.primal_cheap,cache.primal);
		
			free(xi_save[1]);free(xi_save);
			break;
	
		default :
			//printf("Do nothing .\n");
			break;
	}
	if(fp != NULL) 
		fprintf(fp,"\n");
	
	// Compute ratio
	ratio = cache.dual / cache.primal;
	
	// Comput training error
	long **mat_conf = matrix_l(Q,Q);	
	long training_errors = MSVM_2_eval_training(mat_conf, &cache, model);
	model->training_error = (double)training_errors/(double)model->nb_data;
	
	// Switch to slower but more accurate method after 75%
	// (gradient projection optimization of xi)
	// but only for a small number of categories Q < 20
	if(ratio > 0.75 && model->Q < 20)
		upperbound = 1;
		
	// Increase the number of iterations in the optimization of xi
	if(ratio > 0.90)
		maxiter_xi = 500;	// after 90%
	if(ratio > 0.95)
		maxiter_xi = 1000;	// after 95%
				
	// Free memory
	free(xi_opt[1]); free(xi_opt);
	free(mat_conf[1]);free(mat_conf);
	
	return ratio;

}


/*
	output = MSVM_2_function( k, x, model)
	
	Compute the real-valued output of model (without bias term)
	 for the k-th category on pattern x
	
*/
double MSVM_2_function(long category, void *vector, const struct Model *model)
{
	const long nb_data = model->nb_data;
	const enum Kernel_type nature_kernel = model->nature_kernel;
	const double *kernel_par = model->kernel_par;
	long dim_input = model->dim_input;
	
	long index;

	double partial, result = 0.0;

	for(index=1; index<=nb_data; index++)
	  {
	  partial = model->partial_average_alpha[index] - model->alpha[index][category];

	  if(partial != 0.0)
	  result += partial * ker(nature_kernel, model->X[index], vector, dim_input, kernel_par);
	  }

	return result;

}

/*
	label = MSVM_2_classify(x, model, real_outputs)
	
	Classify x and output the label. 
	  
	If real_outputs is not NULL, also compute the Q real-valued outputs
	 of the model and store them in (real_outputs[1]...real_outpust[Q]).
	 
	Note: this function is faster than calling Q times MSVM_2_function 		
*/
long MSVM_2_classify(void *vector, const struct Model *model, double *real_outputs)
{
	const long nb_data = model->nb_data;
	const long Q = model->Q;	
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
	    partial = model->partial_average_alpha[i];
	    
	    if(partial != 0.0) {
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

int MSVM_2_check_feasible_sol(const struct Model *model, char *alpha_file)

{
	long i,k;
	char command[taille];
	const long Q = model->Q;
	const long nb_data = model->nb_data;

	double *constraints, norm;
	int feasible = true;
	
	for(i=1; i<=nb_data; i++)
	  for(k=1; k<=Q; k++)
	    if(((k == model->y[i]) && (model->alpha[i][k] != 0.0)) || (model->alpha[i][k] < 0.0))
	      {
	      printf("\nNo feasible solution: alpha[%ld][%ld] = %lf\n\n",
		     i, k, model->alpha[i][k]);
		     
	      feasible = false;
	      }
	     
	constraints = (double *) calloc(Q+1, sizeof(double));

	for(k=1; k<=Q; k++)
	  constraints[k] = model->sum_all_alpha / Q;

	for(i=1; i<=nb_data; i++)
	  for(k=1; k<=Q; k++)
	    constraints[k] -= model->alpha[i][k];  

	printf("\nSatisfaction of the equality constraints:\n\n");

	for(k=1; k<=Q; k++)
	  printf("%s%e\n", constraints[k] < 0.0 ? " " : "  ", constraints[k]);

	norm = 0.0;

	for(k=1; k<=Q; k++)
	  norm += constraints[k] * constraints[k];

	norm = sqrt(norm);

	if(norm >= very_small)
	  {
	  printf("\nLarge deviation of the equality constraints...\n");
	  feasible = false;
	  Pause("");
	  }

	sprintf(command, "cp %s Feasible/.", alpha_file);
	int rc = system(command);

	free(constraints);
	return feasible;
}

void MSVM_2_compute_gradient(double **gradient, struct Cache *cache, const struct Model *model)

{
	long i,j,k,y_i;
	double partial,kernel;

	const long Q = model->Q;
	const long nb_data = model->nb_data;
	const enum Kernel_type nature_kernel = model->nature_kernel;
	const double *kernel_par = model->kernel_par;
	
	const long dim_input =  model->dim_input;
	const double Qinv = 1. / (Q-1.0);
	
	if(cache->verbose)
		printf("\nStart of the gradient computation...\n");

	for(i=1; i<=nb_data; i++)
	  {
	  if((i%step) == 0 && cache->verbose)
	    printf("\n%9ld", i);
	  y_i = model->y[i];

	  for(k=1; k<=Q; k++)
	    {
	    cache->H_alpha[i][k] = 0.0;
	    cache->H_tilde_alpha[i][k] = 0.0;
	    if(k != y_i)
	      {
	      for(j=1; j<=nb_data; j++)
		{
		partial = model->alpha[j][k] - model->partial_average_alpha[j];

		if(partial != 0.0)
		  {
		  kernel = ker(nature_kernel, model->X[i], model->X[j], dim_input, kernel_par);
		  cache->H_alpha[i][k] += partial * kernel;
		  if(i == j)
		    kernel += 1./(2 * model->C[y_i]);
		  cache->H_tilde_alpha[i][k] += partial * kernel;
		  }

		}
	      gradient[i][k] = - cache->H_tilde_alpha[i][k] + Qinv;
	      }
	    else
	      gradient[i][k] = 0.0;	      
	    }
	  }


	if(cache->verbose)
		printf("\n\nEnd of the gradient computation...\n");

}

/*
	Evaluate the training error
*/
long MSVM_2_eval_training(long **mat_conf, const struct Cache *cache, const struct Model *model)
{
	long i,k,l,y_i,ind_max;
	const long Q = model->Q;
	const long nb_data = model->nb_data;

	double *output = (double *) calloc(Q+1, sizeof(double));
	double maximum;
	long errors=0;
	
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
	  if(y_i != ind_max)
	  	errors++;
	}  

	free(output);
	return errors;
}

void MSVM_2_compute_xi_opt(double **xi_opt, struct Cache *cache, const struct Model *model)

{
	long i,k,y_i;
	const long Q = model->Q;
	const long nb_data = model->nb_data;

	cache->sum_all_xi_opt = 0.0;
	
	for(i=1; i<=nb_data; i++)
	  {
	  y_i = model->y[i];
	  xi_opt[i][y_i] = 0.0;

	  for(k=1; k<=Q; k++)
	    if(k != y_i)
	      {
	      xi_opt[i][k] = (model->alpha[i][k] - model->partial_average_alpha[i]) / (2 * model->C[y_i]);
	      cache->sum_all_xi_opt += xi_opt[i][k];
	      }
	  }

}

void MSVM_2_check_xi_opt(double **xi_opt, double **gradient, const struct Cache *cache, const struct Model *model)

{
	long i,k,y_i;
	const long Q = model->Q;
	const long nb_data = model->nb_data;
	
	const double Qinv = 1. / (Q-1.0);
	double partial;
	
	for(i=1; i<=nb_data; i++)
	  {
	  y_i = model->y[i];

	  for(k=1; k<=Q; k++)
	   if(k != y_i) {
	      partial = cache->H_alpha[i][k] - Qinv + xi_opt[i][k] - gradient[i][k];
	      if(fabs(partial) > very_small)
		{
		printf("\nProblem in the computation of the slack variables...\n");
		printf(" Ha_ik - Qinv + xi_ik + g_ik = %lf - %lf + %lf + %lf = %lf\n",cache->H_alpha[i][k],Qinv , xi_opt[i][k], -gradient[i][k],partial);
		printf("(sum_all_alpha = %lf)\n",model->sum_all_alpha);
		exit(0);
		}
	      }
	      
	   }
	if(cache->verbose)
		printf("\nThe values of the slack variables have been checked...\n");

}

double MSVM_2_compute_primal_obj(double **xi_opt, const struct Cache *cache, const struct Model *model) {
	long i,k,l,y_i;
	const long Q = model->Q;
	const long nb_data = model->nb_data;
	
	double J_xi_i;
	double J_xi = 0;
	
	for(i=1; i<=nb_data; i++) {
	  y_i = model->y[i];
	  J_xi_i = 0;
	  for(k=1; k<=Q; k++)
	    if(k != y_i) {
	    	for(l=1; l<=Q; l++) {
		  if(l != y_i) 
		  	J_xi_i += xi_opt[i][k] * xi_opt[i][l];
		}	
		J_xi_i += xi_opt[i][k] * xi_opt[i][k];
	     }	      
	  J_xi += model->C[y_i] * J_xi_i;
	}
    return (0.5 * cache->alpha_H_alpha +  J_xi);
}
/*
	Put xi on the constraints
*/
void MSVM_2_minimal_xi(const int keepok,double **xi_opt, const struct Cache *cache, const struct Model *model) {
	long i,k,y_i;
	double bound;
	const long Q = model->Q;
	const long nb_data = model->nb_data;
	const double Qd = (1.0 / (Q - 1.0));
	
	if (keepok) {
		// keep previous values of xi that are feasible
		for(i=1; i<=nb_data; i++) {
		  y_i = model->y[i];
		  for(k=1; k<=Q; k++)
			if(k != y_i)
			  {
				bound = -cache->H_alpha[i][k] + model->b_SVM[k] + Qd ;
				if(xi_opt[i][k] < bound)
					  xi_opt[i][k] = bound;
			 }			     
		}

	}
	else {
		// completely reinitialize xi
		for(i=1; i<=nb_data; i++) {
		  y_i = model->y[i];
		  for(k=1; k<=Q; k++)
			if(k != y_i)
			  {
			  xi_opt[i][k] = -cache->H_alpha[i][k] + model->b_SVM[k] + Qd;
			 }			     
		}
	}
}
/*
	Compute an upper bound on the optimum with gradient projection
*/
void MSVM_2_correct_xi_opt(double **xi_opt, struct Cache *cache, const struct Model *model, const int MaxIter) {

	const double *C = model->C;
	const long Q = model->Q;
	const long nb_data = model->nb_data;

	double **bound_xi = matrix(nb_data, Q);
	double **gradient_xi = matrix(nb_data, Q);
	double **descent_xi = matrix(nb_data, Q);
	long **activeset_xi = matrix_l(nb_data, Q);
	int converged_xi,nA;
	double gamma,gamma_max,gamma_denominateur,J_xi,J_xi_i,norm_descent,minu;
	long i,k,l,y_i,i_minu=0,k_minu=0;
	
	const double epsilon_xi = 0.0001;
	
	const int computeCost = 0;
	
	// Compute the constraints on the slack variables 
	for(i=1; i<=nb_data; i++) {
	  y_i = model->y[i];

	  for(k=1; k<=Q; k++)
	    if(k != y_i)
	      {
	      bound_xi[i][k] = -cache->H_alpha[i][k] + model->b_SVM[k] + (1.0 / (Q - 1.0)) ;
	      // Push the slack variable into the feasible set for initialization
	      if(xi_opt[i][k] < bound_xi[i][k]) 
			xi_opt[i][k] = bound_xi[i][k];
	     }		
	     
	}
  	
  	// compute a rough upper bound on the primal (take the first feasible point)
  	
	 //compute J_xi(xi)
	J_xi = 0;
	for(i=1; i<=nb_data; i++) {
	  y_i = model->y[i];
	  J_xi_i = 0;
	  for(k=1; k<=Q; k++)
	    if(k != y_i) {
	    	for(l=1; l<=Q; l++) {
		  if(l != y_i) 
		  	J_xi_i += xi_opt[i][k] * xi_opt[i][l];
		}	
		J_xi_i += xi_opt[i][k] * xi_opt[i][k];
	     }	   	
	   J_xi += model->C[y_i] * J_xi_i;		     
	}
	cache->primal_cheap = 0.5 * cache->alpha_H_alpha + J_xi;
	if(cache->verbose)
		printf("    Cheap upper bound on primal objective: %lf (ratio = %3.6lf \%%)\n", cache->primal_cheap,100*cache->dual/cache->primal_cheap);

  	// Gradient projection algorithm for xi
  	int iter_xi = 0;
  	converged_xi = 0;
	int stop_because_useless = 0; // stop if cost J_xi too large compared to best_J_xi after MaxIter/2
	// best_J_xi is computed from best_primal_upper_bound stored in cache->primal
	const double best_J_xi = (cache->primal*(1.+10./(double)MaxIter) - 0.5 * cache->alpha_H_alpha);
	
  	// Compute the active set	
	nA = 0;
	for(i=1; i<=nb_data; i++) {
	  y_i = model->y[i];
	  for(k=1; k<=Q; k++)
	    if(k != y_i) {
	      if(xi_opt[i][k] <= bound_xi[i][k]) {
		activeset_xi[i][k] = 1;
		nA++;
	      }			
	      else
	      	activeset_xi[i][k] = 0;
	    }
	}
	
	// Main loop
	do {	
			
		// inner computations
		norm_descent = 0;
		gamma_max = HUGE_VAL;
		for (i=1; i<=nb_data; i++) {
		  y_i = model->y[i];
		  for(k=1; k<=Q; k++)
		    if(k != y_i) {
		      // Compute gradient of cost wrt to xi = 4*xi_ik + 2*sum_l\k xi_il = 2*(xi_ik + sum_l xi_il)
		      gradient_xi[i][k] = xi_opt[i][k];
		      for(l=1; l<=Q; l++) {
			  if(l != y_i) 
			    gradient_xi[i][k] += xi_opt[i][l];
		      }
		      gradient_xi[i][k] *= 2;
			
		      // descent direction (project -gradient on the constraints)
		      if( activeset_xi[i][k]) {
		      	// descent direction is 0 along the constraints
		      	descent_xi[i][k] = 0.0;		      	
		      }
		      else  {
			descent_xi[i][k] = -gradient_xi[i][k];
			
			// norm of descent to check convergence
			norm_descent += descent_xi[i][k] * descent_xi[i][k];
		
			// maximal step size
			if(descent_xi[i][k] < 0) {
			  gamma = (bound_xi[i][k] - xi_opt[i][k]) / descent_xi[i][k];
			  if (gamma < gamma_max)
			  	gamma_max = gamma;
			}
		      }
		   
		   }
		   
		}
		norm_descent = sqrt(norm_descent);
		
		// Check convergence
		if(norm_descent < epsilon_xi) {		
			// Compute vector u
			// u = gradient_xi[i][k] for (i,k) in active set, 0 for the others

			// check negative entries in u (if none then optimum found and stop)
			converged_xi = 1;			
			minu = -epsilon_xi;
			for(i=1;i<=nb_data; i++) {
				y_i = model->y[i];
				for(k=1; k<=Q; k++)
			    		if(k != y_i && activeset_xi[i][k] && gradient_xi[i][k] < minu) { // equivalent to u<-epsilon						
						converged_xi=0;
						minu = gradient_xi[i][k];
						i_minu = i; k_minu = k;
					
					}
			}		
			// relax the constraint with minimum u	
			if (!converged_xi) {	
				activeset_xi[i_minu][k_minu] = 0;	
				nA--;	
				descent_xi[i_minu][k_minu] = -minu;		
				norm_descent = -minu;
				//printf("relaxing constraint (%ld,%ld) : descent = %lf\n",i_minu,k_minu,descent_xi[i_minu][k_minu]);		
			}
		}
		
		if(!converged_xi) {
			// Line search for gradient projection step in xi
			// gamma^* = -gradient^T d / 2*d^T M d
			gamma_denominateur = 0;
			gamma = 0;
			for(i=1;i<=nb_data; i++) {
				y_i = model->y[i];
				for(k=1; k<=Q; k++)
			    		if(k != y_i && !activeset_xi[i][k]) {
			    			gamma -= gradient_xi[i][k] * descent_xi[i][k];
			    			gamma_denominateur += descent_xi[i][k] * descent_xi[i][k]; 
			    			for(l=1; l<=Q; l++) {
						  if(l != y_i) {						  	
					  		gamma_denominateur += descent_xi[i][k] * descent_xi[i][l];
					  	  }
						}
			    		}
			}					
			gamma /= 2*gamma_denominateur;
			if(gamma > gamma_max)
				gamma = gamma_max;
				
			// Take a step 
			for(i=1;i<=nb_data; i++) {
				y_i = model->y[i];
				for(k=1; k<=Q; k++)
			    		if(k != y_i && !activeset_xi[i][k]) {		    			
			    			xi_opt[i][k] += gamma * descent_xi[i][k];
			    			
			    			// Update the active set
			    			if(xi_opt[i][k] <= bound_xi[i][k]) {
			    				activeset_xi[i][k] = 1;
							nA++;
			    			}
/*			    			else {
			    				activeset_xi[i][k] = 0;
			    				nA--;
			    			}*/
			    		}
			}
			
			
			if(computeCost || (iter_xi == MaxIter/2)) {			
				//evaluate the objective function J_xi(xi) 
				J_xi = 0;
				for(i=1; i<=nb_data; i++) {
				  y_i = model->y[i];
				  J_xi_i = 0;
				  for(k=1; k<=Q; k++)
				    if(k != y_i) {
				    	for(l=1; l<=Q; l++) {
					  if(l != y_i) 
					  	J_xi_i += xi_opt[i][k] * xi_opt[i][l];
					}	
					J_xi_i += xi_opt[i][k] * xi_opt[i][k];
				     }	      
				  J_xi += model->C[y_i] * J_xi_i;
				}
		       	}
		       	
		       	if(iter_xi == MaxIter/2 ) {
		       		if(J_xi > best_J_xi) {
		       			//printf("stopping useless optimimization at iter = %d...\n",iter_xi);
		       			stop_because_useless = 1;
		       		}
		       			
		       	}
		}				
					
		iter_xi++;	
	} while (!converged_xi && iter_xi < MaxIter && !stop_because_useless);	
		       			
	//printf("iter=%d,  %d active constraints / %ld; norm_d = %lf  : J_xi(xi) = %3.15lf \n",iter_xi,nA,nb_data*(Q-1),norm_descent,J_xi);

	// Compute optimized upper bound 
	if(!computeCost && !stop_because_useless) {
		J_xi = 0;
		for(i=1; i<=nb_data; i++) {
		  y_i = model->y[i];
		  J_xi_i = 0;
		  for(k=1; k<=Q; k++)
		    if(k != y_i) {
		    	for(l=1; l<=Q; l++) {
			  if(l != y_i) 
			  	J_xi_i += xi_opt[i][k] * xi_opt[i][l];
			}	
			J_xi_i += xi_opt[i][k] * xi_opt[i][k];
		     }	      
		  J_xi += model->C[y_i] * J_xi_i;
		}
	}
	/*
	if(0.5 * cache->alpha_H_alpha + J_xi > cache->primal) 
		printf("USELESS optim at iter = %d... %lf / %lf\n",iter_xi, 0.5 * cache->alpha_H_alpha + J_xi,cache->primal);
*/

	cache->primal = 0.5 * cache->alpha_H_alpha + J_xi;
	if(cache->verbose)
		printf("Optimized upper bound on primal objective: %lf (ratio = %3.6lf \%%)\n\n", cache->primal,100*cache->dual/cache->primal);

	free(bound_xi[1]); free(bound_xi);
	free(descent_xi[1]); free(descent_xi);
	free(gradient_xi[1]); free(gradient_xi);
	free(activeset_xi[1]); free(activeset_xi);
	return;
}


void MSVM_2_optimize_b(double **xi_opt, struct Cache *cache, struct Model *model)

{
	const long Q = model->Q;
	const long nb_data = model->nb_data;

	long i,k,compteur, portion, class, Iteration;

	double *grid_index, *b_SVM_base, *best_b_SVM;

	double minprimal = cache->primal;
	double primal;
	double minprimalcheap = cache->primal_cheap;
	double bstep = 0.1;

	double **xi_save = matrix(nb_data,Q);
	for(i=1;i<=nb_data;i++)
		for(k=1;k<=Q;k++)
			xi_save[i][k] = xi_opt[i][k];

	grid_index = (double *) calloc(Q+1, sizeof(double));
	b_SVM_base = (double *) calloc(Q+1, sizeof(double));
	best_b_SVM = (double *) calloc(Q+1, sizeof(double));

	for(class=1; class<=Q; class++)
	  best_b_SVM[class] = model->b_SVM[class];


	grid_index[1] = 0.0;
	b_SVM_base[1] = 0.0;

	for(Iteration=1; Iteration <=10; Iteration++)
	  {
	  printf("Iteration = %ld\n",Iteration);
	  for(k=2; k<=Q; k++)
	    b_SVM_base[k] = model->b_SVM[k] - (5.0 * bstep);

	  for(compteur=0; compteur<(long)pow(10.0, (double)(Q-1)); compteur++)
	    {
	  
	/*    printf("\nPosition in the grid: %6d\n", compteur); */

	    portion = compteur;
	    for(class=2; class<=Q; class++)
	      {
	      grid_index[class] = (double) (portion / (long) pow(10.0, (double) (Q-class)));
	      portion -= (long) (grid_index[class] * pow(10.0, (double) (Q-class)));
	      }

	    for(class=2; class<=Q; class++)
	      model->b_SVM[class] = b_SVM_base[class] + (grid_index[class] * bstep);

	    standardize_b(model->b_SVM,Q);
	    //correct_xi_opt();	

	    MSVM_2_minimal_xi(0,xi_opt,cache,model);
		primal = MSVM_2_compute_primal_obj(xi_opt,cache,model);

		/* Restore original xi_opt */
		for(i=1;i<=nb_data;i++)
			for(k=1;k<=Q;k++)
				xi_opt[i][k] = xi_save[i][k];

		MSVM_2_minimal_xi(1,xi_opt,cache,model);
		cache->primal_cheap = MSVM_2_compute_primal_obj(xi_opt,cache,model);
	
	    if(primal < minprimal) {
	    	minprimal = primal;
	    	for(class=1; class<=Q; class++)
	    		best_b_SVM[class] = model->b_SVM[class];     
	      }
	  	if(cache->primal_cheap < minprimalcheap) 
	    	minprimalcheap = cache->primal_cheap;
	    	
	    }

	  for(class=1; class<=Q; class++)
	    model->b_SVM[class] = best_b_SVM[class];

	    standardize_b(model->b_SVM,Q);
	    //correct_xi_opt();

	    MSVM_2_minimal_xi(0,xi_opt,cache,model);
		primal = MSVM_2_compute_primal_obj(xi_opt,cache,model);

		/* Restore original xi_opt */
		for(i=1;i<=nb_data;i++)
			for(k=1;k<=Q;k++)
				xi_opt[i][k] = xi_save[i][k];

		MSVM_2_minimal_xi(1,xi_opt,cache,model);
		cache->primal_cheap = MSVM_2_compute_primal_obj(xi_opt,cache,model);

	    if(primal < minprimal) {
	    	minprimal = primal;
	    	for(class=1; class<=Q; class++)
	    		best_b_SVM[class] = model->b_SVM[class];     
	      }
	  	if(cache->primal_cheap < minprimalcheap) 
	    	minprimalcheap = cache->primal_cheap;

	  bstep /= 2.0;
	  }

	cache->primal = minprimal;
	cache->primal_cheap = minprimalcheap;

	//standardize_b();

	if(cache->verbose) {	
		printf("\nNew components of the b vector:\n");

		printf("\n");
		for(class=1; class<=Q; class++)
		  printf("%11.6f\n", model->b_SVM[class]);
	}
	
	// Free templates
	free(xi_save[1]);free(xi_save);
	free(grid_index);
	free(b_SVM_base);
	free(best_b_SVM);


}


void MSVM_2_estimate_b(double **gradient,  const struct Cache *cache, struct Model *model)

{

	long i,k;
	const long Q = model->Q;
	const long nb_data = model->nb_data;

	long *nb_sup_vect = (long *) calloc(Q+1, sizeof(long));

	for(k=1; k<=Q; k++)
	  {
	  nb_sup_vect[k] = 0;
	  model->b_SVM[k] = 0.0;
	  }
 
	double sum = 0.0;

	if(cache->verbose)
		printf("\nVector b before centering\n\n");

	for(i=1; i<=nb_data; i++)
	  for(k=1; k<=Q; k++)
	    if(model->alpha[i][k] > 0)
	      {
	      nb_sup_vect[k]++;
	      model->b_SVM[k] += gradient[i][k]; // should be '-=' but we have '-gradient' in gradient
	      }

	for(k=1; k<=Q; k++)
	  {
	  if(nb_sup_vect[k] > 0)
	    model->b_SVM[k] /= (double) nb_sup_vect[k];
	  if(cache->verbose)
		printf(" b_%1ld = %c%e\n", k, (model->b_SVM[k] >= 0) ? ' ': '-', fabs(model->b_SVM[k]));
	  sum += model->b_SVM[k];
	  }

	if(cache->verbose) {
		printf("\nSum of the components of b: %e\n", sum);
		Pause("");
	}
	
	if(sum != 0.0)
	  {
	  sum /= Q;
	  for(k=1; k<=Q; k++)
	    model->b_SVM[k] -= sum;
	  }


	free(nb_sup_vect);
}

void MSVM_2_compute_obj_dual(struct Cache *cache, const struct Model *model)

{
	long i,k;
	const long Q = model->Q;
	const long nb_data = model->nb_data;
	
	double alpha_H_alpha = 0;
	double alpha_H_tilde_alpha = 0;

	for(i=1; i<=nb_data; i++)
	  for(k=1; k<=Q; k++)
	    {
	    alpha_H_alpha += cache->H_alpha[i][k] * model->alpha[i][k];
	    alpha_H_tilde_alpha += cache->H_tilde_alpha[i][k] * model->alpha[i][k];
	    
	    
	    }
	    
	double Q_form_dual = alpha_H_tilde_alpha;
	double L_form_dual = model->sum_all_alpha / (Q - 1.0);

	cache->alpha_H_alpha = alpha_H_alpha;
	cache->alpha_H_tilde_alpha = alpha_H_tilde_alpha;
	cache->dual = -0.5 * Q_form_dual + L_form_dual;
}

void MSVM_2_compute_obj_primal(struct Cache *cache, double **xi_opt, const struct Model *model)

{
	long i,k,l;
	const long Q = model->Q;
	double *C = model->C;
	const long nb_data = model->nb_data;
	
	double partial,primal, ratio;
	double xi_M_xi = 0.0;

	for(i=1; i<=nb_data; i++)
	  for(k=1; k<=Q; k++)
	    for(l=1; l<=Q; l++)
	      {
	      partial = (k == l) ? 2.0 : 1.0;
	      xi_M_xi += C[model->y[i]] * partial * xi_opt[i][k] * xi_opt[i][l];
	      }

	primal = 0.5 * cache->alpha_H_alpha + xi_M_xi;

	if(cache->verbose) {
		printf("\nEmpirical contribution to the primal objective function: %lf\n", xi_M_xi);
	
		if(fabs(model->sum_all_alpha - 2.0 *  Q * cache->sum_all_xi_opt) < very_small)
		  printf("1^T alpha = 2Q 1^T xi = %e\n", model->sum_all_alpha);
	
		printf("\n                Dual objective function: %e", cache->dual);
		printf("\nUnconstrained primal objective function: %e", primal);
	
		if(primal != 0.0) {
		  ratio = cache->dual / primal;
		  printf("\n(ratio %5.2lf\%% )\n", 100.0 * ratio);
		}
		Pause("");
	}
	cache->primal = primal;
}

