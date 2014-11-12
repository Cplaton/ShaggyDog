/* Copyright 2008-2010 Yann Guermeur                                        */

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
/*  Name           : libtrainMSVM_2.c                                       */
/*  Version        : 2.0                                                    */
/*  Creation       : 06/20/08                                               */
/*  Last update    : 01/20/11                                               */
/*  Subject        : Training algorithm of the M-SVM^2                      */
/*  Algo.          : Rosen's algorithm + decomposition method               */
/*  Author         : Fabien Lauer and Yann Guermeur Yann.Guermeur@loria.fr  */
/*--------------------------------------------------------------------------*/

#include "libtrainMSVM_2.h"


/*
	MSVM2 training function (Rosen algorithm)
	
	Solves the dual problem wrt alpha:
	
	min  	1/2 alpha' H_tilde alpha  - (1/Q-1) sum (alpha)
	
	s.t.
	
	alpha_ik >= 0,    			for all i,k, such that 1 <= i <= m, 1 <= k != y_i <= Q
	sum_i (alpha_ik - average_alpha_k) = 0, for all k, 1 <= k <= Q
		
	where
		H_tilde_ik,jl = (delta_k,l - 1/Q) ( k(x_i,x_j) + (1/2C)delta_i,j  )
		
*/
long MSVM_2_train(struct Model *model, struct Data *training_set, long chunk_size, const double accuracy, int cache_memory, const int nprocs, char *alpha0_file, char *model_tmp_file, char *log_file)
{
	long return_status = -1;
	FILE *fp;	
	int t;
	pthread_t *threads = (pthread_t *)malloc(sizeof(pthread_t) * nprocs); 	// threads id
	void *status; 			// for pthread_join
	int rc; 			// return code of pthread functions
	int feasible;
	pthread_t thread_monitor;
	
	enum AlphaInitType alphaiszero;	
	
	if(training_set != NULL) {
		// Initialize model for this particular training set		
		model_load_data(model, training_set);				
	}
	// Check if a kernel_par has been set
	if(model->nature_kernel != LINEAR && model->kernel_par == NULL)
		set_default_kernel_par(model);
		
	// Log file
	if (log_file != NULL) {
		printf("\nLog training info in %s ...\n", log_file);
		fp = fopen(log_file,"a");
	}
	else
		fp = NULL;
	
	// Initialize alpha and b
	if(training_set != NULL)  // otherwise resume training
		init_alpha_b(model, alpha0_file);  // use init_alpha_b(model, NULL); to initialize all alpha to 0

	feasible = MSVM_2_check_feasible_sol_init(model);
	if(feasible == true)
		printf("\nVector alpha is a feasible solution...\n");
	else {
		printf("\n Initial alpha_0 is not a feasible solution.\n");		
		return -1;
	}
	
	
	if(training_set == NULL || alpha0_file != NULL)
		alphaiszero = ALPHA_NOT_ZERO;
	else
		alphaiszero = ALPHA_IS_ZERO;
	
		
	EVAL = 0;	// triggered by signal handler to call eval
	STOP = 0;	// triggered by user to stop training

	// Prepare monitoring of model
	struct MonitorData monitor_data;
	monitor_data.model_tmp_file = model_tmp_file;
	monitor_data.period = MONITOR_PERIOD;
	monitor_data.model = model;

	pthread_create(&thread_monitor, NULL, MSVM_monitor_model_thread, (void *) &monitor_data);

	
	/* Allocate memory for shared ressources  */
	double **gradient = matrix(model->nb_data, model->Q);		
	double **H_alpha = matrix(model->nb_data, model->Q);
	double **H_tilde_alpha = matrix(model->nb_data, model->Q);
	double best_primal_upper_bound = HUGE_VAL;
	int *activeset = (int *)calloc(model->nb_data+1, sizeof(int));	
	// including the kernel cache:
	struct KernelCache kc;
	unsigned long cache_size = (unsigned long)cache_memory * MBYTES;
	kernel_initialize_cache(cache_size, &kc, model);
	
	printf("Initialization... ");fflush(stdout);
	
	// Initialize gradient
	MSVM_2_init_gradient(alphaiszero, gradient, H_alpha, H_tilde_alpha, model);
	
	printf("Done.\n");
	
	// Initialize display
	if(accuracy>0)
		print_training_info(0,model); 
	else {
		printf("Training...");fflush(stdout);
	}

	model->iter = 1;  	// number of iterations (over all threads)
	
	// Prepare data for computing threads
	long nb_SV = 0;
	struct ThreadData thread_data;
	thread_data.nprocs = nprocs;		
	thread_data.model = model;
	thread_data.kernelcache = &kc;	
	thread_data.chunk_size = chunk_size;
	thread_data.accuracy = accuracy;
	thread_data.gradient = gradient;
	thread_data.H_alpha = H_alpha;
	thread_data.H_tilde_alpha = H_tilde_alpha;
	thread_data.best_primal_upper_bound = &best_primal_upper_bound;
	thread_data.activeset = activeset;	
	thread_data.nb_SV = &nb_SV;	
	thread_data.logfile_ptr = fp;
	
	// Launch computing threads
	for(t=0;t<nprocs;t++) {
	
		pthread_mutex_lock(&thread_data_mutex); // Wait for thread_data to be read by previous thread
 		
		thread_data.thread_id = t;
		rc = pthread_create(&threads[t], NULL, MSVM_2_train_thread, (void *) &thread_data);	
	}
	
	// Wait for threads to terminate
	for(t=0;t<nprocs;t++) {
		rc = pthread_join(threads[t],&status);

		// Check if optimum has been reached by this thread		
		if((void *)status == 0)
			return_status=0;
	}
	
	// Cancel monitor thread
	rc = pthread_cancel(thread_monitor);
	rc = pthread_join(thread_monitor,&status);
	 	 	
	if(log_file != NULL)
		fclose(fp);

	free(gradient[1]);free(gradient);
	free(H_alpha[1]);free(H_alpha);
	free(H_tilde_alpha[1]);free(H_tilde_alpha);
	free(activeset);
	kernel_free_cache(&kc);
	free(threads);
	
	if(model->iter == MSVM_TRAIN_MAXIT)
		return_status = 1; // max iterations.
	else if(return_status != 0)
		return_status = model->iter; // interrupted by user OR accuracy reached (XXX).

	return return_status;	
}

void *MSVM_2_train_thread(void *th_data) {

	// Recover data
	struct ThreadData *data =  (struct ThreadData *)th_data;
	const int thread_id = data->thread_id;
	const int nprocs = data->nprocs;	
	struct Model *model = data->model;
	struct KernelCache *kernelcache = data->kernelcache;
	long chunk_size = data->chunk_size;
	const double accuracy = data->accuracy;
	double **gradient = data->gradient;	
	double **H_alpha = data->H_alpha;
	double **H_tilde_alpha = data->H_tilde_alpha;
	double *best_primal_upper_bound = data->best_primal_upper_bound;
	int *activeset = data->activeset;
	long *nb_SV = data->nb_SV;		
	FILE *fp = data->logfile_ptr;	
	
	pthread_mutex_unlock(&thread_data_mutex);	// Release thread_data for next thread 
	
	// Local variables
	long long return_status = -1;
	int do_eval, optimum, descent, feasible;
	char yesno;
	
	// Prepare the cache
	struct TrainingCache cache;
	cache.chunk_size = chunk_size;
	MSVM_2_alloc_memory(&cache, model->Q, model->nb_data, chunk_size);
	cache.activeset = activeset;	
	cache.kc = kernelcache;
	
	double **vector_u = matrix(cache.nb_constraints, 1);
	double **vector_y_bar = matrix(cache.nb_variables, 1);
	double **vector_gradient = matrix(cache.nb_variables, 1);
	double theta_max, theta_opt;
	
	if(accuracy == 0)
		do_eval = 0;
	else 
		do_eval = 1;

	/*
		Prepare parallel gradient computations:
		- the gradient vector is split into NUMTHREADS_GRAD parts (along i)
		- each part is updated by a different thread
	*/
	// max number of threads for gradient updates is nprocs
	pthread_t *grad_threads = (pthread_t *)malloc(sizeof(pthread_t) * nprocs); 

	// start with 1 thread (main load on kernel evaluations)
	int numthreads_grad = 1;		

	void *status; 			
	int rc; 		
	long k;	
	struct ThreadGradient_data *grad_data = (struct ThreadGradient_data *)malloc(sizeof(struct ThreadGradient_data) * nprocs);


	// Disable parallel gradient computation for small data sets
	int parallel_gradient_update = 1;
	if(model->nb_data < 5000 || nprocs == 1)
		parallel_gradient_update = 0;

	if(parallel_gradient_update) {
		for(k=0;k<nprocs;k++) {
			grad_data[k].gradient = gradient;
			grad_data[k].H_alpha = H_alpha;
			grad_data[k].H_tilde_alpha = H_tilde_alpha;			
			grad_data[k].cache = &cache;
			grad_data[k].model = model;
		}		
		grad_data[0].start_i = 1;
		grad_data[0].end_i = model->nb_data / numthreads_grad;	
		for(k=1;k<numthreads_grad-1;k++) {	
			grad_data[k].start_i = grad_data[k-1].end_i + 1;
			grad_data[k].end_i = grad_data[k].start_i + model->nb_data / numthreads_grad -1;
		}
		if(numthreads_grad>1) {
			grad_data[numthreads_grad-1].start_i = grad_data[numthreads_grad-2].end_i + 1;
			grad_data[numthreads_grad-1].end_i = model->nb_data;
		}	
	}
		
#ifdef _WIN32
	// Init POOL
	TP_WORK ** work;
	
	if(parallel_gradient_update) {
		
		work = malloc(sizeof(TP_WORK *) * nprocs);
		for(k=0;k<nprocs;k++)
			work[k] = CreateThreadpoolWork(MSVM2_update_gradient_thread2, (void *) &grad_data[k], NULL);
	}
#endif
	// Switch to nprocs/4 threads for gradient update when 25% of the kernel matrix is cached
	int percentage_step = 1;
	long percentage = model->nb_data / 4;
	int next_numthreads_grad = nprocs/4;
	if(next_numthreads_grad == 0) 
		next_numthreads_grad = 1;
	
	
	// Main loop
	int thread_stop = 0;
	do {	
	  if((TRAIN_SMALL_STEP < TRAIN_STEP) && (model->iter%TRAIN_SMALL_STEP) == 0) {
	    	printf(".");
		fflush(stdout);
  	  }
	   		   	
  	  // Select a random chunk of data to optimize 
	  select_random_chunk(&cache,model);
			
	  // Compute the kernel submatrix for this chunk
	  compute_K(&cache,model);			

	  // Enter Critical Section (using and modifying the model)
	  pthread_mutex_lock(&(model->mutex)); 
  
	  MSVM_2_compute_gradient_chunk(vector_gradient,gradient, &cache,model);
	  MSVM_2_compute_A0(&cache,model);	  
	  MSVM_2_compute_vector_u(vector_u,vector_gradient,&cache,model);
	  optimum = MSVM_2_check_vector_u(vector_u,vector_gradient,&cache,model);

	  if(optimum == 0 )	// u is correct and optimum not reached yet
	    {
	    MSVM_2_compute_vector_y_bar(vector_y_bar, vector_u, vector_gradient, &cache, model);
	    descent = MSVM_2_check_vector_y_bar(vector_y_bar,vector_u,vector_gradient, &cache,model);
	 
	    if(descent == 0)
	      while((descent == 0) && (optimum == 0))
		{
		MSVM_2_simplify_A0(vector_u, &cache,model);
		MSVM_2_compute_vector_u(vector_u,vector_gradient,&cache,model);
	  	optimum = MSVM_2_check_vector_u(vector_u,vector_gradient,&cache,model);

		if((optimum == 0))
		  {
		  MSVM_2_compute_vector_y_bar(vector_y_bar,vector_u,vector_gradient, &cache,model);
	    	  descent = MSVM_2_check_vector_y_bar(vector_y_bar,vector_u,vector_gradient, &cache,model);
		  }
		}

	    if((optimum == 0) && (descent > 0))
	      {
	      theta_max = MSVM_2_compute_theta_max(&cache,model);
	      theta_opt = MSVM_2_compute_theta_opt(theta_max,&cache,model);
	      *nb_SV += MSVM_2_compute_new_alpha(theta_opt,&cache,model);
	     	      	     
	      if(parallel_gradient_update) {
			
			// Update gradient in parallel 
   			for(k=0;k<numthreads_grad;k++) {
			#ifdef _WIN32
				SubmitThreadpoolWork(work[k]);
			#else
				rc = pthread_create(&grad_threads[k], NULL, MSVM2_update_gradient_thread, (void *) &grad_data[k]);	
			#endif	
			}			
			// Wait for gradient computations to terminate
			for(k=0;k<numthreads_grad;k++) {
			#ifdef _WIN32
				WaitForThreadpoolWorkCallbacks(work[k], FALSE);
			#else
				rc = pthread_join(grad_threads[k],&status);					
			#endif
			}
		}
	       else {				
			// old-style non-threaded gradient update (for small data sets)
			MSVM_2_update_gradient(gradient,H_alpha, H_tilde_alpha, &cache, model);
	       }
	      
	      
	      feasible = MSVM_2_check_feasible_sol_train(model, &cache);
	      if(feasible == false)
		MSVM_2_study_unfeasible_sol(descent, vector_u, vector_y_bar, &cache, model);
	      }

	    }
	
	  if((do_eval && (model->iter%TRAIN_STEP) == 0) || EVAL || STOP || (do_eval && model->ratio >= accuracy) ) 
	    {    	   	
		if(fp != NULL)
			fprintf(fp,"%ld ",model->iter);
	
		if(EVAL)
			printf("\n\n*** Evaluating the model at iteration %ld...\n",model->iter);
			 
		// Evaluate how far we are in the optimization
		// (prints more info if interrutped by user)	
		model->ratio = MSVM_eval(best_primal_upper_bound, gradient, H_alpha, H_tilde_alpha, model, EVAL, fp);
		
		print_training_info(*nb_SV, model);
						
		if(EVAL) // if interrupted by user (otherwise let the ratio decide if we go on training)
		  {
			printf("\n *** Do you want to continue training ([y]/n)? ");
			yesno = getchar();
			if(yesno=='n') {
				STOP = 1;				
			}
			EVAL = 0; // reset interruption trigger
		  }		
	    }
	    
	  // Release kernel submatrix in cache
	  release_K(&cache);
	  
	  // Check if a sufficient % of the kernel matrix is cached
	  if( parallel_gradient_update && cache.kc->max_idx >= percentage ) {	
		// and switch thread to compute gradient upates instead of kernel rows if it is		
		thread_stop = switch_thread(nprocs, &numthreads_grad, &next_numthreads_grad, &percentage,  &percentage_step, grad_data, thread_id, model->nb_data);				
		// (threads are actually stopped to leave the CPUs
		//  to other threads that will compute gradient updates)
	  }				

	  model->iter++;

	  // Release mutex: End of critical section (see below)
	  pthread_mutex_unlock(&(model->mutex));			
  
	} while(model->iter <= MSVM_TRAIN_MAXIT && (!do_eval || model->ratio < accuracy) && !STOP && !thread_stop); 
	
#ifdef _WIN32
	if(parallel_gradient_update){
		for(k=0;k<numthreads_grad;k++)
			CloseThreadpoolWork(work[k]);
	}	
#endif

   	// Compute return_status
	if(do_eval && model->ratio >= accuracy)
		return_status = 0; // optimum reached. 
		
	// Free memory
	MSVM_2_free_memory(&cache);
	free(vector_u[1]);free(vector_u);
	free(vector_y_bar[1]); free(vector_y_bar);
	free(vector_gradient[1]); free(vector_gradient);
	free(grad_threads);
	free(grad_data);
	
	pthread_exit((void*)return_status);
}


/* 
	Allocate the memory for the cache 
*/
void MSVM_2_alloc_memory(struct TrainingCache *cache, const long Q, const long nb_data, const long chunk_size) 
{

	long nb_variables = chunk_size * (Q-1);
	long nb_constraints = nb_variables + (Q-1);
	
	cache->in_chunk = (long *) calloc(nb_data+1, sizeof(long));	
	cache->out_of_chunk = (long *) malloc((nb_data+1) * sizeof(long));
	cache->table_chunk = (long *) calloc(chunk_size+1, sizeof(long));

	cache->K = (double **)malloc(sizeof(double *) * (chunk_size+1)); 
	cache->A0 = matrix(nb_constraints, nb_variables);
	cache->A0T = matrix(nb_variables, nb_constraints);
	cache->A0A0T = matrix(nb_constraints, nb_constraints);
	cache->A0A0T_ref = matrix(nb_constraints, nb_constraints);
	cache->A0A0T_1 = matrix(nb_constraints, nb_constraints);
	cache->vector_z_bar = matrix(nb_variables, 1);
	cache->H_y_bar = matrix(chunk_size, Q);
	cache->matrix_y_bar = matrix(chunk_size, Q);
	
	cache->alpha_update = matrix(chunk_size, Q);

	// Computational templates
	cache->eigenvalues = (double *) calloc(nb_constraints+1, sizeof(double));
	cache->working_matrix1 = matrix(nb_constraints, nb_constraints);
	cache->working_matrix2 = matrix(nb_constraints, nb_constraints);

	// For debug purposes (used in study_unfeasible)
	cache->constraints = (double *) calloc(Q+1, sizeof(double));


	cache->nb_constraints = nb_constraints;
	cache->nb_variables = nb_variables;

}

/* 
	Free the cache memory 
*/
void MSVM_2_free_memory(struct TrainingCache *cache) {
	free(cache->table_chunk);
	free(cache->in_chunk);	
	free(cache->out_of_chunk);
	free(cache->K);
	free(cache->A0[1]);free(cache->A0);
	free(cache->A0T[1]);free(cache->A0T);
	free(cache->A0A0T[1]);free(cache->A0A0T);
	free(cache->A0A0T_1[1]);free(cache->A0A0T_1);
	free(cache->A0A0T_ref[1]);free(cache->A0A0T_ref);
	free(cache->vector_z_bar[1]);free(cache->vector_z_bar);
	free(cache->H_y_bar[1]);free(cache->H_y_bar);
	free(cache->matrix_y_bar[1]);free(cache->matrix_y_bar);
	free(cache->eigenvalues);
	free(cache->working_matrix1[1]);free(cache->working_matrix1);
	free(cache->working_matrix2[1]);free(cache->working_matrix2);
	free(cache->constraints);
	
	free(cache->alpha_update[1]);free(cache->alpha_update);
}

/*
	MSVM_2_compute_gradient_chunk(vector_gradient,cache,model)
	
	Compute the components of the gradient vector corresponding to the chunk 
*/
void MSVM_2_compute_gradient_chunk(double **vector_gradient, double **gradient, struct TrainingCache *cache, const struct Model *model)
{
	long i,k,y_i,ind_pattern;
	const long Q = model->Q;
	const long chunk_size = cache->chunk_size;
	
	long row = 0;

	for(i=1; i<=chunk_size; i++) {
	  ind_pattern = cache->table_chunk[i];
	  y_i = model->y[ind_pattern];
	  for(k=1; k<=Q; k++)
	    {
	    if(k != y_i)
	      {
	      row++;
	      vector_gradient[row][1] = gradient[ind_pattern][k];		      
	      }
	    }
	}
	
}

/*
	MSVM_2_init_gradient({ALPHA_IS_ZERO,ALPHA_NOT_ZERO}, gradient, cache, model)
*/
void MSVM_2_init_gradient(const enum AlphaInitType alpha_init_type, double **gradient, double **H_alpha, double **H_tilde_alpha, const struct Model *model)
{
	long i,j,k,l,y_i;
	const long nb_data = model->nb_data;
	const long Q = model->Q;
	const double Qd = (double)Q;
	const double Qinv = 1. / (Qd-1.0);
	const enum Kernel_type nature_kernel = model->nature_kernel;
	const double *kernel_par = model->kernel_par;
	long dim_input = model->dim_input;
	
	double partial, H_alpha_ik, H_tilde_alpha_ik, partial2;

	if(alpha_init_type == ALPHA_IS_ZERO) {	// Fast initialization
		for(i=1; i<=nb_data; i++) {		  
		  for(k=1; k<=Q; k++)
		    {
		    gradient[i][k] = -Qinv;
		    H_alpha[i][k] = 0.0;
		    H_tilde_alpha[i][k] = 0.0;		    
		    }
		    gradient[i][model->y[i]] = 0.0;
		 }		   
	}
	else {  
		// Do a proper initialization based on the values of alpha
		for(i=1; i<=nb_data; i++) {
		  y_i =  model->y[i];
		  
		  for(k=1; k<=Q; k++) {
		    if(k != y_i) {
	       	      H_tilde_alpha_ik = 0.0;	      
	       	      H_alpha_ik = 0.0;
		      for(j=1; j<=nb_data; j++)
			{
			partial = 0.0;
			for(l=1; l<=Q; l++)
			    	partial -= model->alpha[j][l];

			partial /= Qd;
			partial += model->alpha[j][k];
		
			if(partial != 0.0) {
				partial2 = partial * ker(nature_kernel, model->X[i], model->X[j], dim_input,kernel_par);
				H_alpha_ik += partial2; 
				H_tilde_alpha_ik += partial2;
				
				if(j==i) 				
					H_tilde_alpha_ik += partial / (2*model->C[y_i]);
			   }
			}
		      
	 	      gradient[i][k] = H_tilde_alpha_ik - Qinv;
	 	      H_alpha[i][k] = H_alpha_ik;
	 	      H_tilde_alpha[i][k] = H_tilde_alpha_ik;
		    }
		    else {
		       gradient[i][k] = 0.0;
		       H_alpha[i][k] = 0.0;
		       H_tilde_alpha[i][k] = 0.0;		       
		    }
		  }
		}
	}
}

/*
	Thread for updates of the gradient in parallel
	(spawn by main computing thread MSVM2_train_thread)	
*/
void *MSVM2_update_gradient_thread(void *th_data)
{
	// Recover thread data
	struct ThreadGradient_data *data = (struct ThreadGradient_data *) th_data;
	double **gradient = data->gradient;	
	double **H_alpha = data->H_alpha;	
	double **H_tilde_alpha = data->H_tilde_alpha;		
	struct TrainingCache *cache = data->cache;
	const struct Model *model = data->model;
	const long start_i = data->start_i;
	const long end_i = data->end_i;	

	// Local variables	
	long i,j,k,l,y_i,ind_pattern;
	const long Q = model->Q;
	const double Qd = (double)Q;
	const long chunk_size = cache->chunk_size;
	
	double partial, H_tilde_alpha_update_ik, H_alpha_update_ik;

	for(i=start_i; i<=end_i; i++) {
	  y_i =  model->y[i];
	  
	  for(k=1; k<=Q; k++) {
	    if(k != y_i) {
       	      H_tilde_alpha_update_ik = 0.0;	      
       	      H_alpha_update_ik = 0.0;
	      for(j=1; j<=chunk_size; j++)
		{
		ind_pattern = cache->table_chunk[j];
		partial = 0.0;
		for(l=1; l<=Q; l++)
		    partial -= cache->alpha_update[j][l];

		partial /= Qd;
		partial += cache->alpha_update[j][k];
		
		if(partial != 0.0) {
			H_tilde_alpha_update_ik += partial * cache->K[j][i];
			H_alpha_update_ik += partial * cache->K[j][i];
			
			if(ind_pattern==i)
				H_alpha_update_ik -= partial / (2*model->C[y_i]);
			}
	        }
 	      gradient[i][k] += H_tilde_alpha_update_ik;
 	      H_alpha[i][k] += H_alpha_update_ik;
 	      H_tilde_alpha[i][k] += H_tilde_alpha_update_ik;
	    }
	    
	  }
	}

	 pthread_exit(EXIT_SUCCESS);
}

#ifdef _WIN32
void MSVM2_update_gradient_thread2(PTP_CALLBACK_INSTANCE instance, void *th_data)
{
	// Recover thread data
	struct ThreadGradient_data *data = (struct ThreadGradient_data *) th_data;
	double **gradient = data->gradient;	
	double **H_alpha = data->H_alpha;	
	double **H_tilde_alpha = data->H_tilde_alpha;		
	struct TrainingCache *cache = data->cache;
	const struct Model *model = data->model;
	const long start_i = data->start_i;
	const long end_i = data->end_i;	

	// Local variables	
	long i,j,k,l,y_i,ind_pattern;
	const long Q = model->Q;
	const double Qd = (double)Q;
	const long chunk_size = cache->chunk_size;
	
	double partial, H_tilde_alpha_update_ik, H_alpha_update_ik;

	for(i=start_i; i<=end_i; i++) {
	  y_i =  model->y[i];
	  
	  for(k=1; k<=Q; k++) {
	    if(k != y_i) {
       	      H_tilde_alpha_update_ik = 0.0;	      
       	      H_alpha_update_ik = 0.0;
	      for(j=1; j<=chunk_size; j++)
		{
		ind_pattern = cache->table_chunk[j];
		partial = 0.0;
		for(l=1; l<=Q; l++)
		    partial -= cache->alpha_update[j][l];

		partial /= Qd;
		partial += cache->alpha_update[j][k];
		
		if(partial != 0.0) {
			H_tilde_alpha_update_ik += partial * cache->K[j][i];
			H_alpha_update_ik += partial * cache->K[j][i];
			
			if(ind_pattern==i)
				H_alpha_update_ik -= partial / (2*model->C[y_i]);
			}
	        }
 	      gradient[i][k] += H_tilde_alpha_update_ik;
 	      H_alpha[i][k] += H_alpha_update_ik;
 	      H_tilde_alpha[i][k] += H_tilde_alpha_update_ik;
	    }
	    
	  }
	}

	// pthread_exit(EXIT_SUCCESS);
}
#endif
/*
	Old-style non-threaded gradient update
	
*/
void MSVM_2_update_gradient(double **gradient, double **H_alpha,double **H_tilde_alpha, struct TrainingCache *cache, const struct Model *model)
{
	long i,j,k,l,y_i,ind_pattern;
	const long nb_data = model->nb_data;
	const long Q = model->Q;
	const double Qd = (double)Q;
	const long chunk_size = cache->chunk_size;
	
	double partial, H_tilde_alpha_update_ik, H_alpha_update_ik;

	for(i=1; i<=nb_data; i++) {
	  y_i =  model->y[i];
	  
	  for(k=1; k<=Q; k++) {
	    if(k != y_i) {
       	      H_tilde_alpha_update_ik = 0.0;	      
       	      H_alpha_update_ik = 0.0;
	      for(j=1; j<=chunk_size; j++)
		{
		ind_pattern = cache->table_chunk[j];
		partial = 0.0;
		for(l=1; l<=Q; l++)
		    partial -= cache->alpha_update[j][l];

		partial /= Qd;
		partial += cache->alpha_update[j][k];
		
		if(partial != 0.0) {
			H_tilde_alpha_update_ik += partial * cache->K[j][i];
			H_alpha_update_ik += partial * cache->K[j][i];
			
			if(ind_pattern==i)
				H_alpha_update_ik -= partial / (2*model->C[y_i]);
			}
		}
		
 	      gradient[i][k] += H_tilde_alpha_update_ik;
 	      H_alpha[i][k] += H_alpha_update_ik;
 	      H_tilde_alpha[i][k] += H_tilde_alpha_update_ik;
	    }
	    
	  }
	}
}


/* Computation of the matrices A0 and A0T */
void MSVM_2_compute_A0(struct TrainingCache *cache, const struct Model *model)
{
	long i,k,l,ind_pattern,y_i;
	const long Q = model->Q;
	double Qd = (double)Q;
	const long chunk_size = cache->chunk_size;
	long nb_constraints = cache->nb_constraints;
	long nb_variables = cache->nb_variables;
	
	long row=0;
	long column=0;

	for(k=1; k<=nb_constraints; k++)
	  for(l=1; l<=nb_variables; l++)
	    cache->A0[k][l] = 0.0;

	for(i=1; i<=chunk_size; i++)
	  {
	  ind_pattern = cache->table_chunk[i];
	  y_i = model->y[ind_pattern];
	  for(k=1; k<=Q; k++)
	    {
	    if(k != y_i)
	      {
	      column++;
	      if(model->alpha[ind_pattern][k] == 0.0)
		{
		row++; 
		cache->A0[row][column] = -1.0;
		}
	      }
	    }
	  }

	column=0;

	for(i=1; i<=chunk_size; i++)
	  {
	  ind_pattern = cache->table_chunk[i];
	  y_i = model->y[ind_pattern];

	  for(l=1; l<=Q; l++)
	    if(l != y_i)  
	      {
	      column++;
	      for(k=1; k<Q; k++)
		cache->A0[row+k][column] = (l != k) ? 1.0 : 1.0-Qd;
	      }

	  }

	cache->nb_sat = row+Q-1;
	trans_mat(cache->A0, cache->A0T, cache->nb_sat, nb_variables);

}

/* Computation of the vector u satisfying A0A0T u = - A0 grad J(alpha)*/
void MSVM_2_compute_vector_u(double **vector_u, double **vector_gradient, struct TrainingCache *cache, const struct Model *model)
{
	long nb_variables = cache->nb_variables;
	long nb_sat = cache->nb_sat;
	
	mult_mat(cache->A0, cache->A0T, cache->A0A0T, nb_sat, nb_variables, nb_sat);

	MSVM_2_compute_pseudo_inverse(cache);

	mult_mat(cache->A0, vector_gradient, cache->working_matrix1, nb_sat, nb_variables, 1);
	mult_mat(cache->A0A0T_1, cache->working_matrix1, cache->working_matrix2, nb_sat, nb_sat, 1);
	scal_mat(-1.0, cache->working_matrix2, vector_u, nb_sat, 1);

}

/* 
	optimum = MSVM_2_check_vector_u(u, gradient, cache, model)
	
	Check the equation defining vector u and returns 
		 1  if optimal conditions are satisfied
		 0  if u is correct and optimal conditions not reached yet
		-1  if u is not correct (problem in pseudo-inverse)
*/
int MSVM_2_check_vector_u(double **vector_u, double **vector_gradient, struct TrainingCache *cache, const struct Model *model)
{
	long k;
	long nb_sat = cache->nb_sat;
	long nb_variables = cache->nb_variables;
	const long Q = model->Q;	

	int optimum = 1;
	
	mult_mat(cache->A0A0T_ref, vector_u, cache->working_matrix1, nb_sat, nb_sat, 1);
	mult_mat(cache->A0, vector_gradient, cache->working_matrix2, nb_sat, nb_variables, 1);

	for(k=1; k<=nb_sat; k++)
	  cache->working_matrix2[k][1] *= -1.0;

	if(compare(cache->working_matrix1, cache->working_matrix2, nb_sat, 1, very_small) == false)
	  {
	  return -1;
	  }

	k=1;

	while((k<=nb_sat-Q+1) && (optimum == 1))
	  {
	  if(vector_u[k][1] < 0.0)
	    optimum=0;
	  else
	    k++;
	  }

	return optimum;
}

/* 
	MSVM_2_compute_vector_y_bar(vector_y_bar, vector_u, vector_gradient, cache, model)
	
	Compute the new direction of descent 
*/
void MSVM_2_compute_vector_y_bar(double **vector_y_bar, double **vector_u, double **vector_gradient, struct TrainingCache *cache, const struct Model *model)

{
	long i,k,y_i;	
	const long Q = model->Q;
	const long chunk_size = cache->chunk_size;
	long nb_sat = cache->nb_sat;
	long nb_variables = cache->nb_variables;
	
	long row = 0;
	
	mult_mat(cache->A0T, vector_u, cache->vector_z_bar, nb_variables, nb_sat, 1);

	add_mat(vector_gradient, cache->vector_z_bar, cache->working_matrix1, nb_variables, 1);
	scal_mat(-1.0, cache->working_matrix1, vector_y_bar, nb_variables, 1);

	for(k=1; k<=nb_variables; k++)
	  if(fabs(vector_y_bar[k][1]) < 0.01 * very_small)
	    vector_y_bar[k][1] = 0.0;


	for(i=1; i<=chunk_size; i++) {
	  y_i = model->y[cache->table_chunk[i]];
	  for(k=1; k<=Q; k++)
	    {
	    if(k != y_i)
	      {
	      row++;
	      cache->matrix_y_bar[i][k] = vector_y_bar[row][1];
	      }
	    else
	      cache->matrix_y_bar[i][k] = 0.0;
	    }

	}
}

/* 
	descent = MSVM_2_check_vector_y_bar(...)
		
		=  1 if non-zero norm feasible direction
		=  0 if zero-norm 
		= -1 if non-zero norm infeasible direction

	Check the feasibility of the new direction of descent 
*/
int MSVM_2_check_vector_y_bar(double **vector_y_bar, double **vector_u, double **vector_gradient, struct TrainingCache *cache, const struct Model *model)

{
	long i,k,row, y_i;
	const long Q = model->Q;
	const long chunk_size = cache->chunk_size;
	long nb_sat = cache->nb_sat;
	long nb_variables = cache->nb_variables;
	
	int nullity, descent = 0;
	
	double norm, norm_y_bar=0.0;

	for(k=1; k<=nb_variables; k++)
	  norm_y_bar += vector_y_bar[k][1] * vector_y_bar[k][1];

	norm_y_bar = sqrt(norm_y_bar);
	cache->initial_norm_y_bar = norm_y_bar;

	if(norm_y_bar <= very_small)
	  nullity=true;
	else
	  nullity=false;

	if(nullity == false)
	  {
	  row=0;

	  for(i=1; i<=chunk_size; i++) {
	    y_i = model->y[cache->table_chunk[i]];
	    for(k=1; k<=Q; k++)
	      {
	      if(k != y_i)
		{
		row++;
		vector_y_bar[row][1] /= norm_y_bar;
		cache->matrix_y_bar[i][k] = vector_y_bar[row][1];
		}
	      else
		cache->matrix_y_bar[i][k] = 0.0;
	      }
	  }
	  
	  descent=1;

	  mult_mat(cache->A0, vector_y_bar, cache->working_matrix1, nb_sat, nb_variables, 1);
	  norm=0.0;

	  for(k=1; k<=nb_sat; k++)
	    norm += cache->working_matrix1[k][1] * cache->working_matrix1[k][1];

	  norm = sqrt(norm);

	  if(norm > sqrt(nb_sat) * very_small)
	    {
	/*
	printf("\nbar{y} does not belong to the null space of A0: ||A0 bar{y}||= %e\n",
	       norm);
	*/
	    descent=-1;
	    }

	  cache->gradientT_y_bar=0.0;

	  for(k=1; k<=nb_variables; k++)
	    cache->gradientT_y_bar += vector_y_bar[k][1] * vector_gradient[k][1];

	  if(cache->gradientT_y_bar > 0.0)
	    {
	    if(cache->gradientT_y_bar > very_small)
	      {
	/*
	  printf("\ngradient^T bar{y} = %e: bar{y} is not a direction of descent...\n",
		 cache->gradientT_y_bar);
	*/
	      }
	    descent=-1;
	    }
	    
	  }

	return descent;
}

/* Relax one of the constraints to be satisfied by the direction of descent */
void MSVM_2_simplify_A0(double **vector_u, struct TrainingCache *cache, const struct Model *model)
{
	long k,l;
	const long Q = model->Q;
	long nb_sat = cache->nb_sat;
	long nb_variables = cache->nb_variables;
	
	double smallest_u = 0.0;
	long index_smallest_u = 1;

	for(k=1; k<=nb_sat-Q+1; k++)
	  if(vector_u[k][1] < smallest_u)
	    {
	    smallest_u = vector_u[k][1];
	    index_smallest_u = k;
	    }

	nb_sat--;

	for(k=index_smallest_u; k<=nb_sat; k++)
	  for(l=1; l<=nb_variables; l++)
	    cache->A0[k][l] = cache->A0[k+1][l];

	trans_mat(cache->A0, cache->A0T, nb_sat, nb_variables);

	cache->nb_sat = nb_sat;
}

/*
	theta_max = MSVM_2_compute_theta_max(cache,model)
	
	Computation of the largest feasible value for theta 
*/
double MSVM_2_compute_theta_max(const struct TrainingCache *cache, const struct Model *model) {

	long i,k,ind_pattern;
	const long Q = model->Q;
	const long chunk_size = cache->chunk_size;
	
	double theta_partial;
	double theta_max = HUGE_VAL;

	for(i=1; i<=chunk_size; i++)
	  {
	  ind_pattern = cache->table_chunk[i];
	  for(k=1; k<=Q; k++)
	    if((k != model->y[ind_pattern]) && (cache->matrix_y_bar[i][k] < 0.0))
	      {
	      theta_partial = - model->alpha[ind_pattern][k] / cache->matrix_y_bar[i][k];
	      if(theta_partial < theta_max)
		theta_max = theta_partial;
	      }
	  }
	
	return theta_max;
}


/* 
	Computation of the vector H bar{y} used in compute_theta_opt 
*/
void MSVM_2_compute_H_y_bar(struct TrainingCache *cache, const struct Model *model)
{
	long i,j,k,l,ind_pattern, y_i;
	const long Q = model->Q;
	double Qd = (double)Q;
	const long chunk_size = cache->chunk_size;

	double partial, partial_sum_y_bar = 0.0;

	for(i=1; i<=chunk_size; i++) {
	  y_i = model->y[cache->table_chunk[i]];
	  for(k=1; k<=Q; k++)
	    {
	    cache->H_y_bar[i][k] = 0.0;
	    if(k != y_i)
	      {
	      for(j=1; j<=chunk_size; j++)
		{
		ind_pattern = cache->table_chunk[j];
		partial_sum_y_bar = 0.0;

		for(l=1; l<=Q; l++)
		  if(l != model->y[ind_pattern])
		    partial_sum_y_bar += cache->matrix_y_bar[j][l];

		partial = cache->matrix_y_bar[j][k] - (partial_sum_y_bar / Qd);
		cache->H_y_bar[i][k] += partial * cache->K[i][ind_pattern];
		}
	      }
	    }
	 }
}

/* 
	theta_opt = MSVM_2_compute_theta_opt(theta_max,cache,model)
	
	Computation of the optimal value for theta 
*/
double MSVM_2_compute_theta_opt(const double theta_max, struct TrainingCache *cache, const struct Model *model)

{
	long i,k, y_i;
	const long Q = model->Q;
	const long chunk_size = cache->chunk_size;
	
	double y_barT_H_y_bar=0.0;
	double theta_opt = 0.0;
	
	MSVM_2_compute_H_y_bar(cache,model);

	for(i=1; i<=chunk_size; i++) {
	  y_i = model->y[cache->table_chunk[i]];
	  for(k=1; k<=Q; k++)
	    if(k != y_i)
	      y_barT_H_y_bar += cache->matrix_y_bar[i][k] * cache->H_y_bar[i][k];
	}
	
	if(y_barT_H_y_bar <= 0.0)
	  {
	  printf("\nThe value of the quadratic form is: %e\n", y_barT_H_y_bar);
	  printf("\nMatrix bar{y}\n");
	  display_mat(cache->matrix_y_bar, chunk_size, Q, 7, 4);
	  exit(0);
	  }

	if(y_barT_H_y_bar > tiny)
	  theta_opt = minimum (- cache->gradientT_y_bar / y_barT_H_y_bar, theta_max);
	else
	  {
	  printf("\nbar{y}^T H bar{y} is too small: %e", y_barT_H_y_bar);
	  theta_opt = 0.0;
	  }

	return theta_opt;
}

/* 
	delta_nb_SV = compute_new_alpha(theta_opt,cache,model)

	Computation of the new value of the vector of dual variables alpha
	and return the difference in number of SVs
*/
long MSVM_2_compute_new_alpha(const double theta_opt, const struct TrainingCache *cache, struct Model *model)
{
	long i,k,ind_pattern;
	const long Q = model->Q;
	const double Qd = (double)Q;
	const long chunk_size = cache->chunk_size;
	long nb_SV = 0;

	for(i=1; i<=chunk_size; i++)
	  {
	  ind_pattern = cache->table_chunk[i];
	  model->partial_average_alpha[ind_pattern] = 0.0;
	  // Assume x_i is not a SV
	  if(cache->activeset[ind_pattern]) {
		cache->activeset[ind_pattern] = 0;
	  	nb_SV--;
	  }
	  
	  for(k=1; k<=Q; k++)
	    if(k != model->y[ind_pattern])
	      {
	      cache->alpha_update[i][k] = theta_opt * cache->matrix_y_bar[i][k];
	      model->alpha[ind_pattern][k] += cache->alpha_update[i][k];
	      
	      if(model->alpha[ind_pattern][k] < 0.0)
		model->alpha[ind_pattern][k] = 0.0;
	      else if (model->alpha[ind_pattern][k] != 0.0) {
    	    	// Update nb_SV and activeset
    		if(cache->activeset[ind_pattern] == 0) {
    			cache->activeset[ind_pattern] = 1;  	
    			nb_SV++;
    		}
	      }
	      model->partial_average_alpha[ind_pattern] += model->alpha[ind_pattern][k];
	      }
	    else
	      cache->alpha_update[i][k] = 0.0;
	      
	  model->partial_average_alpha[ind_pattern] /= Qd;  
	  }
	  return nb_SV;
}


/* 
	Check the feasibility of the initial alpha 
*/
int MSVM_2_check_feasible_sol_init(struct Model *model)
{
	long i,k;
	const long Q = model->Q;
	const long nb_data = model->nb_data;

	double norm;
	int feasible = true;

	double *constraints = (double *) calloc(Q+1, sizeof(double));
	
	model->sum_all_alpha = 0.0;

	for(i=1; i<=nb_data; i++)
	  for(k=1; k<=Q; k++)
	    if(((k == model->y[i]) && (model->alpha[i][k] != 0.0)) || (model->alpha[i][k] < 0.0))
	      {
	      printf("\nNo feasible solution: alpha[%ld][%ld] = %e\n\n",
		     i, k, model->alpha[i][k]);
	      feasible = false;
	      break;
	      }
	    else
	      model->sum_all_alpha += model->alpha[i][k];

	for(k=1; k<=Q; k++)
	  constraints[k] = model->sum_all_alpha / Q;

	for(i=1; i<=nb_data; i++)
	  for(k=1; k<=Q; k++)
	    constraints[k] -= model->alpha[i][k];

	norm = 0.0;

	for(k=1; k<=Q; k++)
	  norm += constraints[k] * constraints[k];

	norm = sqrt(norm);

	if(norm >= very_small)
	  {
	  feasible = false;
	  printf("\n\nLarge deviation of the equality constraints...\n\n");	  
	  }

	free(constraints);
	return feasible;
}


/* 
	Check the feasibility of the current solution alpha 
*/
int MSVM_2_check_feasible_sol_train(struct Model *model,  struct TrainingCache *cache)
{
	long i,k;
	const long Q = model->Q;
	const long nb_data = model->nb_data;

	double norm;
	int feasible = true;

	model->sum_all_alpha = 0.0;

	for(i=1; i<=nb_data; i++)
	  for(k=1; k<=Q; k++)
	    if(((k == model->y[i]) && (model->alpha[i][k] != 0.0)) || (model->alpha[i][k] < 0.0))
	      {
	      printf("\nNo feasible solution: alpha[%ld][%ld] = %e\n\n",
		     i, k, model->alpha[i][k]);
	      feasible = false;
	      break;
	      }
	    else
	      model->sum_all_alpha += model->alpha[i][k];

	for(k=1; k<=Q; k++)
	  cache->constraints[k] = model->sum_all_alpha / Q;

	for(i=1; i<=nb_data; i++)
	  for(k=1; k<=Q; k++)
	    cache->constraints[k] -= model->alpha[i][k];

	norm = 0.0;

	for(k=1; k<=Q; k++)
	  norm += cache->constraints[k] * cache->constraints[k];

	norm = sqrt(norm);

	if(norm >= very_small)
	  {
	  feasible = false;
	  printf("\n\nLarge deviation of the equality constraints...\n\n");	  
	  }

	return feasible;
}


/* 
	Characterize the achieving of an unfeasible solution 
*/
void MSVM_2_study_unfeasible_sol(const int descent, double **vector_u, double **vector_y_bar, const struct TrainingCache *cache, const struct Model *model)

{
	long k;
	const long Q = model->Q;
	const long chunk_size = cache->chunk_size;
	long nb_sat = cache->nb_sat;
	long nb_variables = cache->nb_variables;
	
	for(k=1; k<=Q; k++)
	  printf("%s%e\n", cache->constraints[k] < 0.0 ? " " : "  ", cache->constraints[k]);

	printf("\nMatrix A0\n");
	display_mat(cache->A0, nb_sat, nb_variables, 7, 4);

	if(descent == 0)
	  printf("\nbar{y} is the null vector");
	else
	  printf("\nbar{y} is not the null vector");
	printf("\nNorm of bar{y}: %e\n", cache->initial_norm_y_bar);

	printf("\nVector u\n");
	display_mat(vector_u, nb_sat, 1, 7, 4);

	printf("\nVector bar{y}\n");
	display_mat(cache->matrix_y_bar, chunk_size, Q, 7, 4);

	mult_mat(cache->A0, vector_y_bar, cache->working_matrix1, nb_sat, nb_variables, 1);
	printf("\nA0 bar{y}\n");
	display_mat(cache->working_matrix1, nb_sat, 1, 9, 6);

	exit(0);
}

/* 
	val_obj_function = MSVM_2_compute_objective_function(model, former_val_obj_function)

	Computation of the value J(alpha) of the objective function 
*/
double MSVM_2_compute_objective_function(const struct Model *model, double former_val_obj_function)
{
	long i,j,k,l;
	const long Q = model->Q;
	double Qd = (double)Q;
	const long nb_data = model->nb_data;
	long dim_input = model->dim_input;
	const enum Kernel_type nature_kernel = model->nature_kernel;
	const double *kernel_par = model->kernel_par;
	
	double Q_term=0.0, L_term=0.0, partial_1, partial_2, kernel, val_obj_function;

	for(i=1; i<=nb_data; i++)
	  for(j=1; j<=nb_data; j++)
	    {
	    partial_1 = 0.0;
	    for(k=1; k<=Q; k++)
	      for(l=1; l<=Q; l++)
		{
		partial_2 = -1.0;
		if(k == l) 
		  partial_2 += Qd;
		partial_1 += partial_2 * model->alpha[i][k] * model->alpha[j][l];
		}
	    if(partial_1 != 0.0)
	      {
	      kernel = ker(nature_kernel, model->X[i], model->X[j], dim_input, kernel_par); 
	      if(i == j)
		kernel += 1 / (2*model->C[model->y[i]]);
	      Q_term += partial_1 * kernel;
	      }
	    }

	Q_term /= Qd;
	L_term = model->sum_all_alpha / (Qd - 1.0);
	val_obj_function = -0.5 * Q_term + L_term;

	printf("\n       Objective function: %e", val_obj_function);

	if(val_obj_function < former_val_obj_function)
	  if(former_val_obj_function - val_obj_function > very_small)
	    {
	    printf("\nThe value of the objective function has decreased...\n");
	    printf("%e < %e\n\n", val_obj_function, former_val_obj_function);
	    exit(0);
	    }
	
	return val_obj_function;
}

/* 	
	MSVM_2_compute_pseudo_inverse(cache)

	Computation of the pseudo_inverse of A0A0T 
*/
void MSVM_2_compute_pseudo_inverse(struct TrainingCache *cache)

{
	long i,k;
	long nb_sat = cache->nb_sat;	
	double diag_kk_inv;

	copy(cache->A0A0T, cache->A0A0T_ref, nb_sat, nb_sat);
	diagonalize(cache->A0A0T, nb_sat, cache->eigenvalues);

//	copy(A0A0T, eigenvectors, nb_sat, nb_sat); 
// A0A0T replaces eigenvectors

//	trans_mat(A0A0T, eigenvectorsT, nb_sat, nb_sat);
// use A0A0T[j][i] for eigenvectorsT[i][j]


//	mult_mat(new_diagonal_matrix, eigenvectorsT, working_matrix1,
//		 nb_sat, nb_sat, nb_sat);

// This mulplication by a diagonal matrix is done in the loop below:

	for(k=1; k<=nb_sat; k++)
	  if(fabs(cache->eigenvalues[k]) > very_small) {
	    diag_kk_inv = 1.0 / cache->eigenvalues[k];
	    
	    for(i=1;i<=nb_sat;i++)
	    	cache->working_matrix1[k][i] = diag_kk_inv * cache->A0A0T[i][k]; // = diaginv * eigenvectorsT[k][i];
	  }
	  else {

	    for(i=1;i<=nb_sat;i++)
	    	cache->working_matrix1[k][i] = 0.0;

	  }
	  
	mult_mat(cache->A0A0T, cache->working_matrix1, cache->A0A0T_1,
		 nb_sat, nb_sat, nb_sat);
}

