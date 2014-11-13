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
/*  Name           : libtrainMSVM_2fw.c                                     */
/*  Version        : 2.0                                                    */
/*  Creation       : 04/27/00                                               */
/*  Last update    : 11/29/10                                               */
/*  Subject        : Training algorithm of the M-SVM^2 			    */
/*  Algo.          : Frank-Wolfe algorithm + decomposition method           */
/*  Author         : Fabien Lauer and Yann Guermeur Yann.Guermeur@loria.fr  */
/*--------------------------------------------------------------------------*/

#include "libtrainMSVM_2fw.h"

/*
	MSVM2 training function (Frank-Wolfe algorithm)
	
	Solves the dual problem wrt alpha:
	
	min  	1/2 alpha' H_tilde alpha  - (1/Q-1) sum (alpha)
	
	s.t.
	
	alpha_ik >= 0,    			for all i,k, such that 1 <= i <= m, 1 <= k != y_i <= Q
	sum_i (alpha_ik - average_alpha_k) = 0, for all k, 1 <= k <= Q
		
	where
		H_tilde_ik,jl = (delta_k,l - 1/Q) ( k(x_i,x_j) + (1/2C)delta_i,j  )
		
*/
long MSVM_2fw_train(struct Model *model, struct Data *training_set, long chunk_size, const double accuracy, int cache_memory, const int nprocs, char *alpha0_file, char *model_tmp_file, char *log_file)
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

	// Initialize alpha and b
	if(training_set != NULL)  // otherwise resume training
		init_alpha_b(model, alpha0_file);  // use init_alpha_b(model, NULL); to initialize all alpha to 0

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


	// Allocate memory for shared ressources
	double **gradient = matrix(model->nb_data, model->Q);
	double **H_alpha = matrix(model->nb_data, model->Q);
	double **H_tilde_alpha = matrix(model->nb_data, model->Q);
	double best_primal_upper_bound = HUGE_VAL;
	int *activeset = (int *)calloc(model->nb_data+1, sizeof(int));	
	double *lp_rhs = (double*)calloc(model->Q, sizeof(double));
	
	// including the kernel cache:
	struct KernelCache kc;
	unsigned long cache_size = (unsigned long)cache_memory * MBYTES;
	kernel_initialize_cache(cache_size, &kc, model);
	
	printf("Initialization... ");fflush(stdout);
	
	// Initialize gradient
	MSVM2fw_init_gradient(alphaiszero, gradient, H_alpha,H_tilde_alpha, model);
	
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
	thread_data.lp_rhs = lp_rhs;		
	thread_data.logfile_ptr = fp;
	
	// Launch computing threads
	for(t=0;t<nprocs;t++) {
	
		pthread_mutex_lock(&thread_data_mutex); // Wait for thread_data to be read by previous thread
 		
		thread_data.thread_id = t;
		rc = pthread_create(&threads[t], NULL, MSVM2fw_train_thread, (void *) &thread_data);	
	}
	
	// Wait for threads to terminate
	for(t=0;t<nprocs;t++) {
		rc = pthread_join(threads[t],&status);
		
		// Check if optimum has been reached by this thread		
		if((long long)status == 0)
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
	free(lp_rhs);
	kernel_free_cache(&kc);
	free(threads);
	
	if(model->iter == MSVM_TRAIN_MAXIT)
		return_status = 1; // max iterations.
	else if(return_status != 0)
		return_status = model->iter; // interrupted by user

	return return_status;	
}


void *MSVM2fw_train_thread(void *th_data) {

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
	double *lp_rhs = data->lp_rhs;	
	FILE *fp = data->logfile_ptr;	
	
	pthread_mutex_unlock(&thread_data_mutex);	// Release thread_data for next thread 
	 
	// Local variables
	int do_eval;
	char yesno;
	long long return_status = -1;	
	
	// Prepare the cache
	struct TrainingCache cache;
	cache.chunk_size =  chunk_size;
	MSVM2fw_alloc_memory(&cache, model->Q, model->nb_data, chunk_size);
	cache.kc = kernelcache;
	cache.activeset = activeset;
	cache.lp_rhs = lp_rhs;
	
	double **delta = matrix(chunk_size, model->Q);
	double theta_opt;
	int jump = false;
			
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
			work[k] = CreateThreadpoolWork(MSVM2fw_update_gradient_thread2, (void *) &grad_data[k], NULL);
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
		
		jump = MSVM2fw_solve_lp(gradient, &cache, model);
	  	
	  	if(jump == false)
	    		jump = MSVM2fw_check_opt_sol(gradient,&cache,model);
	    		
		if(jump == false) {
			
		      	MSVM2fw_compute_delta(delta,&cache,model);
		    	theta_opt = MSVM2fw_compute_theta_opt(delta, &cache, model);	
		    	*nb_SV += MSVM2fw_compute_new_alpha(theta_opt,&cache,model);

			if(parallel_gradient_update && numthreads_grad > 1) {
				// Update gradient in parallel 
	   			for(k=0;k<numthreads_grad;k++) {
				#ifdef _WIN32
					SubmitThreadpoolWork(work[k]);
				#else
					rc = pthread_create(&grad_threads[k], NULL, MSVM2fw_update_gradient_thread, (void *) &grad_data[k]);
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
				MSVM2fw_update_gradient(gradient,H_alpha, H_tilde_alpha, &cache,model);
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
			// and switch thread to compute gradient updates instead of kernel rows if it is		
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
	MSVM2fw_free_memory(&cache);
	free(delta[1]);free(delta);
	free(grad_threads);
	free(grad_data);
	
	pthread_exit((void*)return_status);
}

/* 
	Allocate the memory for the cache 
*/
void MSVM2fw_alloc_memory(struct TrainingCache *cache, const long Q, const long nb_data, const long chunk_size) 
{
	cache->table_chunk = (long *) calloc(chunk_size+1, sizeof(long));
	cache->out_of_chunk = (long *) calloc(nb_data+1, sizeof(long));
	cache->in_chunk = (long *) calloc(nb_data+1, sizeof(long));
	cache->lp_sol = matrix(chunk_size, Q);
	cache->K = (double **)malloc(sizeof(double *) * (chunk_size+1)); 
	cache->H_delta = matrix(chunk_size, Q);
	cache->alpha_update = matrix(chunk_size, Q);	
}

/* 
	Free the cache memory 
*/
void MSVM2fw_free_memory(struct TrainingCache *cache) {
	free(cache->table_chunk);
	free(cache->out_of_chunk);
	free(cache->in_chunk);
	free(cache->K);
	free(cache->lp_sol[1]);free(cache->lp_sol);
	free(cache->H_delta[1]);free(cache->H_delta);
	free(cache->alpha_update[1]);free(cache->alpha_update);
}
/*
	Solve the LP subproblem by calling lp_solve API 
*/
int MSVM2fw_solve_lp(double **gradient, const struct TrainingCache *cache, const struct Model *model)
{
	long i,k,l,ind_pattern,y_i;
	const long Q = model->Q;
	const double Qd = (double)Q;
	const long chunk_size = cache->chunk_size;
	
	const int nRows = Q-1; 
	const int nCols = chunk_size * (Q-1);

	double *obj = (double*)malloc(sizeof(double) * (1+nCols));
	double *row = (double*)malloc(sizeof(double) * (1+nCols));
	double *rhs = (double*)malloc(sizeof(double) * Q);
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
	  y_i = model->y[ind_pattern];
	  for(k=1; k<=Q; k++)	      
	      if(k != y_i)
	      {
	      	obj[col] = gradient[ind_pattern][k];
		lp_sol_table[col][1] = i;	// keep a table of correspondance between
		lp_sol_table[col][2] = k;	// LPSOLVE vector of variables and lp_sol matrix
		lp_sol_table_inv[i][k] = col++; // lp_sol[i][k] = the 'lp_solve_table_inv[i][k]'-th variable for LPSOLVE
	      }
	      else
	      	cache->lp_sol[i][k] = 0;
	  }
	set_obj_fn(lp, obj);



/*	// Make RHS of constraints
	// -- complete computation --
	for(k=1; k<Q; k++)
	  {
	  rhs[k] = 0.0;
	  for(i=1; i<=nb_data; i++)
	    if(cache->in_chunk[i] == 0)
	      {
	      for(l=1; l<=Q; l++)
		 rhs[k] += model->alpha[i][l];
	      rhs[k] -= Qd * model->alpha[i][k];
		
	      }
	  }
*/	  

	// Make RHS of constraints
	// -- updates to cache->rhs are made in compute_new_alpha()
	//    to keep track of rhs
	//    we only need to remove the contribution of the examples in the chunk
	for(k=1; k<Q; k++)
	  {
	  rhs[k] = cache->lp_rhs[k];
	  for(i=1; i<=chunk_size; i++)
	      {
	      ind_pattern = cache->table_chunk[i];
	      for(l=1; l<=Q; l++)
		 rhs[k] -= model->alpha[ind_pattern][l];
	      rhs[k] += Qd * model->alpha[ind_pattern][k];		
	      }
	  }
	  	
	// Make constraints
	for(k=1; k<Q; k++)
	  {
	  for(col = 1;col <=nCols; col++)
	     row[col] = 0.0;

	  for(i=1; i<=chunk_size; i++)
	    {
	    ind_pattern = cache->table_chunk[i];
	    y_i = model->y[ind_pattern];

            for(l=1; l<=Q; l++)
              if(l != y_i)
        	{
        	row[lp_sol_table_inv[i][l]] = -1.0;
        	if(l == k)
        	     row[lp_sol_table_inv[i][l]] += Qd;
		}
	    }

	  add_constraint(lp, row, EQ, rhs[k]);
	  }

	// No upper bound constraints of the type alpha <= C for MSVM2
	// But we need an upper bound anyway 
	for(col=1;col<=nCols;col++)
		set_upbo(lp, col, 1000);
	
	
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
			if(sol[nRows+col] < -epsel) {
				jump = true;
				break;
			}
			// Round off tolerance
			if(fabs(sol[nRows+col]) < epsel) 
				sol[nRows+col] = 0.0;							
				
			// Set the value in lp_sol matrix
			cache->lp_sol[lp_sol_table[col][1]][lp_sol_table[col][2]] = sol[nRows+col];
		}
		
	}

	delete_lp(lp);

	free(obj);
	free(row);
	free(rhs);
	free(lp_sol_table[1]);free(lp_sol_table);
	free(lp_sol_table_inv[1]);free(lp_sol_table_inv);
	free(sol);

	return jump;	
}

/*
	MSVM2fw_init_gradient({ALPHA_IS_ZERO,ALPHA_NOT_ZERO}, gradient, cache, model)
*/
void MSVM2fw_init_gradient(const enum AlphaInitType alpha_init_type, double **gradient, double **H_alpha, double **H_tilde_alpha, const struct Model *model)
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
	(spawn by main computing thread MSVM2fw_train_thread)
	
*/

#ifdef _WIN32
void MSVM2fw_update_gradient_thread2(PTP_CALLBACK_INSTANCE instance,void *th_data)
{
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
				H_alpha_update_ik -= partial / (2.*model->C[y_i]);
			}
	        }
 	      gradient[i][k] += H_tilde_alpha_update_ik;
 	      H_alpha[i][k] += H_alpha_update_ik;
 	      H_tilde_alpha[i][k] += H_tilde_alpha_update_ik;
	    }
	    
	  }
	}
	
	//pthread_exit(EXIT_SUCCESS);
}
#endif 

void *MSVM2fw_update_gradient_thread(void *th_data)
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
				H_alpha_update_ik -= partial / (2.*model->C[y_i]);
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

/*
	Old-style non-threaded gradient update
	
*/
void MSVM2fw_update_gradient(double **gradient, double **H_alpha,double **H_tilde_alpha, struct TrainingCache *cache, const struct Model *model)
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
				H_alpha_update_ik -= partial  / (2.*model->C[y_i]);
			}
	        }
 	      gradient[i][k] += H_tilde_alpha_update_ik;
 	      H_alpha[i][k] += H_alpha_update_ik;
 	      H_tilde_alpha[i][k] += H_tilde_alpha_update_ik;
	    }
	    
	  }
	}
}

/*
	Same as WW_compute_H_delta()
	and LLW
*/
void MSVM2fw_compute_H_delta(double **delta, struct TrainingCache *cache, const struct Model *model)
{
	long i,k,j,y_i,l;
	const long Q = model->Q;
	const double Qd = (double)Q;
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
		partial = 0.0;

		for(l=1; l<=Q; l++)
		    partial -= delta[j][l];

		partial /= Qd;
		
		partial += delta[j][k];
		cache->H_delta[i][k] += partial * cache->K[i][cache->table_chunk[j]];
		}
	      }
	    }
	  }
}

int MSVM2fw_check_opt_sol(double **gradient, struct TrainingCache *cache, const struct Model *model)
{
	double gradient_alpha = 0.0; 
	double gradient_Gamma = 0.0;
	double gradient_diff = 0.0;
	long i,k,ind_pattern, y_i;
	const long Q = model->Q;
	const long chunk_size = cache->chunk_size;
	int jump = false;
	
	for(i=1; i<=chunk_size; i++)
	  {
	  ind_pattern = cache->table_chunk[i];	
	  y_i = model->y[ind_pattern]; 
	  for(k=1; k<=Q; k++)
	   if(k != y_i)
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
	 #else
	  if(gradient_diff < -small)
	 #endif
	    {
	    printf("\nInappropriate subset selection...\n");
	    printf("\ngradient^T (alpha - gamma): %lf", gradient_diff);
	    }
	
	  jump = true;
	  }

	cache->gradient_diff = gradient_diff;
	return jump;

}
/*
	Update model paramaters alpha_ik 
	Return the difference in number of SVs
*/
long MSVM2fw_compute_new_alpha(const double theta_opt, const struct TrainingCache *cache, struct Model *model)
{
	long i,k,l,ind_pattern;
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
	    {
	    if(k!=model->y[ind_pattern])
	    	{
	    	    cache->alpha_update[i][k] = theta_opt * ( cache->lp_sol[i][k] - model->alpha[ind_pattern][k] );
		    model->alpha[ind_pattern][k] += cache->alpha_update[i][k];
		    model->sum_all_alpha += cache->alpha_update[i][k];	   	
		    
	      	    if(model->alpha[ind_pattern][k] < 0.0) {
			cache->alpha_update[i][k] -= model->alpha[ind_pattern][k];
			model->alpha[ind_pattern][k] = 0.0;
		    }
		    else if(model->alpha[ind_pattern][k] != 0.0 ) {
	    	    	// Update nb_SV and activeset
	    		if(cache->activeset[ind_pattern] == 0) {
	    			cache->activeset[ind_pattern] = 1;  	
	    			nb_SV++;
	    		}
	            }
	            model->partial_average_alpha[ind_pattern] += model->alpha[ind_pattern][k];
	    	}
	    else 
	       	cache->alpha_update[i][k] = 0;	   
	    }
    	  model->partial_average_alpha[ind_pattern] /= Qd;  
    	  
    	  // Update RHS of constraints in LP
	  for(k=1; k<Q; k++)
	  {
            for(l=1; l<=Q; l++)
		 cache->lp_rhs[k] += cache->alpha_update[i][l];
	      cache->lp_rhs[k] -= Qd * cache->alpha_update[i][k];
	  }
	}
	return nb_SV;
}

void MSVM2fw_compute_delta(double **delta, const struct TrainingCache *cache, const struct Model *model)
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

double MSVM2fw_compute_theta_opt(double **delta, struct TrainingCache *cache, const struct Model *model)
{
	long i,k;
	const long chunk_size = cache->chunk_size;
	const long Q = model->Q;

	double theta_opt, denominator = 0.0;

	MSVM2fw_compute_H_delta(delta, cache,model);

	for(i=1; i<=chunk_size; i++) {
	  for(k=1; k<=Q; k++)
	       denominator += delta[i][k] * cache->H_delta[i][k];
	}
	
	if(denominator <= 0) {
		printf("\nWrong estimate of theta: denominator= %e\n", denominator);
		exit(0);
	}

	theta_opt = cache->gradient_diff / denominator;

	if(theta_opt > 1.0)
	  theta_opt = 1.0;
	
	return theta_opt;
}

