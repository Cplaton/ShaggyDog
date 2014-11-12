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
/*  Name           : libevalMSVM_WW.c                                         */
/*  Version        : 2.0                                                      */
/*  Creation       : 04/30/00                                                 */
/*  Last update    : 01/20/11                                                 */
/*  Subject        : Implementation of the M-SVM of Weston and Watkins        */
/*  Module         : Evaluation functions                                     */
/*  Author         : Fabien Lauer and Yann Guermeur Yann.Guermeur@loria.fr    */
/*----------------------------------------------------------------------------*/
 

#include "libevalMSVM_WW.h"

void WW_estimate_b_fast(double **gradient, const struct Cache *cache, struct Model *model) {

	long i,k,l;
	const long Q = model->Q;
	const long nb_data = model->nb_data;
	const double *C = model->C;

	double **delta_b_SVM = matrix(Q, Q);
	
	double *average = (double*)malloc(sizeof(double) * (Q+1));
	double *min_dist = (double *)malloc(sizeof(double) * (Q+1));
	long *closest_i = (long *)malloc(sizeof(long) * (Q+1));
	long *closest_y_i = (long *)malloc(sizeof(long) * (Q+1));
	
	
	for(k=1; k<=Q; k++) {
	    average[k] = C[k] / 2.0;;
	    min_dist[k] = C[k]/2.0;
      	    closest_i[k] = 0;
  	    closest_y_i[k] = 0;
      	}
      	
      	// Find the alpha_ik closest to C/2 for each i and k
	  for(i=1; i<=nb_data; i++)
	    {
	    for(k=1; k<=Q; k++)
	      if(fabs(model->alpha[i][k] - average[k]) < min_dist[k])
		{
		min_dist[k] = fabs(model->alpha[i][k] - average[k]);
		closest_i[k] = i;
		closest_y_i[k] = model->y[i];
		}
	    }

	  /* Check the presence of margin SV for all pairs of classes
	  for(k=1; k<=Q-1; k++)
	    if(closest_i[k] == 0)
	      for(l=k+1; l<=Q; l++)
	        if(closest_y_i[l] != k)
		{
		printf("No margin SV for classes %ld/%ld\n",k,l);
		
		}
	*/
	
	// init b values
	for(k=1; k<=Q; k++)
	   model->b_SVM[k] = 0.0;
	  
	// and delta_b
	for(k=1; k<=Q; k++)
	  for(l=1; l<=Q; l++)
	    delta_b_SVM[k][l] = 0.0;

	// compute delta_b
	  for(k=1; k<=Q; k++)	  	
	    if(closest_i[k] != 0)
	      delta_b_SVM[closest_y_i[k]][k] = -gradient[closest_i[k]][k];

	// compute _b_tilde
	for(k=2; k<=Q; k++)
	    model->b_SVM[k] = (delta_b_SVM[k][1] - delta_b_SVM[1][k]) / 2.0;
	 
	// compute b
    	standardize_b(model->b_SVM,Q);
	
	if(cache->verbose) {
		printf("\nComponents of the b vector:\n");
		printf("\n");
		for(k=1; k<=Q; k++)
		  printf("%11.6f\n", model->b_SVM[k]);
	}
	
	free(delta_b_SVM[1]); free(delta_b_SVM);
	free(average);
	free(min_dist);
	free(closest_i);free(closest_y_i);
}

/*
	Main evalutaion function for WW: compute the ratio dual/upper bound
*/
double MSVM_WW_eval(double *best_primal_upper_bound, double **gradient, double **H_alpha, struct Model *model, const int verbose, FILE *fp) {

	long i,k,check;

	const long Q = model->Q;
	const long nb_data = model->nb_data;

	double R_emp, ratio, max_alpha = 0;	
	int bad_b = false;
	char choice='a';	// default to automatic choice
	
	struct Cache cache;
	cache.verbose = verbose;
	cache.H_alpha = H_alpha; 
	
	long **mat_conf = matrix_l(Q,Q); 
	long training_errors;
	
	long **margin_vect = matrix_l(nb_data, Q);
	long **nb_margin_vect = matrix_l(Q, Q);
	
	int test_NoAlphaCloseToC;
	
	WW_check_feasible_sol(model,NULL,verbose);

	if(verbose)
		WW_check_sup_vect(model);
		
	WW_check_margin_vect(margin_vect,nb_margin_vect,model);
		
	if(model->nature_kernel == 1 && verbose) {
		// compute the weights of the linear model
	  	WW_compute_W(model);
  		printf("\nMatrix of the vectors defining the separating hyperplanes\n");
		display_mat(model->W, model->dim_input, Q, 12, 6);
		Pause("");
	}
  	 
  	
	WW_estimate_b(gradient, margin_vect, nb_margin_vect, &cache, model);
//	WW_estimate_b_fast(gradient, &cache, model);
	
	R_emp = WW_eval_training(gradient,mat_conf,model);

	// Compute training error
	training_errors = model->nb_data;
	for(i=1;i<=Q;i++)
		training_errors -= mat_conf[i][i];
	
	model->training_error = (double)training_errors/(double)model->nb_data;
	
	// Compute the ratio dual/upper bound (stopping criterion)

	WW_compute_obj_dual(&cache,model);
	WW_compute_obj_primal(R_emp,&cache,model);
	
	// Make sure the upper bound does not increase
	if(cache.primal > *best_primal_upper_bound)
		cache.primal = *best_primal_upper_bound;
	else
		*best_primal_upper_bound = cache.primal;
	
	test_NoAlphaCloseToC = 1;
	for(i=1; i<=nb_data; i++)
		for(k=1;k<=Q; k++) {
			if(model->alpha[i][k] > max_alpha)
				max_alpha = model->alpha[i][k];
			#ifdef _WIN32	
			if(model->alpha[i][k] > model->C[model->y[i]] * (1.0 - Small))
			#else
			if(model->alpha[i][k] > model->C[model->y[i]] * (1.0 - small))
			#endif
				test_NoAlphaCloseToC = 0;
		}
			 
	if(test_NoAlphaCloseToC && (0.5 * cache.alpha_H_alpha >= cache.dual))
	  {
	  check = 0;
	  for(k=1; k<=Q; k++)
	    check += mat_conf[k][k];

	  if(check == nb_data)
	    {
	    bad_b = true;
	    if(cache.primal > (1.5 * 0.5 * cache.alpha_H_alpha))
	      printf("\nBeware, very poor estimates of the bias (primal = %lf)\n", cache.primal);
	    cache.primal = 0.5 * cache.alpha_H_alpha;
	    }
	  }
	
	ratio = cache.dual / cache.primal;
	if(verbose) {
		printf("\nDifference between the primal and the dual: %lf -> %lf", cache.primal, cache.dual);
		printf("\n(ratio %5.2lf \%%)\n", 100.0 * ratio);
	
		printf("\n*** Training performance\n");
		display_stats(Q, mat_conf);
		display_full_stats(Q, mat_conf);
	
		printf("Do you want vector b to be optimized  ");
		printf("(this is not recommended for Q > 4 ) ([y]/n)? ");
		choice = getchar();
	}
		
	if((choice == 'y') || (choice == 'Y') || (choice == 'a' && Q<=4 )) {
		/* if automatic choice: 
			 do not optimize b for Q > 4 (too large dimension for the grid search)
			 
			 before version 1.4: did not optimize b before reaching a ratio of 75 % 
 			 (but this caused training to stall with linear kernel on e.g. iris data set)
		*/
		
	  WW_optimize_b(gradient, &cache, model);
	  
	  if(cache.primal > *best_primal_upper_bound)
		cache.primal = *best_primal_upper_bound;
	  else
		*best_primal_upper_bound = cache.primal;

    	  ratio = cache.dual / cache.primal;	  
	  if(verbose) {
		  printf("\nDifference between the primal and the dual: %lf -> %lf", cache.primal, cache.dual);
		  printf("\n(ratio %5.2lf \%%)\n", 100.0 * ratio);
	  }  
	  if(bad_b)
	    printf("\nWith optimal values for the biases, the primal should be: %lf\n", 0.5*cache.alpha_H_alpha);
	  if(verbose) {
		  printf("\n*** Training performance\n");
		  display_stats(Q, mat_conf);
		  display_full_stats(Q, mat_conf);
		  Pause("");
	  }
	  
	  // Update training error with new b
  	  R_emp = WW_eval_training(gradient,mat_conf,model);
	  training_errors = model->nb_data;
	  for(i=1;i<=Q;i++)
		training_errors -= mat_conf[i][i];
	
	  model->training_error = (double)training_errors/(double)model->nb_data;

	}
	else if(choice != 'a')
		choice = getchar();

	if((model->nature_kernel == 1) && (model->dim_input == 2) && (Q == 3) && (max_alpha > 0.0) && verbose)
	  WW_compute_boundaries(model);

	// Log info
	if(fp != NULL)
		fprintf(fp, "%lf %lf\n ",cache.dual,cache.primal);
	

	// Free memory
	free(mat_conf[1]); free(mat_conf);
	free(margin_vect[1]); free(margin_vect);
	free(nb_margin_vect[1]); free(nb_margin_vect);

	return ratio;
	
}


/*
	output = WW_function( k, x, model)

	Compute the real-valued output of the model corresponding to the k-th category on the pattern x.
*/
double WW_function(long category, void *vector, const struct Model *model)
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
	label = WW_classify(x, model, real_outputs)
	
	Classify x and output the label. 
	  
	If real_outputs is not NULL, also compute the Q real-valued outputs
	 of the model and store them in (real_outputs[1]...real_outpust[Q]).
	 
	Note: this function is faster than calling Q times WW_function 		
*/
long WW_classify(void *vector, const struct Model *model, double *real_outputs)
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
		real_outputs[k] = model->b_SVM[k];		

	for(i=1; i<=nb_data; i++)
	  {  
	    partial = 0.0;
	    isSV = false;
	    for(k=1; k<=Q; k++) {
	      partial += model->alpha[i][k];
	      if(model->alpha[i][k] != 0.0)
	      	isSV = true;
	    }
	    if(isSV) {
		    kernel = ker(nature_kernel, model->X[i], vector, dim_input, kernel_par);
		    for (k=1; k<=Q; k++) {
		    	    if(model->y[i] == k)
			    	real_outputs[k] += partial * kernel;
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


void WW_optimize_b(double **gradient, struct Cache *cache, struct Model *model)
{
	const long Q = model->Q;

	long count, portion, class, Iteration, k;
	long max_count = 10; 	
	for(k=0;k<Q-2;k++)
		max_count *= 10;	// max_count = 10^(Q-1);
		
	if(max_count <= 0 || max_count > 1000000) {
		printf("Too large Q, b will not be optimized.\n"); 
		return;
	}
	
	double bstep = 0.1;
	
	double *grid_index = (double *) calloc(Q+1, sizeof(double));
	double *b_SVM_base = (double *) calloc(Q+1, sizeof(double));
	double *best_b_SVM = (double *) calloc(Q+1, sizeof(double));

	double R_emp;
	
	double R_emp_min = WW_eval_training(gradient, NULL, model);

	for(class=1; class<=Q; class++)
	  best_b_SVM[class] = model->b_SVM[class];


	grid_index[1] = 0.0;
	b_SVM_base[1] = 0.0;

	for(Iteration=1; Iteration <=10; Iteration++)
	  {
	  for(k=2; k<=Q; k++)
	    b_SVM_base[k] = model->b_SVM[k] - (5.0 * bstep);

	  for(count=0; count<max_count; count++)
	    {
	  
//	    printf("\nPosition in the grid: %6d\n", count); 

	    portion = count;
	    for(class=2; class<=Q; class++)
	      {
	      grid_index[class] = (double) (portion / (long) pow(10.0, (double) (Q-class)));
	      portion -= (long) (grid_index[class] * pow(10.0, (double) (Q-class)));
	      }

	    for(class=2; class<=Q; class++)
	      model->b_SVM[class] = b_SVM_base[class] + (grid_index[class] * bstep);

	    R_emp = WW_eval_training(gradient, NULL, model);
	    
	    if(R_emp < R_emp_min)
		  {
		  R_emp_min = R_emp;
		  for(class=1; class<=Q; class++)
		    best_b_SVM[class] = model->b_SVM[class];     
		  }
    	    }

	  for(class=1; class<=Q; class++)
	    model->b_SVM[class] = best_b_SVM[class];

	  R_emp = WW_eval_training(gradient, NULL, model);
	    
	  if(R_emp < R_emp_min)
		  {
		  R_emp_min = R_emp;
		  for(class=1; class<=Q; class++)
		    best_b_SVM[class] = model->b_SVM[class];     
		  }
  
	  bstep /= 2.0;
	  }

    	standardize_b(model->b_SVM,Q);

	if(cache->verbose) {	
		printf("\nNew components of the b vector:\n");

		printf("\n");
		for(class=1; class<=Q; class++)
		  printf("%11.6f\n", model->b_SVM[class]);
	}
	
	WW_compute_obj_primal(R_emp,cache,model);

	free(grid_index);
	free(b_SVM_base);
	free(best_b_SVM);
}

void WW_check_feasible_sol(const struct Model *model, char *alpha_file,int verbose)
{
	char command[taille];
	const long Q = model->Q;
	const long nb_data = model->nb_data;
	const double *C = model->C;
	double norm;
	int rc;
	
	long i,k,l;	
	double *constraints;

	for(i=1; i<=nb_data; i++)
	  for(k=1; k<=Q; k++)
	    if((model->alpha[i][k] < 0.0) || (model->alpha[i][k] > C[model->y[i]]))
	       {
	       printf("\nNo feasible solution: alpha[%ld][%ld] = %1.12lf  >  %lf\n\n",
		      i, k, model->alpha[i][k], C[model->y[i]]);
	       exit(0);
	       }
	     
	constraints = (double *)calloc(Q+1, sizeof(double));

	for(k=1; k<=Q; k++)
	  constraints[k] = 0.0;

	for(i=1; i<=nb_data; i++)
	  for(k=1; k<=Q; k++)
	     {
	     if(model->y[i] == k)
	       for(l=1; l<=Q; l++)
		 constraints[k] -= model->alpha[i][l];
	     else
	       constraints[k] += model->alpha[i][k];  
	     }
	
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

void WW_check_sup_vect(const struct Model *model)
{
	long i,k,l;
	const long Q = model->Q;
	const long nb_data = model->nb_data;

	long **sup_vect = matrix_l(nb_data, Q);
	long **nb_sup_vect = matrix_l(Q, Q);

	for(i=1; i<=nb_data; i++)
	  for(k=1; k<=Q; k++)
	    sup_vect[i][k] = 0;

	for(k=1; k<=Q; k++)
	  for(l=1; l<=Q; l++)
	    nb_sup_vect[k][l] = 0;

	for(i=1; i<=nb_data; i++)
	  for(k=1; k<=Q; k++)
	    if(model->alpha[i][k] > model->C[model->y[i]] * small_enough)
	      {
	      sup_vect[i][k] = 1;
	      nb_sup_vect[model->y[i]][k]++;
	      }

	printf("\nDistribution matrix of the support vectors\n");
	display_mat_l(nb_sup_vect, Q, Q, 6);
	
	free(sup_vect[1]); free(sup_vect);
	free(nb_sup_vect[1]); free(nb_sup_vect);
}

void WW_check_margin_vect(long **margin_vect, long **nb_margin_vect, const struct Model *model)

{
	long i,k,l,y_i;
	const long Q = model->Q;
	const long nb_data = model->nb_data;
	const double *C = model->C;
	
	int over=false;

	double *average = (double*)malloc(sizeof(double) * (Q+1));

	double **radius = matrix(Q, Q);
	
	const double radius_step = 1000.0;
	
	for(i=1; i<=nb_data; i++)
	  for(k=1; k<=Q; k++)
	    margin_vect[i][k] = 0;
	
	
	for(k=1; k<=Q; k++) {
	  average[k] = C[k] / 2.0;;

	  for(l=1; l<=k; l++) {
	    radius[k][l] = C[k] / radius_step;
	    radius[l][k] = C[k] / radius_step;
 	  }
 	}
 	
	while(over == false) {

	  for(k=1; k<=Q; k++)
	    for(l=1; l<=Q; l++)
	      nb_margin_vect[k][l] = 0;

	  for(i=1; i<=nb_data; i++)
	    {
	    y_i = model->y[i];
	    for(k=1; k<=Q; k++)
	      if(fabs(model->alpha[i][k] - average[model->y[i]]) <= radius[y_i][k])
		{
		margin_vect[i][k] = 1;
		nb_margin_vect[y_i][k]++;
		}
	    }

	  over = true;

	  for(k=1; k<=Q-1; k++)
	    for(l=k+1; l<=Q; l++)
	      if((nb_margin_vect[k][l] == 0) && (nb_margin_vect[l][k] == 0))
		{
		over = false;
		if((average[k] - radius[k][l]) > (average[k] / 5000.0))
		  radius[k][l] += average[k] / radius_step;
		else
		  radius[k][l] = (radius[k][l] + average[k]) / 2.0;
		radius[l][k] = radius[k][l];
		}

	}

	free(radius[1]); free(radius);
	free(average);
}


double WW_eval_training(double **gradient, long **mat_conf, const struct Model *model)
{
	long i,k,l,y_i;
	const long Q = model->Q;
	const long nb_data = model->nb_data;

	long ind_min = 0;
	double mini = HUGE_VAL, delta = 0.0;
	
	double R_emp = 0.0;

	if(mat_conf != NULL) {
		for(k=1; k<=Q; k++)
		  for(l=1; l<=Q; l++)
		    mat_conf[k][l] = 0;
	}
	
	for(i=1; i<=nb_data; i++)  {
	  mini = HUGE_VAL;
	  ind_min = 0;
	  y_i = model->y[i];

/*	 Do " for(k=1; k<=Q; k++)  if(k != y_i) ... " in 2 loops without if   */
	  for(k=1;k<y_i;k++)
	      {
	      delta = gradient[i][k] + model->b_SVM[y_i] - model->b_SVM[k];
	      if(delta < 0.0)
		 R_emp -= model->C[y_i] * delta;
	      if(delta < mini)
		{
		mini = delta;
		ind_min = k;
		}
	      }
  	  for(k=y_i+1;k<=Q;k++)
	      {
	      delta = gradient[i][k] + model->b_SVM[y_i] - model->b_SVM[k];
	      if(delta < 0.0)
		 R_emp -= model->C[y_i] * delta;
	      if(delta < mini)
		{
		mini = delta;
		ind_min = k;
		}
	      }
	  if(mat_conf != NULL) {
		  if(mini <= -1.0)
		    mat_conf[y_i][ind_min]++;
		  else
		    mat_conf[y_i][y_i]++;
	  }
	}

	return R_emp;
}

void WW_compute_W(struct Model *model)
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

void WW_estimate_b(double **gradient, long **margin_vect, long **nb_margin_vect, const struct Cache *cache, struct Model *model)
{

	long i,k,l;
	const long Q = model->Q;
	const long nb_data = model->nb_data;

	double **delta_b_SVM = matrix(Q, Q);


	for(k=1; k<=Q; k++)
	   model->b_SVM[k] = 0.0;
	  

	for(k=1; k<=Q; k++)
	  for(l=1; l<=Q; l++)
	    delta_b_SVM[k][l] = 0.0;

	for(i=1; i<=nb_data; i++)
	  for(k=1; k<=Q; k++)
	    if(margin_vect[i][k] == 1)
	      delta_b_SVM[model->y[i]][k] -= gradient[i][k];

	for(k=1; k<=Q; k++)
	  for(l=1; l<=Q; l++)
	    if(k != l)
	      if(nb_margin_vect[k][l] != 0)
		delta_b_SVM[k][l] /= (double)nb_margin_vect[k][l];


	for(k=2; k<=Q; k++)
	  if(nb_margin_vect[k][1] + nb_margin_vect[1][k] > 0)
	    {
	    model->b_SVM[k] = (nb_margin_vect[k][1] * delta_b_SVM[k][1] 
		         - nb_margin_vect[1][k] * delta_b_SVM[1][k]) / 
		          (double)(nb_margin_vect[k][1] + nb_margin_vect[1][k]);
	    }

    	standardize_b(model->b_SVM,Q);
	
	if(cache->verbose) {
		printf("\nComponents of the b vector:\n");
		printf("\n");
		for(k=1; k<=Q; k++)
		  printf("%11.6f\n", model->b_SVM[k]);
	}
	
	free(delta_b_SVM[1]); free(delta_b_SVM);
}

void WW_compute_obj_dual(struct Cache *cache, struct Model *model)

{
	long i,k;
	const long Q = model->Q;
	const long nb_data = model->nb_data;
	
	double alpha_H_alpha = 0;
	double L_form_dual = 0;
	
	for(i=1; i<=nb_data; i++)
	  for(k=1; k<=Q; k++)
	    alpha_H_alpha += cache->H_alpha[i][k] * model->alpha[i][k];

	for(i=1; i<=nb_data; i++)
	  for(k=1; k<=Q; k++)
	    L_form_dual += model->alpha[i][k];

	model->sum_all_alpha = L_form_dual;

	cache->alpha_H_alpha = alpha_H_alpha;
	cache->dual = -0.5*alpha_H_alpha + L_form_dual;
}

void WW_compute_obj_primal(double R_emp, struct Cache *cache, const struct Model *model)
{
 
	cache->primal = 0.5 * cache->alpha_H_alpha + R_emp;
	if(cache->primal < cache->dual)
	  {
	  printf("\nInconsistency in the values of the objective functions...\n primal = %lf + %lf = %lf  <  %lf (dual)\n", 0.5 * cache->alpha_H_alpha, R_emp , cache->primal, cache->dual);
	/*  exit(0); */
	  }
	
}

void WW_compute_boundaries(const struct Model *model)
{

	if(model->W == NULL) {
		printf("Cannot compute_boundaries(): no linear weights W in model.\n");
		return;
	}

	double slope, y_at_origin;

	slope = (model->W[1][1] - model->W[1][2]) / (model->W[2][2] - model->W[2][1]);
	y_at_origin = (model->b_SVM[1] -  model->b_SVM[2]) / (model->W[2][2] - model->W[2][1]);

	printf("\nSlope : %lf", slope);
	printf("\nOrdinate at the origin : %lf\n", y_at_origin);

	Pause("");

	slope = (model->W[1][1] - model->W[1][3]) / (model->W[2][3] - model->W[2][1]);
	y_at_origin = (model->b_SVM[1] -  model->b_SVM[3]) / (model->W[2][3] - model->W[2][1]);

	printf("\nSlope : %lf", slope);
	printf("\nOrdinate at the origin : %lf\n", y_at_origin);

	Pause("");

	slope = (model->W[1][2] - model->W[1][3]) / (model->W[2][3] - model->W[2][2]);
	y_at_origin = (model->b_SVM[2] -  model->b_SVM[3]) / (model->W[2][3] - model->W[2][2]);

	printf("\nSlope : %lf", slope);
	printf("\nOrdinate at the origin : %lf\n", y_at_origin);

	Pause("");

}

