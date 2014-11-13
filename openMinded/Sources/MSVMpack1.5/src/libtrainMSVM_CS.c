/* Copyright 2000-2010 Yann Guermeur                                        */

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
/*  Name           : libtrainMSVM_CS.c                                      */
/*  Version        : 2.0                                                    */
/*  Creation       : 04/27/00                                               */
/*  Last update    : 02/04/11                                               */
/*  Subject        : Training algorithm of the M-SVM of Crammer and Singer  */
/*  Algo.          : Frank-Wolfe algorithm + decomposition method           */
/*  Author         : Fabien Lauer and Yann Guermeur Yann.Guermeur@loria.fr  */
/*--------------------------------------------------------------------------*/

#include "libtrainMSVM_CS.h"


/*
	Crammer and Singer (CS) MSVM training function
	
	Solves the dual problem wrt alpha:
	
	min  	1/2 alpha' H alpha  + sum_i,k ( delta_yi,k alpha_ik )
	
	s.t. 	alpha_ik >= 0,    	for all i,k, such that 1 <= i <= m, 1 <= k <= Q
		sum_k alpha_ik = C, 	for all i, 1 <= i <= m
		
	where
	H_ik,jl = (delta_yi,yj - delta_yi,l - delta_yj,k + delta_k,l) k(x_i,x_j) 
		
*/
long MSVM_CS_train(struct Model *model, struct Data *training_set, long chunk_size, const double accuracy, int cache_memory, const int nprocs, char *alpha0_file, char *model_tmp_file, char *log_file)
{
	long return_status = -1;
	FILE *fp;
	int t;
	pthread_t *threads = (pthread_t *)malloc(sizeof(pthread_t) * nprocs); 	// threads id
	void *status; 			// for pthread_join
	int rc; 			// return code of pthread functions
	pthread_t thread_monitor;	
	enum AlphaInitType alphaiszero;
	
	if(training_set != NULL) {
		// Initialize model for this particular training set		
		model_load_data(model, training_set);		
	}
	if(model->nb_data > 2147483647) {
		printf("Cannot compute table chunk: number of data above random number limit\n");
		exit(0);
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

	double **gradient = matrix(model->nb_data, model->Q);
	double **H_alpha = matrix(model->nb_data, model->Q);	
	double best_primal_upper_bound = HUGE_VAL;
	int *activeset = (int *)calloc(model->nb_data+1, sizeof(int));	
	
	// including the kernel cache:
	struct KernelCache kc;
	unsigned long cache_size = (unsigned long)cache_memory * MBYTES;	
	kernel_initialize_cache(cache_size, &kc, model);

	printf("Initialization...");fflush(stdout);

	// Initialize alpha and b
	if(training_set != NULL)  // otherwise resume training
		CS_init_alpha(model, alpha0_file);
	 
	if(training_set == NULL || alpha0_file != NULL)
		alphaiszero = ALPHA_NOT_ZERO;
	else
		alphaiszero = ALPHA_IS_ZERO;
	
	// Initialize gradient
	CS_init_gradient(alphaiszero, gradient,H_alpha,model);

	printf(" Done.\n");
	
	// Initialize display
	if(accuracy>0)
		print_training_info(0,model); 
	else {
		printf("Training...");fflush(stdout);
	}
	
	EVAL = 0;	// triggered by signal handler to call eval
	STOP = 0;	// triggered by user to stop training
	model->iter = 1;  	// number of iterations (over all threads)

	// Prepare monitoring of model
	struct MonitorData monitor_data;
	monitor_data.model_tmp_file = model_tmp_file;
	monitor_data.period = MONITOR_PERIOD;
	monitor_data.model = model;

	pthread_create(&thread_monitor, NULL, MSVM_monitor_model_thread, (void *) &monitor_data);


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
	thread_data.best_primal_upper_bound = &best_primal_upper_bound;
	thread_data.activeset = activeset;
	thread_data.nb_SV = &nb_SV;	
	thread_data.logfile_ptr = fp;

	// Launch nprocs (= #CPUs) computing threads
	int numthreads=nprocs;
	 
	if(model->nb_data >= 5000) {
		// Except if working set selection is used
		// then retain 25% of CPUs for gradient updates
		numthreads = nprocs - nprocs/4 + 1;	
		if(numthreads > nprocs)
			numthreads=nprocs;
	}
	
	for(t=0;t<numthreads;t++) {
	
		pthread_mutex_lock(&thread_data_mutex); // Wait for thread_data to be read by previous thread
 		
		thread_data.thread_id = t;
		rc = pthread_create(&threads[t], NULL, CS_train_thread, (void *) &thread_data);	
	}
	
	// Wait for threads to terminate
	for(t=0;t<numthreads;t++) {
		rc = pthread_join(threads[t],&status);

		// Check if optimum has been reached by this thread		
		if((void *)status == 0)
			return_status=0;		
	}
	
	// Cancel monitor thread
	#ifdef _WIN32
	pthread_cond_broadcast(&monitor_cond);
	#endif
	rc = pthread_cancel(thread_monitor);
	rc = pthread_join(thread_monitor,&status);	
		
	if(log_file != NULL)
		fclose(fp);

	free(gradient[1]);free(gradient);
	free(H_alpha[1]);free(H_alpha);
	free(activeset);	
	kernel_free_cache(&kc);
	free(threads);
	
	if(model->iter == MSVM_TRAIN_MAXIT)
		return_status = 1; // max iterations.
	else if(return_status != 0)
		return_status = model->iter; // interrupted by user 

	return return_status;
}

/*
	Main computing thread
*/
void *CS_train_thread(void *th_data) {

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
	double *best_primal_upper_bound = data->best_primal_upper_bound;
	int *activeset = data->activeset;
	long *nb_SV = data->nb_SV;
	FILE *fp = data->logfile_ptr;	
	
	pthread_mutex_unlock(&thread_data_mutex);	// Release thread_data for next thread 


	// Prepare the cache
	struct TrainingCache cache;
	cache.chunk_size =  chunk_size;
	CS_alloc_memory(&cache, model->Q, model->nb_data, chunk_size);
	cache.kc = kernelcache;
	cache.activeset = activeset;
	
	// Disable parallel gradient computation and working set selection for small data sets
	int parallel_gradient_update = 1;
	int workingset_selection = 1;
	
	if(model->nb_data < 5000) {
		parallel_gradient_update = 0;
		workingset_selection = 0;
	}
	
	/*
		Prepare parallel gradient computations:
		- the gradient vector is split into numthreads_grad parts (along i)
		- each part is updated by a different thread
	*/
	// max number of threads for gradient updates is nprocs
	pthread_t *grad_threads = (pthread_t *)malloc(sizeof(pthread_t) * nprocs); 

	// start with 1 thread (main load on kernel evaluations)
	int numthreads_grad = 1;		
	
	if(workingset_selection)
		numthreads_grad = nprocs/4; // or #CPUS/4 threads if working set selection is used
		
	if(numthreads_grad==0)
		numthreads_grad = 1;
		
	// More CPUs for gradient updates if Q > dimension of data
	if(model->dim_input < model->Q) 
		numthreads_grad = nprocs;
		
	void *status; 			
	int rc; 		
	long k;	
	struct ThreadGradient_data *grad_data = (struct ThreadGradient_data *)malloc(sizeof(struct ThreadGradient_data) * nprocs);

	if(parallel_gradient_update) {
		for(k=0;k<nprocs;k++) {
			grad_data[k].gradient = gradient;
			grad_data[k].H_alpha = H_alpha;
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
	TP_WORK ** work;
		
	if(parallel_gradient_update) {
		
		work = malloc(sizeof(TP_WORK *) * nprocs);
		for(k=0;k<nprocs;k++)
			work[k] = CreateThreadpoolWork(CS_update_gradient_thread2, (void *) &grad_data[k], NULL);
	}
#endif
			
	// Switch to #CPUs/4 threads for gradient update when 25% of the kernel matrix is cached
	int percentage_step = 1;
	long percentage = model->nb_data / 4;
	int next_numthreads_grad = nprocs/4;
	if(workingset_selection) {
		// Switch to #CPUs/2 threads for gradient update when 10% of the kernel matrix is cached
		percentage = model->nb_data / 10;
		next_numthreads_grad = nprocs/2;
	}
		
	if(next_numthreads_grad == 0) 
		next_numthreads_grad = 1;
	
	// No thread switches if Q > dimension of data (keep all CPUs for gradient updates)
	if(model->dim_input < model->Q) 
		percentage = 2*model->nb_data;
	
		
	// Local variables
	int verbose = 0;
	int do_eval;
	char yesno;	
	long long return_status = -1;
		
	double **delta = matrix(chunk_size, model->Q);
	double theta_opt;
	int jump = false;
	
	if(accuracy == 0)
		do_eval = 0;
	else 
		do_eval = 1;
 
	int thread_stop = 0;
	double psi_max = HUGE_VAL;

	// Select a random chunk of data for the first step
	select_random_chunk(&cache,model);

	do {
	
	  	if((TRAIN_SMALL_STEP < TRAIN_STEP) && (model->iter%TRAIN_SMALL_STEP) == 0) {
		    	printf(".");
			fflush(stdout);
	  	} 	
		//printf("Thread ID : %d\n",thread_id);
		
		// Compute the kernel submatrix for the chunk
  		compute_K(&cache,model);			
 	
		// Enter Critical Section (using and modifying the model)
		pthread_mutex_lock(&(model->mutex)); 

		jump = CS_solve_lp(gradient, &cache, model);
	  	
	  	if(jump == false)
    		jump = CS_check_opt_sol(gradient,&cache,model);
	    		
		if(jump == false) {
			
	      	CS_compute_delta(delta,&cache,model);
	    	theta_opt = CS_compute_theta_opt(delta, &cache, model);		
			*nb_SV += CS_compute_new_alpha(theta_opt,&cache,model);

			if(parallel_gradient_update) {
	
				// Update gradient in parallel 
	   			for(k=0;k<numthreads_grad;k++) {
				#ifdef _WIN32
					SubmitThreadpoolWork(work[k]);
				#else
					rc = pthread_create(&grad_threads[k], NULL, CS_update_gradient_thread, (void *) &grad_data[k]);
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
				CS_update_gradient(gradient,H_alpha, &cache,model); 
			}
			
   		}
	
		// Evaluate termination criterion & print info
		if((do_eval && (model->iter%TRAIN_STEP) == 0) || EVAL || STOP || (do_eval && model->ratio >= accuracy) ) 
		    {    	   	
			if(fp != NULL)
				fprintf(fp,"%ld ",model->iter);
	
			if(EVAL)
				printf("\n\n*** Evaluating the model at iteration %ld...\n",model->iter);
				 
			// Evaluate how far we are in the optimization
			// (prints more info if interrutped by user)
			model->ratio = MSVM_eval(best_primal_upper_bound, gradient, H_alpha, NULL, model, EVAL, fp);

			if(*best_primal_upper_bound < 1e-6 && model->ratio <= 0.0)
				model->ratio = 1.0;

			print_training_info(*nb_SV, model);

			
			if(verbose && workingset_selection)
				printf("\t\t\t\t\t\t  (psi = %e )\n",psi_max);			
			
		
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
	
		  	
		// Select working set (for next iteration)
		if(workingset_selection)
	 		psi_max = CS_workingset_selection(gradient,&cache,model);
	 	else 
	 		(void)CS_compute_table_chunk(&cache,model);
	 		
	 			
			
		// Check if a sufficient % of the kernel matrix is cached
		if( parallel_gradient_update && cache.kc->max_idx >= percentage ) {	
			/*
			 and switch thread to compute gradient upates instead of kernel rows if it is	
			 	(threads are actually stopped to leave the CPUs
				to other threads that will compute gradient updates)
			*/
			if(workingset_selection)	
				thread_stop = CS_switch_thread(nprocs, &numthreads_grad, &next_numthreads_grad, &percentage,  &percentage_step, grad_data, thread_id, model->nb_data);
			else
				thread_stop = switch_thread(nprocs, &numthreads_grad, &next_numthreads_grad, &percentage,  &percentage_step, grad_data, thread_id, model->nb_data);
		}				
								    
  		model->iter++;
		
		// Release mutex: End of critical section
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
	CS_free_memory(&cache);
	free(delta[1]);free(delta);
	free(grad_threads);
	free(grad_data);
		
	pthread_exit((void*)return_status);
}

/*
	CS_init_alpha(model_ptr, alpha_filename)
	
	Initialize the alpha of the model pointed by model_ptr by
		reading them from the file alpha_filename or 
		setting all alpha to 0 except one for each point
		
*/
void CS_init_alpha(struct Model *model, char *alpha_file) {
	long i,k;
	const long Q = model->Q;
	const long nb_data = model->nb_data;
	const double *C = model->C;
	
	// Initialize alpha
	if(alpha_file == NULL) {
		if (model->alpha != NULL) {
			free(model->alpha[1]); free(model->alpha);
		}
		model->alpha = matrix(nb_data, Q);
		model->sum_all_alpha = 0;
		for(i=1; i<=nb_data; i++) {
		
    		    for(k=1; k<=Q; k++)			// Set all alpha_ik to 0
    		    	model->alpha[i][k] = 0.0;
    		    model->alpha[i][model->y[i]] = C[model->y[i]]; // except alpha_iy_i set to C_yi
		
		    model->sum_all_alpha += C[model->y[i]];
		}
		
	}
	else
		read_alpha(model, alpha_file);
		
	return;
}
/* 
	Allocate the memory for the cache 
*/
void CS_alloc_memory(struct TrainingCache *cache, const long Q, const long nb_data, const long chunk_size) 
{
	cache->table_chunk = (long *) calloc(chunk_size+1, sizeof(long));
	cache->table_active = (long *) calloc(nb_data+1, sizeof(long));
	cache->out_of_chunk = (long *) calloc(nb_data+1, sizeof(long));	
	cache->in_chunk = (long *) calloc(nb_data+1, sizeof(long));
	cache->lp_sol = matrix(chunk_size, Q);
	cache->K = (double **)malloc(sizeof(double *) * (chunk_size+1)); 
	cache->H_delta = matrix(chunk_size, Q);
	cache->alpha_update = matrix(chunk_size, Q);
	cache->psi = (double *)malloc(sizeof(double) * (nb_data + 1));
}

/* 
	Free the cache memory 
*/
void CS_free_memory(struct TrainingCache *cache) {
	free(cache->table_chunk);
	free(cache->table_active);
	free(cache->in_chunk);
	free(cache->out_of_chunk);	
	free(cache->K); 
	free(cache->lp_sol[1]);free(cache->lp_sol);
	free(cache->H_delta[1]);free(cache->H_delta);	
	free(cache->alpha_update[1]);free(cache->alpha_update);
	free(cache->psi);
}
/*
	Solve the LP subproblem by calling lp_solve API 
*/
int CS_solve_lp(double **gradient, const struct TrainingCache *cache, const struct Model *model)
{
	long i,k,ind_pattern;
	const long Q = model->Q;
	const long chunk_size = cache->chunk_size;
	const double *C = model->C;
	
	const int nRows = chunk_size;
	const int nCols = chunk_size * Q;

	double *obj = (double*)malloc(sizeof(double) * (1+nCols));
	double *row = (double*)malloc(sizeof(double) * Q);
	int *cols = (int*)malloc(sizeof(int) * Q);
	long **lp_sol_table = matrix_l(nCols, 2);
	long **lp_sol_table_inv = matrix_l(chunk_size, Q);
	double *sol = (double*)malloc(sizeof(double) * (1+nRows+nCols));
	double epsel;
		
	// Make LP
	lprec *lp = make_lp(0, nCols);
	set_add_rowmode(lp, TRUE);

	// Make objective function
	int col = 1;
	for(i=1; i<=chunk_size; i++)
	  {
	  ind_pattern = cache->table_chunk[i];
	  for(k=1; k<=Q; k++)
	      {
	      	obj[col] = gradient[ind_pattern][k];
		lp_sol_table[col][1] = i;	// keep a table of correspondance between
		lp_sol_table[col][2] = k;	// LPSOLVE vector of variables and lp_sol matrix
		lp_sol_table_inv[i][k] = col++; // lp_sol[i][k] = the 'lp_solve_table_inv[i][k]'-th variable for LPSOLVE
	      }
	  }
	set_obj_fn(lp, obj);

	// Make constraints : forall i,  sum_k alpha_ik = C_yi
	for(i=1; i<=chunk_size; i++) {
		for(k=1; k<=Q; k++) {
			cols[k-1] = (int)lp_sol_table_inv[i][k];
			row[k-1] = 1.0;			
		}
		add_constraintex(lp, Q, row, cols, EQ, C[model->y[cache->table_chunk[i]]]); 
	}
		
	// End of LP making
	set_add_rowmode(lp, FALSE);	
	//print_lp(lp);
	
	// Solve LP
	int jump = false;
	set_outputfile(lp,"");

	if(solve(lp)) {
		printf("Problem with the LP... \n");
		jump = true;
	}
	else {
		// Recover solution in the matrix lp_sol
		get_primal_solution(lp, sol);	// sol: template for lp_solve solution format
						// 	sol=[obj, constraints, variables] 

		epsel = get_epsel(lp);	// tolerance in lp_solve
		
		// Put solution into lp_sol
		for(col=1; col<= nCols; col++) {

			// Check feasibility of the col-th variable
			if((sol[nRows+col] < -epsel) || (sol[nRows+col] > C[model->y[cache->table_chunk[lp_sol_table[col][1]]]] + epsel)) {
				jump = true;
				break;
			}
			// Round off tolerance
			if(fabs(sol[nRows+col]) < epsel) 
				sol[nRows+col] = 0.0;
				
			else if(fabs(sol[nRows+col] - C[model->y[cache->table_chunk[lp_sol_table[col][1]]]]) < epsel)
				sol[nRows+col] = C[model->y[cache->table_chunk[lp_sol_table[col][1]]]];
				
			// Set the value in lp_sol matrix
			cache->lp_sol[lp_sol_table[col][1]][lp_sol_table[col][2]] = sol[nRows+col];
		}
		
	}

	delete_lp(lp);

	free(obj);
	free(row);
	free(cols);
	free(lp_sol_table[1]);free(lp_sol_table);
	free(lp_sol_table_inv[1]);free(lp_sol_table_inv);
	free(sol);

	return jump;	
}

/* 
	nb_SV = CS_compute_table_chunk(model)
	
	Select a subset of the training set for the next step of optimization 
*/

long CS_compute_table_chunk(struct TrainingCache *cache, const struct Model *model)
{
	long i,random_num,rank;
	const long nb_data = model->nb_data;
	const long chunk_size = cache->chunk_size;
	const long half_chunk_size = chunk_size / 2;
	
	long nb_points_chunk,nb_active_var_in_chunk,nb_active_var = 0;

	for(i=1; i<=nb_data; i++)
	  {	
	  cache->in_chunk[i] = 0;

	  if(cache->activeset[i]) 
	  	cache->table_active[++nb_active_var] = i;
	  }		  	  

	nb_points_chunk = 0;
	nb_active_var_in_chunk = nb_active_var;
	if(nb_active_var_in_chunk > half_chunk_size)
		nb_active_var_in_chunk = half_chunk_size;


	if(nb_active_var_in_chunk == nb_active_var)
	  for(nb_points_chunk=1; nb_points_chunk <= nb_active_var; nb_points_chunk++)
	    {
	    cache->table_chunk[nb_points_chunk] = cache->table_active[nb_points_chunk];
	    cache->in_chunk[cache->table_active[nb_points_chunk]] = 1;
	    }
	else
	  while(nb_points_chunk < nb_active_var_in_chunk)  
	    {
	    random_num = nrand48(xi);
	    rank = (random_num % nb_active_var)+1;
	    if(cache->in_chunk[cache->table_active[rank]] == 0)
	      {
	      cache->in_chunk[cache->table_active[rank]] = 1;
	      nb_points_chunk++;
	      cache->table_chunk[nb_points_chunk] = cache->table_active[rank];
	      }
	    }

	nb_points_chunk = nb_active_var_in_chunk;

	while(nb_points_chunk < chunk_size)
	  {
	  random_num = nrand48(xi);
	  rank = (random_num % nb_data)+1;
	  if(cache->in_chunk[rank] == 0)
	    {
	    cache->in_chunk[rank] = 1;
	    nb_points_chunk++;
	    cache->table_chunk[nb_points_chunk] = rank;
	    }
	  }
	 
	  return nb_active_var;
}


void CS_init_gradient(const enum AlphaInitType alpha_init_type, double **gradient, double **H_alpha, const struct Model *model)
{
	long i,j,k,l,y_i,y_j;
	const long nb_data = model->nb_data;
	const long Q = model->Q;
	const enum Kernel_type nature_kernel = model->nature_kernel;
	const double *kernel_par = model->kernel_par;
	const long dim_input = model->dim_input;
	
	double partial, gradient_ik;
	
	if(alpha_init_type == ALPHA_IS_ZERO) {	// Fast initialization
	/*
	Matrix alpha is not really zero, but only alpha_i_y_i = C,
	which results in gradient_ik = 0 except for gradient_i_y_i = 1
	*/
	
		for(i=1; i<=nb_data; i++) {		  
		  for(k=1; k<=Q; k++)
		    {
		    gradient[i][k] = 0.0;
		    H_alpha[i][k] = 0.0;
		    }
		    gradient[i][model->y[i]] = 1.0;
		 }
	}
	else {	// Do a proper initialization based on the values of alpha
		for(i=1; i<=nb_data; i++)
		  {
		  if(i%1000 == 0) {
		  	printf(".");
		  	fflush(stdout);
		  }
		  
		  y_i = model->y[i];
		  
		  for(k=1; k<=Q; k++)
		    {
		    if(k != y_i)
		      {
		      gradient_ik = 0.0;
		      for(j=1; j<=nb_data; j++)
			{
			y_j = model->y[j];
			partial = 0.0;

			if(y_j == y_i)
			  for(l=1; l<=Q; l++)
			    partial += model->alpha[j][l];

			if(y_j == k)
			  for(l=1; l<=Q; l++)
			    partial -= model->alpha[j][l];

			partial += model->alpha[j][k] - model->alpha[j][y_i];
			if(partial != 0.0)
				gradient_ik += partial * ker(nature_kernel, model->X[i], model->X[j], dim_input,kernel_par);
			}
		      
		      gradient[i][k] = gradient_ik;
	     	      H_alpha[i][k] = gradient_ik;		    
		      }
		    else {
		    	gradient[i][k] = 1.0;
		    	H_alpha[i][k] = 0.0;
		    	}
		    }
		  }
	}
}

/*
	Thread for updates of the gradient in parallel
	(spawn by main computing thread CS_train_thread)
*/
void *CS_update_gradient_thread(void *th_data)
{
	// Recover thread data
	struct ThreadGradient_data *data = (struct ThreadGradient_data *) th_data;
	double **gradient = data->gradient;	
	double **H_alpha = data->H_alpha;	
	struct TrainingCache *cache = data->cache;
	const struct Model *model = data->model;
	const long start_i = data->start_i;
	const long end_i = data->end_i;	
		
	// Local variables
	long i,j,k,l,y_i,y_j,ind_pattern;
	const long Q = model->Q;
	const long chunk_size = cache->chunk_size;
	
	double partial, gradient_update_ik;					
					
	for(i=start_i; i<=end_i; i++)
	  {
	  
	  y_i = model->y[i];
	  for(k=1;k<=Q;k++) {
	  	
	    if(k != y_i)
	      {
	      gradient_update_ik = 0.0;
	      for(j=1; j<=chunk_size; j++)
		{
		ind_pattern = cache->table_chunk[j];
		y_j = model->y[ind_pattern];
		partial = 0.0;

		if(y_j == y_i)
		  for(l=1; l<=Q; l++)
		    partial += cache->alpha_update[j][l];

		if(y_j == k)
		  for(l=1; l<=Q; l++)
		    partial -= cache->alpha_update[j][l];

		partial += cache->alpha_update[j][k] - cache->alpha_update[j][y_i];
		gradient_update_ik += partial * cache->K[j][i];
		}
	      
	      gradient[i][k] += gradient_update_ik;
	      H_alpha[i][k] += gradient_update_ik;
	      }	     
	  
	  }
	}
	pthread_exit(EXIT_SUCCESS);
}
#ifdef _WIN32
void CS_update_gradient_thread2(PTP_CALLBACK_INSTANCE instance, void *th_data)
{
	// Recover thread data
	struct ThreadGradient_data *data = (struct ThreadGradient_data *) th_data;
	double **gradient = data->gradient;	
	double **H_alpha = data->H_alpha;	
	struct TrainingCache *cache = data->cache;
	const struct Model *model = data->model;
	const long start_i = data->start_i;
	const long end_i = data->end_i;	
		
	// Local variables
	long i,j,k,l,y_i,y_j,ind_pattern;
	const long Q = model->Q;
	const long chunk_size = cache->chunk_size;
	
	double partial, gradient_update_ik;					
					
	for(i=start_i; i<=end_i; i++)
	  {
	  
	  y_i = model->y[i];
	  for(k=1;k<=Q;k++) {
	  	
	    if(k != y_i)
	      {
	      gradient_update_ik = 0.0;
	      for(j=1; j<=chunk_size; j++)
		{
		ind_pattern = cache->table_chunk[j];
		y_j = model->y[ind_pattern];
		partial = 0.0;

		if(y_j == y_i)
		  for(l=1; l<=Q; l++)
		    partial += cache->alpha_update[j][l];

		if(y_j == k)
		  for(l=1; l<=Q; l++)
		    partial -= cache->alpha_update[j][l];

		partial += cache->alpha_update[j][k] - cache->alpha_update[j][y_i];
		gradient_update_ik += partial * cache->K[j][i];
		}
	      
	      gradient[i][k] += gradient_update_ik;
	      H_alpha[i][k] += gradient_update_ik;
	      }	     
	  
	  }
	}
	//pthread_exit(EXIT_SUCCESS);
}
#endif
/*
	Old-style non-threaded gradient update
*/
void CS_update_gradient(double **gradient, double **H_alpha, struct TrainingCache *cache, const struct Model *model)
{
	long i,j,k,l,y_i,y_j,ind_pattern;
	const long nb_data = model->nb_data;
	const long Q = model->Q;
	const long chunk_size = cache->chunk_size;
	
	double partial, gradient_update_ik;
		
	for(i=1; i<=nb_data; i++)
	  {
	  y_i = model->y[i];
	  
	  for(k=1; k<=Q; k++)
	    {
	    if(k != y_i)
	      {
	      gradient_update_ik = 0.0;
	      for(j=1; j<=chunk_size; j++)
		{
		ind_pattern = cache->table_chunk[j];
		y_j = model->y[ind_pattern];
		partial = 0.0;

		if(y_j == y_i)
		  for(l=1; l<=Q; l++)
		    partial += cache->alpha_update[j][l];

		if(y_j == k)
		  for(l=1; l<=Q; l++)
		    partial -= cache->alpha_update[j][l];

		partial += cache->alpha_update[j][k] - cache->alpha_update[j][y_i];
		gradient_update_ik += partial * cache->K[j][i];
		}
	      
	      gradient[i][k] += gradient_update_ik;
	      H_alpha[i][k] += gradient_update_ik;
	      
	      }	     
	    }
	  }

}

/*
	Same as WW_compute_H_delta()
*/
void CS_compute_H_delta(double **delta, struct TrainingCache *cache, const struct Model *model)
{
	long i,k,j,y_i,y_j,l;
	const long Q = model->Q;
	const long chunk_size = cache->chunk_size;
	double partial;

	for(i=1; i<=chunk_size; i++)
	  {
	  y_i = model->y[cache->table_chunk[i]];

	  for(k=1; k<=Q; k++)
	    {
	    cache->H_delta[i][k] = 0.0; 
	    if(k != y_i)
	      {
	      for(j=1; j<=chunk_size; j++)
		{
		y_j = model->y[cache->table_chunk[j]];
		partial = 0.0;

		if(y_j == y_i)
		  for(l=1; l<=Q; l++)
		    partial += delta[j][l];

		if(y_j == k)
		  for(l=1; l<=Q; l++)
		    partial -= delta[j][l];

		partial += delta[j][k] - delta[j][y_i];
		cache->H_delta[i][k] += partial * cache->K[i][cache->table_chunk[j]];
		}
	      }
	    }
	  }
}

int CS_check_opt_sol(double **gradient, struct TrainingCache *cache, const struct Model *model)
{
	double gradient_alpha = 0.0; 
	double gradient_Gamma = 0.0;
	double gradient_diff = 0.0;
	long i,k,ind_pattern;
	const long Q = model->Q;
	const long chunk_size = cache->chunk_size;
	int jump = false;
	
	for(i=1; i<=chunk_size; i++)
	  {
	  ind_pattern = cache->table_chunk[i];	 
	  for(k=1; k<=Q; k++)
	      {
	      gradient_alpha += gradient[ind_pattern][k] * model->alpha[ind_pattern][k];
	      gradient_Gamma += gradient[ind_pattern][k] * cache->lp_sol[i][k];
	      }
	  }

	gradient_diff = gradient_alpha - gradient_Gamma;

	if(gradient_diff <= 0.0)
	  {
#ifdef _WIN32	
		if(gradient_diff < -Small)
	    {
	    printf("\nInappropriate subset selection...\n");
	    printf("\ngradient^T (alpha - gamma): %lf", gradient_diff);
	    }
#else
		if(gradient_diff < -small)
	    {
	    printf("\nInappropriate subset selection...\n");
	    printf("\ngradient^T (alpha - gamma): %lf", gradient_diff);
	    }
#endif
	
	  jump = true;
	  }
	

	cache->gradient_diff = gradient_diff;
	return jump;

}

/*
	Update model paramaters alpha_ik 
	Return the difference in number of SVs
*/
long CS_compute_new_alpha(const double theta_opt, const struct TrainingCache *cache, struct Model *model)
{
	long i,k,ind_pattern,y_i;
	const long Q = model->Q;
	const double *C = model->C;
	const long chunk_size = cache->chunk_size;
	double alpha_tmp;
	long nb_SV = 0;
	
	for(i=1; i<=chunk_size; i++) {
	  ind_pattern = cache->table_chunk[i];
	  y_i = model->y[ind_pattern];
	  
	  // Assume x_i is not a SV
	  if(cache->activeset[ind_pattern]) {
		cache->activeset[ind_pattern] = 0;
	  	nb_SV--;
	  }
	  
	  // Update alpha_ik
	  for(k=1; k<=Q; k++)
	   {	   	   
	    cache->alpha_update[i][k] = theta_opt * ( cache->lp_sol[i][k] - model->alpha[ind_pattern][k] );
	    alpha_tmp = model->alpha[ind_pattern][k] + cache->alpha_update[i][k];
	    
	    if(alpha_tmp > C[y_i] && alpha_tmp < C[y_i] * ( 1.0 + small_enough)) {
		cache->alpha_update[i][k] = C[y_i] - model->alpha[ind_pattern][k];
	   	model->alpha[ind_pattern][k] = C[y_i];	   	
	    }
	    else {  
	    	model->alpha[ind_pattern][k] = alpha_tmp;
	    	if(alpha_tmp > 0.0 && alpha_tmp < C[y_i])  {
	    		// Update nb_SV and activeset
	    		if(cache->activeset[ind_pattern] == 0) {
	    			cache->activeset[ind_pattern] = 1;  	
	    			nb_SV++;
	    		}
	    	}
	      }
	   }
	 }	  
	return nb_SV;
}

void CS_compute_delta(double **delta, const struct TrainingCache *cache, const struct Model *model)
{
	long i,k,ind_pattern;
	const long Q = model->Q;
	const long chunk_size = cache->chunk_size;

	for(i=1; i<=chunk_size; i++)
	  {
	  ind_pattern = cache->table_chunk[i];
	  for(k=1; k<=Q; k++)
	    delta[i][k] = model->alpha[ind_pattern][k] - cache->lp_sol[i][k];
	  }
}

double CS_compute_theta_opt(double **delta, struct TrainingCache *cache, const struct Model *model)
{
	long i,k, y_i;
	const long chunk_size = cache->chunk_size;
	const long Q = model->Q;

	double theta_opt, denominator = 0.0;

	CS_compute_H_delta(delta, cache,model);

	for(i=1; i<=chunk_size; i++) {
	  y_i = model->y[cache->table_chunk[i]];
	  for(k=1; k<=Q; k++)
	    if(k != y_i)
	       denominator += delta[i][k] * cache->H_delta[i][k];
	}
	
	if(denominator < 0) {
		printf("\nWrong estimate of theta: denominator= %e\n", denominator);
		exit(0);
	}
	else if (denominator == 0)
		theta_opt = 0.0;
	else
		theta_opt = cache->gradient_diff / denominator;

	if(theta_opt > 1.0)
	  theta_opt = 1.0;
	
	return theta_opt;
}


/*
	Working set selection based on KKT condition

	select points with maximal violation of the KKT condition
	as measured by

	psi_i = max_{k, alpha_ik>0} gradient_ik   -  min_k gradient_ik

	returns max_i (psi_i) 
*/
double CS_workingset_selection(double **Fmatrix, struct TrainingCache *cache, const struct Model *model) {
	//int isSV=0;
	
	long i,j;
	const long chunk_size = cache->chunk_size;
	long *next_chunk = cache->table_chunk;
	double *psi = cache->psi;
	const long Q = model->Q;
	const long nb_data = model->nb_data;	
	long k,ind_pattern;	
	double Fmax, Fmin,Fik, psi_max;
						
	for(j=1; j<=chunk_size; j++) 
		next_chunk[j] = 0;

	
	for(i=1; i<=nb_data; i++) {
  	  Fmax = -HUGE_VAL;
	  Fmin = HUGE_VAL;

	  for(k=1; k<=Q; k++) {
   	      	Fik = Fmatrix[i][k];
  	
  	      	if(Fik > Fmax && (model->alpha[i][k] > 0.001))
	        	Fmax = Fik;
		if(Fik < Fmin ) 
	        	Fmin = Fik;	   
	   }
	   
	  // Update psi_i
	  psi[i] = Fmax - Fmin;
	  	
	  // and the ordered list of maximal psi  	  
	  for(j=1; j<=chunk_size; j++) {
	  	ind_pattern = next_chunk[j];
	  	if(ind_pattern == 0 || psi[i] > psi[ind_pattern]) {
	  		// Insert i here
	  		for(k=chunk_size;k>j;k--)
	  			next_chunk[k] = next_chunk[k-1];
	  		next_chunk[j] = i;
	  		break;
	  	}
	  }
	}
	psi_max = psi[next_chunk[1]];
	/*
	for(j=1; j<=chunk_size; j++) {
  		if(model->alpha[next_chunk[j]][model->y[next_chunk[j]]] < model->C)
  			isSV++;
	}
	if(isSV < chunk_size)
		printf("isSV=%d\n",isSV);
	*/	
	return psi_max;
}

/*
	This function overrides switch_thread() from libtrainMSVM.C
	with better thresholds when nonrandom working set selection is used.

	Switch working load between threads 
	from main (kernel) computations to gradient updates
	
	As as result, the number of threads used for gradient updates increases
	while main computing threads (CS_train_thread) are terminated

	returns 1 if the calling main thread must stop 
*/
int CS_switch_thread(const int nprocs, int *numthreads_grad, int *next_numthreads_grad, long *percentage, int *percentage_step, struct ThreadGradient_data *grad_data, int thread_id, long nb_data) {	

	int k;

	// Keep (nprocs - next_numthreads_grad + 1) main computing threads
	// for kernel evaluations	
	if(thread_id < nprocs - *next_numthreads_grad + 1){
	
		// This calling thread continues with more children to compute gradient updates
		*numthreads_grad = *next_numthreads_grad;

		// Update splits of the gradient vector
		grad_data[0].start_i = 1;
		grad_data[0].end_i = nb_data / *numthreads_grad;	
		for(k=1;k<*numthreads_grad-1;k++) {	
			grad_data[k].start_i = grad_data[k-1].end_i + 1;
			grad_data[k].end_i = grad_data[k].start_i + nb_data / *numthreads_grad -1;
		}
		if(*numthreads_grad>1) {
			grad_data[*numthreads_grad-1].start_i = grad_data[*numthreads_grad-2].end_i + 1;
			grad_data[*numthreads_grad-1].end_i = nb_data;
		}
		
		// Update percentage for next step
		switch(*percentage_step) {
		case 1:
			*percentage =  nb_data / 5; 		// 20% of kernel matrix
			*next_numthreads_grad = 3*nprocs/4;	// -> 75% of CPUs for gradient
			break;
		case 2:
			*percentage = nb_data / 3; 		// 33%
			*next_numthreads_grad = nprocs-1;	// -> all CPUs - 1
			break;
		case 3:
			*percentage = nb_data / 2; 		// 50%
			*next_numthreads_grad = nprocs;	// -> all CPUs
			break;		
		default:
			*percentage = 2*nb_data; // no more steps
			break;
		}
		if(*next_numthreads_grad == 0)
			*next_numthreads_grad = 1;
			
		*percentage_step += 1;
		return 0;
	}

	else {
		// The calling thread must stop (enough kernel rows have been computed)
		
		//printf("%ld rows of kernel matrix cached, stopping thread %d... \n",percentage,thread_id);
		return 1;
	}

}

