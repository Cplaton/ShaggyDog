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
/*  Name           : libtrainMSVM.c                                         */
/*  Version        : 2.0                                                    */
/*  Creation       : 06/20/08                                               */
/*  Last update    : 06/27/13                                               */
/*  Subject        : Training functions for MSVMpack                        */
/*  Algo.          : mostly wrapper functions                               */
/*  Author         : Fabien Lauer and Yann Guermeur fabien.lauer@loria.fr  */
/*--------------------------------------------------------------------------*/
 
#include "libtrainMSVM.h"
#include "libtrainMSVM_WW.h"
#include "libtrainMSVM_CS.h"
#include "libtrainMSVM_LLW.h"
#include "libtrainMSVM_2.h"
#include "libtrainMSVM_2fw.h"

#ifdef _WIN32
#include <inttypes.h>
#endif
pthread_mutex_t thread_data_mutex = PTHREAD_MUTEX_INITIALIZER;

#ifdef _WIN32
pthread_cond_t      monitor_cond = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t     monitor_mutex = PTHREAD_COND_INITIALIZER;

static uint64_t iterate48(unsigned short subi[3])
{
	const uint64_t a = UINT64_C(0x5DEECE66D);
	const unsigned c = 13;
	const uint64_t mask = UINT64_C(0xFFFFFFFFFFFF); // 48 bits

	uint64_t x = ((uint64_t)subi[0] << 32)
		| ((uint32_t)subi[1] << 16)
		| subi[2];

	x *= a;
	x += c;
	x &= mask;

	subi[0] = (x >> 32) & 0xFFFF;
	subi[1] = (x >> 16) & 0xFFFF;
	subi[2] = (x >> 0) & 0XFFFF;

	return x;
}

double erand48(unsigned short subi[3])
{
	uint64_t r = iterate48(subi);
	return ((double)r) / 281474976710655.;
}

long jrand48(unsigned short subi[3])
{
	return ((int64_t)iterate48(subi)) >> 16;
}

long nrand48(unsigned short subi[3])
{
	return iterate48(subi) >> 17;
}
#endif
/*
	Main training function

	status = MSVM_train(model, training_set, chunk_size, accuracy, cache_memory, nprocs, cache_size_in_mb, alpha0_file, model_tmp_file, log_file)
	
		use cache_memory = 0	to use the maximal amount of memory
		use nprocs = 0			to use all processors	
		use accuracy = 0 	  	for infinite training 
		use alpha0_file = NULL 	  for default initialization of alpha
		use model_tmp_file = NULL to deactivate periodic saving of model
		use log_file = NULL 	  to deactivate optimization log
		
	Return value:
		-1	: Error (infeasible initial alpha or unknown model type)
		0 	: Optimum found with desired accuracy level
		1	: Stopped after maximum number of iterations
		N > 1	: Stopped by user after N iterations
		
*/
long long MSVM_train(struct Model *model, struct Data *training_set, long chunk_size, const double accuracy, int cache_memory, int nprocs, char *alpha0_file, char *model_tmp_file, char *log_file)
{
	long long return_status = -1;
	
	FILE *fp;
	int t;
	pthread_t *threads; 	// threads id
	void *status; 			// for pthread_join
	int rc; 			// return code of pthread functions
	int feasible;
	pthread_t thread_monitor;	
	enum AlphaInitType alphaiszero;
	
	// Check number of threads
	if (nprocs <= 0 || nprocs > get_nprocessors())
		nprocs = get_nprocessors();
	
	threads = (pthread_t *)malloc(sizeof(pthread_t) * nprocs);
	
	printf("Using %d processor(s) for training.\n",nprocs);
	
	if(training_set != NULL) {
		// Initialize model for this particular training set		
		model_load_data(model, training_set);
		
		// Check that chunk_size >= Q for WW
		if(model->type == WW && chunk_size < model->Q) {
			chunk_size = model->Q;		
		}
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

	if(model->type == MSVM2 && model->algorithm == Rosen) {
		printf("Starting Rosen's algorithm to train an M-SVM^2...\n");
	}
	
	
	// Allocate memory for shared ressources
	double **gradient = matrix(model->nb_data, model->Q);
	double **H_alpha = matrix(model->nb_data, model->Q);
	double **H_tilde_alpha;
	if(model->type == MSVM2)
		H_tilde_alpha = matrix(model->nb_data, model->Q);
	double best_primal_upper_bound = HUGE_VAL;
	int *activeset = (int *)calloc(model->nb_data+1, sizeof(int));	
	double *lp_rhs;
	if(model->type != CS && model->algorithm == FrankWolfe)
		lp_rhs = (double*)calloc(model->Q, sizeof(double));

	// including the kernel cache:
	unsigned long long cache_size;	
	if(cache_memory <= 0) {
		// set to max: amount of physical memory minus 1 GB
		//cache_size = get_physical_memory() - GBYTE;
		#ifdef _WIN32
		cache_size = get_physical_memory() - (1 * GBYTE) - GBYTE;
		#else
		cache_size = get_physical_memory() - GBYTE;
		#endif
		
		if (cache_size <= 0)
			cache_size = GBYTE;	// or 1GB if failed to detect amount of memory
	}
	else {
		// or to the requested amount
	 	cache_size = (unsigned long long)cache_memory * MBYTES;
	}	 

	struct KernelCache kc;
	cache_size = kernel_initialize_cache(cache_size, &kc, model);
	//printf("cache size = %lld\n", cache_size);
	printf("Initialization... ");fflush(stdout);
		
	// Initialize alpha and b
	if(training_set != NULL)  { 		// (otherwise resume training)
		if(model->type == CS)
			CS_init_alpha(model, alpha0_file);  // use init_alpha_b(model, NULL); to initialize all alpha to 0
		else 
			init_alpha_b(model, alpha0_file);  // use init_alpha_b(model, NULL); to initialize all alpha to 0
	}
	
	if(model->type == MSVM2 && model->algorithm==Rosen) {
		feasible = MSVM_2_check_feasible_sol_init(model);
		if(feasible == true)
			printf("\nVector alpha is a feasible solution...\n");
		else {
			printf("\n Initial alpha_0 is not a feasible solution.\n");		
			return -1;
		}
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
	
	// Initialize gradient
	switch(model->type) {
		case WW:
			WW_init_gradient(alphaiszero, gradient, H_alpha, model);
			break;
		case CS:
			CS_init_gradient(alphaiszero, gradient, H_alpha, model);
			break;
		case LLW:
			LLW_init_gradient(alphaiszero, gradient, H_alpha, model);
			break;
		case MSVM2:
			if(model->algorithm == Rosen) 
				MSVM_2_init_gradient(alphaiszero, gradient, H_alpha, H_tilde_alpha, model);
			else
				MSVM2fw_init_gradient(alphaiszero, gradient, H_alpha,H_tilde_alpha, model);
			break;
		default:
			printf("MSVM_train: Unknown model type. \n");
			return_status = -1;
			break;	
	}
	printf("Done.\n"); // (Initialization... Done.)
	
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
	if(model->type == MSVM2)
		thread_data.H_tilde_alpha = H_tilde_alpha;
	thread_data.best_primal_upper_bound = &best_primal_upper_bound;
	thread_data.activeset = activeset;	
	thread_data.nb_SV = &nb_SV;
	if(model->type != CS && model->algorithm == FrankWolfe)
		thread_data.lp_rhs = lp_rhs;
	thread_data.logfile_ptr = fp;
	
	
	// Launch nprocs (= #CPUs) computing threads 
	int numthreads=nprocs;
	
	if(model->type == CS && model->nb_data >= 5000) {
		// Except for CS if working set selection is used
		// then retain 25% of CPUs for gradient updates
		numthreads = nprocs - nprocs/4 + 1;	
		if(numthreads > nprocs)
			numthreads=nprocs;
	}
	
	
	for(t=0;t<numthreads;t++) {
	
		pthread_mutex_lock(&thread_data_mutex); // Wait for thread_data to be read by previous thread
 		
		thread_data.thread_id = t;

		// Choose training function according to model type
		switch (model->type) {
		case WW:
			rc = pthread_create(&threads[t], NULL, WW_train_thread, (void *) &thread_data);	
			break;		
			
		case CS:
			rc = pthread_create(&threads[t], NULL, CS_train_thread, (void *) &thread_data);	
			break;		
			
		case LLW:
			rc = pthread_create(&threads[t], NULL, LLW_train_thread, (void *) &thread_data);
			break;		
			
		case MSVM2:
			if(model->algorithm == Rosen) {
				rc = pthread_create(&threads[t], NULL, MSVM_2_train_thread, (void *) &thread_data);	
			}
			else {
				rc = pthread_create(&threads[t], NULL, MSVM2fw_train_thread, (void *) &thread_data);
			}
			break;
			
		default:
			printf("MSVM_train: Unknown model type. \n");
			return_status = -1;
			break;	
		}
	}
	
	// Wait for threads to terminate
	for(t=0;t<numthreads;t++) {
		rc = pthread_join(threads[t],&status);

		// Check if optimum has been reached by this thread		
		if((long long)status == 0)
			return_status=0;		
	}
	
	// Cancel monitor thread
	rc = pthread_cancel(thread_monitor);
	rc = pthread_join(thread_monitor,&status);	
		
	// Close log file
	if(fp != NULL)
		fclose(fp);

	// Free memory
	free(gradient[1]);free(gradient);
	free(H_alpha[1]);free(H_alpha);
	if(model->type == MSVM2) {
		free(H_tilde_alpha[1]);
		free(H_tilde_alpha);
	}
	free(activeset);
	if(model->type != CS && model->algorithm == FrankWolfe)
		free(lp_rhs);	
	
	kernel_free_cache(&kc);
	free(threads);
	
	// Set return status
	if(model->iter == MSVM_TRAIN_MAXIT)
		return_status = 1; // max iterations.
	else if(return_status != 0)
		return_status = model->iter; // interrupted by user 
	
	return return_status;
}

/*

	K-fold Cross-validation with K=folds
	
	error_rate = MSVM_train_cv(model, training_set, folds, chunk_size, accuracy, cache_memory, nprocs, cache_size_in_mb, alpha0_file, model_tmp_file, log_file)	
		
		see MSVM_train() above for details on the arguments. 
		
	Return value: 	2-by-(K+1) array of error_rates
	
		error_rates[0] = [Overall training error rate, 
								training error_rate on fold 1, 
								...,
								training error_rate on fold K]

		error_rates[1] = [Cross-validation estimate of the error, 
								test error_rate on fold 1, 
								...,
								test error_rate on fold K]
								
		So the most important number (cv error estimate) is in error_rates[1][0]. 
*/

double **MSVM_train_cv(struct Model *model, struct Data *training_set, int folds, long chunk_size, const double accuracy, int cache_memory, int nprocs, char *log_file)
{

	/*
	
		Notes on the implementation:
	
		- The data subsets are created after a random permutation of the data set
		
		- K models are trained in parallel on different data subsets
		- Only nprocs threads are run at the same time: for instance, with 4 cpus,
			model 5 has to wait for model 1 to finnish before it can be trained. 			
		- If nprocs > K, each model is trained with (nprocs / K) local threads
				
		- A master kernel cache stores the rows of the complete kernel matrix 
			computed from the entire data set
		- Each one of the K models has a small local kernel cache 
		- The local caches simply store kernel values mapped from the master cache
		- Every local request for a new kernel row implies the computation of the 
			full kernel row in the master cache, which can then be used for another model.
		
		- The fraction of memory used for the master cache is set by
			fraction_master_vs_local_cache (defined just below)
			with a maximum such that the local caches can store 
			at least 10 chunks of rows for each processor. 
		
	*/
	

	double fraction_master_vs_local_cache = 0.9; // fraction of memory used for master cache

	FILE *fp;
	int t;
	pthread_t **threads; 	// threads id
	void *status; 			// for pthread_join
	int rc; 			// return code of pthread functions
	int feasible;
	pthread_t thread_monitor;	
	enum AlphaInitType alphaiszero;
	int nprocs_local, nprocs_available;
	int *numthreads;	
	
	// Check number of threads
	if (nprocs <= 0 || nprocs > get_nprocessors())
		nprocs = get_nprocessors();
	
	printf("Using %d processor(s) for the cross validation.\n",nprocs);
	nprocs_available = nprocs; 
	
	threads =  (pthread_t **)malloc(sizeof(pthread_t *) * (folds + 1));
	numthreads = (int *)malloc(sizeof(int) * (folds + 1));
	
	int cv;
	long i;
	long *nb_data_train = (long*)calloc(folds+1,sizeof(long));
	long *nb_data_test = (long*)calloc(folds+1,sizeof(long));	
	long j,k;
	long *labels;
	char modelcvfile[200];
	char outputsfile[200];	
	long *nerrors = (long*)calloc(folds+1,sizeof(long));
	long total_nerrors = 0;
	double **error_rates = (double **)malloc(sizeof(double *) * 2);
	error_rates[0] = (double *)calloc(folds+1, sizeof(double)); // Training error rates
	error_rates[1] = (double *)calloc(folds+1, sizeof(double)); // Test error rates
	
	double error_rate; 
	
	long foldsize = training_set->nb_data / folds;
	
		
	if(training_set == NULL) {
		return NULL; // no data error
	}

	// Initialize model for this training set
	model_load_data(model, training_set);

	// Check that chunk_size >= Q for WW
	if(model->type == WW && chunk_size < model->Q) {
		chunk_size = model->Q;		
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

	if(accuracy<=0) {
		printf("Cannot perform cross validation with infinite training (-a 0).\n");
		exit(0);
	}

	
	// Make data subsets structures
	struct Model **model_cv = (struct Model **)malloc(sizeof(struct Model *) * (folds + 1)); 
	struct Data **subset_train = (struct Data **)malloc(sizeof(struct Data *) * (folds+1));
	struct Data **subset_test = (struct Data **)malloc(sizeof(struct Data *) * (folds+1));
	
	for (cv = 1; cv <= folds ; cv++) {
		subset_train[cv] = MSVM_make_dataset(NULL, model->datatype);
		subset_train[cv]->name = (char *)malloc(sizeof(char) * 30);
		sprintf(subset_train[cv]->name, "k-fold subset_train %d",cv);
		subset_train[cv]->Q = model->Q;
		
		subset_test[cv] = MSVM_make_dataset(NULL, model->datatype);
		subset_test[cv]->Q = model->Q;
		 
		 
		if (cv < folds) {
			allocate_dataset(subset_test[cv], foldsize, training_set->dim) ;
			allocate_dataset(subset_train[cv], foldsize * (folds-2) + training_set->nb_data - (folds-1)*foldsize, training_set->dim) ;			
		}
		else  {
			allocate_dataset(subset_test[cv], training_set->nb_data - (folds-1)*foldsize, training_set->dim) ;
			allocate_dataset(subset_train[cv], (folds-1)*foldsize, training_set->dim) ;
		}
	}
	
	// Split data into folds 
	 
	// permutation[i] = original index of i-th example for the whole cross validation
	long *permutation = randperm(training_set->nb_data); 
	
	// maps is a matrix of indexes of size folds-by-max(training subset size)
	// maps[j][i] = original index of i-th example for j-th training subset of cross validation
	long **maps = matrix_l(folds, foldsize * (folds-2) + training_set->nb_data - (folds-1)*foldsize); 

	if(folds * foldsize == training_set->nb_data)
		printf("Creating %d folds of size %ld...\n",folds,foldsize);	
	else
		printf("Creating %d folds of size ~%ld...\n",folds,foldsize);
		
	for(i=1;i<=training_set->nb_data; i++) {
		
		// Choose fold to assign the ith data
		cv = 1 + ( (i-1) / foldsize );
		
		// Assign remaining data to the last fold (which can be larger than the others)
		if(cv > folds)
			cv = folds; 

		for (j=1;j<=folds;j++) {
			if(j==cv) {
				// set permutation[i] as a test point for the cv-th fold
				nb_data_test[j]++;
				copy_data_point(training_set, permutation[i], subset_test[j], nb_data_test[j]);
			}
			else {
				// or a training point for the others
				nb_data_train[j]++;
				copy_data_point(training_set, permutation[i], subset_train[j], nb_data_train[j]);
				
				maps[j][nb_data_train[j]] = permutation[i];
			}
		}
	}

	// Ressources for each fold in, e.g., gradient[fold], H_alpha[fold]... 
	double ***gradient = (double ***) malloc(sizeof(double **) * (folds + 1));
	double ***H_alpha = (double ***) malloc(sizeof(double **) * (folds + 1));
	double ***H_tilde_alpha;
	if(model->type == MSVM2) {
		 H_tilde_alpha = (double ***) malloc(sizeof(double **) * (folds + 1));
	}
	double *best_primal_upper_bound = (double *) malloc(sizeof(double) * (folds + 1));
	int **activeset = (int **)malloc(sizeof(int *) * (folds + 1));
	double **lp_rhs;
	if(model->type != CS && model->algorithm == FrankWolfe)
		lp_rhs = (double **) malloc(sizeof(double *) * (folds + 1));

	long *nb_SV = (long *) malloc(sizeof(long) * (folds + 1));

	// Use single thread_data for all threads across all folds 
	// (only used to pass arguments to computing threads)
	struct ThreadData thread_data;

	// Set cache_size
	unsigned long total_cache_size;	
	if(cache_memory <= 0) {
		// set to max: amount of available memory minus 1 GB
		// total_cache_size = get_physical_memory() - GBYTE;
		#ifdef _WIN32
		total_cache_size = get_physical_memory() - (1 * GBYTE) - GBYTE;
		#else
		total_cache_size = get_physical_memory() - GBYTE;
		#endif
		if (total_cache_size <= 0)
			total_cache_size = GBYTE;	// or 1GB if failed to detect amount of memory
	}
	else {
		// or to the requested amount
	 	total_cache_size = (unsigned long)cache_memory * MBYTES;
	}	 

	// Create an array of local kernel caches, one for each fold,
	// plus one master kernel cache in kc_array[0]
	struct KernelCache *kc_array = (struct KernelCache *)malloc(sizeof(struct KernelCache) * (folds + 1));
	unsigned long master_cache_size = (unsigned long) ((double)total_cache_size * fraction_master_vs_local_cache);
	
	// Leave enough memory for all local caches 
	// (so that each local cache can store at least 10*chunk_size*nthreads rows)
	if (total_cache_size - master_cache_size < nprocs * sizeof(double) * 10 * chunk_size * model->nb_data)
		master_cache_size = nprocs * 8 * 10 * chunk_size * model->nb_data; 
	
	// Initialize and allocate master kernel cache
	master_cache_size = kernel_initialize_cache(master_cache_size, &(kc_array[0]), model);

	// Set the size of local caches
	unsigned long local_cache_size, true_local_cache_size;
	if(nprocs > folds) {
		// All models trained simultaneously, the amount of memory must
		// be shared across the folds
		local_cache_size = (total_cache_size - master_cache_size) / folds;
	}
	else {
		// No more than nprocs models are trained simultaneously,
		// so we can allocate more memory for each fold in this case
		local_cache_size = (total_cache_size - master_cache_size) / nprocs;
	}		
	

	EVAL = 0;	// triggered by signal handler to call eval
	STOP = 0;	// triggered by user to stop training	

	// Initialize display
	printf("\n Fold  ");			
	if(model->type == MSVM2)
		printf("Iteration\t Number of Support Vectors\t Ratio dual/primal\n");
	else
		printf("Iteration\t Number of margin Sup. Vec.\t Ratio dual/primal\n");			
	printf("---------------------------------------------------------------------------\n");


	int cv_done_training = 0; // how many models are done training?

	// Start procedure
	for (cv = 1; cv <= folds ; cv++) {

		// Create a temporary model with the right parameters (C, kernel...)
		model_cv[cv] = MSVM_make_model_copy(model);		
 		if(model_cv[cv] == NULL) {
 			printf("Error in MSVM_train_cv(): cannot create model_cv.\n");
 			exit(0);
 		}
 		model_cv[cv]->crossvalidation = cv; 
 		 
 		// Initialize model for this particular training subset		
		model_load_data(model_cv[cv], subset_train[cv]);
		model_cv[cv]->iter = 1;  	// number of iterations (over all threads) 
		
	 	// Allocate memory for locally (fold-wise) shared ressources
		gradient[cv] = matrix(model_cv[cv]->nb_data, model->Q);
		H_alpha[cv] = matrix(model_cv[cv]->nb_data, model->Q);
		if(model->type == MSVM2)
			H_tilde_alpha[cv] = matrix(model_cv[cv]->nb_data, model->Q);
		best_primal_upper_bound[cv] = HUGE_VAL;
		activeset[cv] = (int *)calloc(model_cv[cv]->nb_data+1, sizeof(int));	
		if(model_cv[cv]->type != CS && model_cv[cv]->algorithm == FrankWolfe)
			lp_rhs[cv] = (double*)calloc(model_cv[cv]->Q, sizeof(double));
	
	 	// Local kernel cache 
		true_local_cache_size = kernel_initialize_cache(local_cache_size, &(kc_array[cv]), model_cv[cv]);
		kc_array[cv].master = & (kc_array[0]); // with a master cache
		kc_array[cv].master_model = model; 
		kc_array[cv].map =  maps[cv];
		
		
		//printf("Initialization for fold %d ... ",cv);fflush(stdout);
		
		// Initialize alpha and b
		if(model->type == CS)
			CS_init_alpha(model_cv[cv], NULL);
		else 
			init_alpha_b(model_cv[cv], NULL);
	
		alphaiszero = ALPHA_IS_ZERO;
		
		// Initialize gradient
		switch(model->type) {
			case WW:
				WW_init_gradient(alphaiszero, gradient[cv], H_alpha[cv], model_cv[cv]);
				break;
			case CS:
				CS_init_gradient(alphaiszero, gradient[cv], H_alpha[cv], model_cv[cv]);
				break;
			case LLW:
				LLW_init_gradient(alphaiszero, gradient[cv], H_alpha[cv], model_cv[cv]);
				break;
			case MSVM2:
				if(model->algorithm == Rosen) 
					MSVM_2_init_gradient(alphaiszero, gradient[cv], H_alpha[cv], H_tilde_alpha[cv], model_cv[cv]);
				else
					MSVM2fw_init_gradient(alphaiszero, gradient[cv], H_alpha[cv],H_tilde_alpha[cv], model_cv[cv]);
				break;
			default:
				printf("MSVM_train_cv: Unknown model type. \n");
				return NULL;
				break;	
		}
		//printf("Done.\n"); // (Initialization... Done.)
	
	
		// Prepare data for computing threads
		nb_SV[cv] = 0;
		
		if(nprocs > folds) {
			// Use nprocs / folds computing threads for each fold. 
			if(cv < folds)
				nprocs_local = nprocs / folds;
			else 
				nprocs_local = nprocs - (nprocs / folds) * (folds-1); // (plus remaining for the last fold)
		}
		else {
			nprocs_local = 1;
		}
		 
		pthread_mutex_lock(&thread_data_mutex); // Wait for thread_data to be read by previous thread
		
		thread_data.nprocs = nprocs_local;
		thread_data.model = model_cv[cv];
		thread_data.kernelcache = & (kc_array[cv]); // use local kernel cache
		thread_data.chunk_size = chunk_size;
		thread_data.accuracy = accuracy;
		thread_data.gradient = gradient[cv];
		thread_data.H_alpha = H_alpha[cv];
		if(model->type == MSVM2)
			thread_data.H_tilde_alpha = H_tilde_alpha[cv];
		thread_data.best_primal_upper_bound = &(best_primal_upper_bound[cv]);
		thread_data.activeset = activeset[cv];	
		thread_data.nb_SV = & (nb_SV[cv]);
		if(model->type != CS && model->algorithm == FrankWolfe)
			thread_data.lp_rhs = lp_rhs[cv];
		thread_data.logfile_ptr = fp;
		

		// Launch nprocs_local computing threads for current fold
		numthreads[cv]=nprocs_local;

		if(model->type == CS && model_cv[cv]->nb_data >= 5000) {
			// Except for CS if working set selection is used
			// then retain 25% of CPUs for gradient updates
			numthreads[cv] = nprocs_local - nprocs_local/4 + 1;	
			if(numthreads[cv] > nprocs_local)
				numthreads[cv]=nprocs_local;
		}

		threads[cv] =  (pthread_t *)malloc(sizeof(pthread_t) * numthreads[cv]);

		for(t=0;t<numthreads[cv];t++) {
			if(t>0)
				pthread_mutex_lock(&thread_data_mutex); // Wait for thread_data to be read by previous thread
	 		
			thread_data.thread_id = t;

			// Choose training function according to model type
			switch (model->type) {
			case WW:
				rc = pthread_create(&threads[cv][t], NULL, WW_train_thread, (void *) &thread_data);	
				break;		
			
			case CS:
				rc = pthread_create(&threads[cv][t], NULL, CS_train_thread, (void *) &thread_data);	
				break;		
			
			case LLW:
				rc = pthread_create(&threads[cv][t], NULL, LLW_train_thread, (void *) &thread_data);
				break;		
			
			case MSVM2:
				if(model->algorithm == Rosen) {
					rc = pthread_create(&threads[cv][t], NULL, MSVM_2_train_thread, (void *) &thread_data);	
				}
				else {
					rc = pthread_create(&threads[cv][t], NULL, MSVM2fw_train_thread, (void *) &thread_data);
				}
				break;
			
			default:
				printf("MSVM_train: Unknown model type. \n");
				return NULL;
				break;	
			}

			nprocs_available--;
		}	
		
		/* Wait for processors to become available
		 	before launching more threads if folds > nprocs
		 	
		 	When training on fold 1 is done, the test subset 1 is classified
		 	and statistics are computed. 
		 	Then, training on the next waiting fold can be launched.
		*/
		while(nprocs_available <= 0) {
			for(t=0;t<numthreads[cv_done_training+1];t++) {
				pthread_join(threads[cv_done_training+1][t],&status);				
				nprocs_available++;				
			}

			cv_done_training++;
						
			// Free local memory used for training
			free(gradient[cv_done_training][1]);free(gradient[cv_done_training]);
			free(H_alpha[cv_done_training][1]);free(H_alpha[cv_done_training]);
			if(model->type == MSVM2) {
				free(H_tilde_alpha[cv_done_training][1]);
				free(H_tilde_alpha[cv_done_training]);
			}
			free(activeset[cv_done_training]);
			if(model->type != CS && model->algorithm == FrankWolfe)
				free(lp_rhs[cv_done_training]);	
	
			kernel_free_cache(&(kc_array[cv_done_training]));
			
			free(threads[cv_done_training]);
			
			
			// Process results for this fold
			
			// Compute statistics 	
			error_rates[0][cv_done_training] = model_cv[cv_done_training]->training_error;
			error_rates[0][0] += model_cv[cv_done_training]->training_error * (double)subset_train[cv_done_training]->nb_data;
	
			// Test on a data subset (with all available processors) 
			labels = (long*)malloc(sizeof(long) * (subset_test[cv_done_training]->nb_data +1 ));
		
			sprintf(outputsfile,"cv.fold-%d.outputs",cv_done_training);
			MSVM_classify_set(labels, subset_test[cv_done_training]->X, subset_test[cv_done_training]->y, subset_test[cv_done_training]->nb_data, outputsfile,  model_cv[cv_done_training], nprocs_available);
	
			for(i = 1;i <= subset_test[cv_done_training]->nb_data; i++) {
				if (labels[i] != subset_test[cv_done_training]->y[i])
					nerrors[cv_done_training]++;	
			}
			free(labels);

			error_rates[1][cv_done_training] = (double)nerrors[cv_done_training] / (double)nb_data_test[cv_done_training];
			total_nerrors += nerrors[cv_done_training];

			// Save partial model to a temp file 
			sprintf(modelcvfile,"cv.fold-%d.model",cv_done_training);
			MSVM_save_model(model_cv[cv_done_training], modelcvfile);
	
			// delete temporary data
			MSVM_delete_model(model_cv[cv_done_training]);
			MSVM_delete_dataset(subset_train[cv_done_training]);
			MSVM_delete_dataset(subset_test[cv_done_training]);		
		}
	}

	/* Wait for all remaining threads to terminate
	    before processing the results sequentially.
	    
	   (This could possibly be made faster by processing 
	   the results of any fold as soon as it is done training. )
	*/
	for(cv = cv_done_training+1; cv <= folds; cv++) {
		
		for(t=0;t<numthreads[cv];t++) {
			rc = pthread_join(threads[cv][t],&status);			
		}
		
		// Free local memory used for training 
		free(gradient[cv][1]);free(gradient[cv]);
		free(H_alpha[cv][1]);free(H_alpha[cv]);
		if(model->type == MSVM2) {
			free(H_tilde_alpha[cv][1]);
			free(H_tilde_alpha[cv]);
		}
		free(activeset[cv]);
		if(model->type != CS && model->algorithm == FrankWolfe)
			free(lp_rhs[cv]);	

		kernel_free_cache(&(kc_array[cv]));
	
		free(threads[cv]);
	}


	// Free memory shared across folds
	free(threads);
	free(gradient);
	free(H_alpha);
	if(model->type == MSVM2) {
		 free(H_tilde_alpha);
	}
	free(best_primal_upper_bound);
	free(activeset);
	if(model->type != CS && model->algorithm == FrankWolfe)
		free(lp_rhs);

	free(nb_SV);

	kernel_free_cache(& (kc_array[0])); // free master cache
	free(kc_array);
	
	free(maps[1]);free(maps);
	


	/* 
		Get results of remaining folds 
	*/
	for(cv = cv_done_training+1; cv <= folds; cv++) {		
	
		// Compute statistics 	
		error_rates[0][cv] = model_cv[cv]->training_error;
		error_rates[0][0] += model_cv[cv]->training_error * (double)subset_train[cv]->nb_data;
		
		// Test on a data subset (with all processors) 
		labels = (long*)malloc(sizeof(long) * (subset_test[cv]->nb_data +1 ));
			
		sprintf(outputsfile,"cv.fold-%d.outputs",cv);
		MSVM_classify_set(labels, subset_test[cv]->X, subset_test[cv]->y, subset_test[cv]->nb_data, outputsfile,  model_cv[cv], nprocs);
		
		for(i = 1;i <= subset_test[cv]->nb_data; i++) {
			if (labels[i] != subset_test[cv]->y[i])
				nerrors[cv]++;	
		}
		free(labels);

		error_rates[1][cv] = (double)nerrors[cv] / (double)nb_data_test[cv];
		total_nerrors += nerrors[cv];

		// Save partial model to a temp file 
		sprintf(modelcvfile,"cv.fold-%d.model",cv);
		MSVM_save_model(model_cv[cv], modelcvfile);
		
		// delete temporary data
		MSVM_delete_model(model_cv[cv]);
		MSVM_delete_dataset(subset_train[cv]);
		MSVM_delete_dataset(subset_test[cv]);		
	}
	
	
	// Compute cross-validation estimate of the error:
	error_rate = (double) total_nerrors / (double)(training_set->nb_data); 
	error_rates[1][0] = error_rate;
	
	// Overall training error:
	error_rates[0][0] /= (double)(training_set->nb_data); 
	
	// Close log file
	if(fp != NULL)
		fclose(fp);
		
	// save permutation 
	fp = fopen("cv.permutation","w");
	if(fp != NULL) {
		for (i=1;i<=training_set->nb_data;i++)
			fprintf(fp, "%ld\n", permutation[i]); 
		
		fclose(fp);	
	}
	
	//save_outputs_file(permutation, subset_test); 
	
	
	// Free memory	
	free(numthreads);
	free(model_cv);
	free(subset_train);
	free(subset_test);
	
	free(permutation);
	free(nb_data_train);
	free(nb_data_test);
	free(nerrors);
		
	return error_rates;
}


/*
   Reorder computed outputs to obtain a pred.outputs 
	 file consistent with the data file. 
	
void save_outputs_file(long *permutation, struct Data **subset_test, int folds) {
	long i,j;
	
	FILE *fo, *fsub;
	fo = fopen ("cv.pred.outputs", "w");
	if(fo == NULL) {
		printf("Cannot save outputs file.\n");
		return; 
	}
	
}
*/


/*
	An alternative to MSVM_train_cv that uses less memory
	
	Only 1 training subset lies in memory at a given time, 
	and no test subset is stored in memory. 
*/
double **MSVM_train_cv_slowbutlessmemory(struct Model *model, struct Data *training_set, int folds, long chunk_size, const double accuracy, int cache_memory, int nprocs, char *alpha0_file, char *model_tmp_file, char *log_file)
{
	long return_status;
	
	// Check number of threads
	if (nprocs <= 0 || nprocs > get_nprocessors())
		nprocs = get_nprocessors();
	
	printf("Using %d processor(s) for training.\n",nprocs);
	
	
	int cv;
	struct Model *model_cv;
	struct Data *subset;
	long i;
	long *nb_data_train = (long*)calloc(folds+1,sizeof(long));
	long *nb_data_test = (long*)calloc(folds+1,sizeof(long));	
	long **table_cv = matrix_l(training_set->nb_data, folds);
	int j,label;
	char modelcvfile[200];
	long *nerrors = (long*)calloc(folds+1,sizeof(long));
	long total_nerrors = 0;
	double **error_rates = (double **)malloc(sizeof(double *) * 2);
	error_rates[0] = (double *)calloc(folds+1, sizeof(double)); // Training error rates
	error_rates[1] = (double *)calloc(folds+1, sizeof(double)); // Test error rates
	
	double error_rate; 

	/*
		table_cv contains the original indexes of the examples, 
		starting with the training ones and ending with the test ones. 
			
	For 1 <= i <= nb_data_train[j] :

		table_cv[j][i] = original index of the i-th training example for the j-th fold

	For 1 <= i < = nb_data_test[j] :

		table_cv[j][nb_data + 1 - i] = original index of the i-th test example for the j-th fold
		
	*/
	
	
	
	// Split data into folds 
	long *permutation = randperm(training_set->nb_data); 
	long foldsize = training_set->nb_data / folds;
	printf("Creating %d folds of size ~%ld...\n",folds,foldsize);
	for(i=1;i<=training_set->nb_data; i++) {
		
		// Choose fold to assign the ith data
		cv = 1 + ( (i-1) / foldsize );
		
		// Assign remaining data to the last fold (which can be larger than the others)
		if(cv > folds)
			cv = folds; 

		for (j=1;j<=folds;j++) {
			if(j==cv) {
				// set i as a test point for the cv-th fold
				table_cv[j][training_set->nb_data - nb_data_test[j]] = permutation[i];
				nb_data_test[j]++;
			}
			else {
				// or a training point for the others
				nb_data_train[j]++;
				table_cv[j][nb_data_train[j]] = permutation[i];
			}
		}
	}
	free(permutation);
	
	// Check number of data in each fold: 
	// (allow a 10% margin around nb_data/folds)
	for (cv = 1; cv <= folds ; cv++) {
		if((double) nb_data_test[cv] / (double)training_set->nb_data < (1. / (double)folds) - 0.1 
			|| (double) nb_data_test[cv] / (double)training_set->nb_data > (1. / (double)folds) + 0.1 ) {
			printf(" Unbalanced partitioning into %d folds. Stopping the procedure.\n", folds);
			for (cv = 1; cv <= folds ; cv++) {
				printf("fold %d: %ld data\n",cv,nb_data_test[cv]);				
			}
			exit(0);
		}  
	} 
 
	// Start procedure
	for (cv = 1; cv <= folds ; cv++) {

		printf("\nTesting fold %d...\n\n", cv);

		// Create a temporary model with the right parameters (C, kernel...)
		model_cv = MSVM_make_model_copy(model);
 		if(model_cv == NULL) {
 			printf("Error in MSVM_train_cv(): cannot create model_cv.\n");
 			exit(0);
 		}
		
		// Prepare data	
		subset = MSVM_make_dataset(NULL, model_cv->datatype);
		subset->name = (char *)malloc(sizeof(char) * 20);
		strcpy(subset->name, "k-fold subset");
		subset->Q = model->Q;
		allocate_dataset(subset, nb_data_train[cv], training_set->dim) ;
		
		for (i=1;i<= nb_data_train[cv]; i++) {
			// copy data (required due to memory alignment issues)
			copy_data_point(training_set, table_cv[cv][i], subset, i);
		}
		 
		// Launch training for the current fold
		return_status = MSVM_train(model_cv, subset, chunk_size, accuracy, cache_memory, nprocs, alpha0_file,model_tmp_file,log_file);
		
		if (return_status < 0) {
			printf("Error in MSVM_train() for fold %d, cannot proceed with cross validation\n",cv);
			exit(0);
		} 
		
		error_rates[0][cv] = model_cv->training_error;
		error_rates[0][0] += model_cv->training_error * (double)subset->nb_data;
		
		// Test on a data subset
		for(i=training_set->nb_data; i> (training_set->nb_data - nb_data_test[cv]); i--) {

			label = MSVM_classify(training_set->X[table_cv[cv][i]], model_cv, NULL);

			if (label != training_set->y[table_cv[cv][i]])
				nerrors[cv]++;			
		}

		error_rates[1][cv] = (double)nerrors[cv] / (double)nb_data_test[cv];
		total_nerrors += nerrors[cv];

		// Save partial model to a temp file 
		sprintf(modelcvfile,"cv.fold-%d.model",cv);
		MSVM_save_model(model_cv, modelcvfile);
		
		// delete temporary data
		MSVM_delete_model(model_cv);
		MSVM_delete_dataset(subset);
	}
	
	// Compute cross-validation estimate of the error:
	error_rate = (double) total_nerrors / (double)(training_set->nb_data); 
	error_rates[1][0] = error_rate;
	
	// Overall training error:
	error_rates[0][0] /= (double)(training_set->nb_data); 
	
	free(nb_data_train);
	free(nb_data_test);
	free(table_cv[1]); free(table_cv);
	free(nerrors);
		
	return error_rates;
} 

/* 
	Initializes the parameters and filenames used for training an M-SVM 
    	from a .com file.
*/
long MSVM_init_train_comfile(struct Model *model, char *com_file, char *training_file, char *alpha0_file, char *alpha_file, char *log_file) {

	long k,chunk_size;
	unsigned int kernel_type;
	FILE *fs;
	long nr = 0;
	
	if((fs=fopen(com_file, "r"))==NULL)
	  {
	  printf("\n.com file %s cannot be open.\n", com_file);
	  exit(0);
	  }

	nr += fscanf(fs, "%ld", &(model->Q));
	nr += fscanf(fs, "%d", &kernel_type);
	model->nature_kernel = kernel_type;
	set_default_kernel_par(model);
	
	model->C = (double*)realloc(model->C, sizeof(double) * (model->Q + 1));
	for(k=1;k<=model->Q;k++)
		nr += fscanf(fs, "%lf", &(model->C[k]));
	
	nr += fscanf(fs, "%ld", &chunk_size);
	nr += fscanf(fs, "%s", training_file);
	printf("\nThe file of the training data is: %s", training_file);
	nr += fscanf(fs, "%s", alpha0_file);
	nr += fscanf(fs, "%s", alpha_file);
	nr += fscanf(fs, "%s", log_file);	
	
	fclose(fs);
	if(nr < 5) 
		printf("\nError in reading com file %s, not enough data.\n", com_file);
	return chunk_size;
}

/*
	init_alpha_b(model_ptr, alpha_file)
	
	Initialize the alpha of the model pointed by model_ptr by
		reading them from the file alpha_file or 
		setting all alpha to 0 if alpha_file is NULL
		
	Also (re)allocate memory for the vector b 
*/
void init_alpha_b(struct Model *model, char *alpha_file) {
	long i,k;
	const long Q = model->Q;
	const long nb_data = model->nb_data;

	if(model->type == CS) {
		printf("Do not use init_alpha_b() with CS-type M-SVM:\n This M-SVM has no vector b and cannot initalize all alpha to 0.\n Use CS_init_alpha() instead.\n");
		exit(0);
	}
		
	// Initialize alpha
	if(alpha_file == NULL) {
		if (model->alpha != NULL) {
			free(model->alpha[1]); free(model->alpha);
		}
		model->alpha = matrix(nb_data, Q);
		model->partial_average_alpha = (double *) realloc(model->partial_average_alpha, sizeof(double) * (nb_data+1));	
		
		for(i=1; i<=nb_data; i++) {
		  model->partial_average_alpha[i] = 0.0;
		  for(k=1; k<=Q; k++)
		    model->alpha[i][k] = 0.0;
		}
		model->sum_all_alpha = 0.0;	
	}
	else
		read_alpha(model, alpha_file);
	
	// Initialize b
	model->b_SVM = (double *)realloc(model->b_SVM, (Q+1) *  sizeof(double));
		
	return;
}


/* 
	compute_K(cache,model)

	Compute the submatrix of the Kernel matrix corresponding to the chunk 
*/
void compute_K(struct TrainingCache *cache, const struct Model *model)
{
	long i;	
	const long chunk_size = cache->chunk_size;
		
	for(i=1; i<=chunk_size; i++) {
	  cache->K[i] = kernel_get_row(cache->table_chunk[i], cache->kc, model);
	}

}
/*
	Release the cache space reserved for the kernel submatrix corresponding to the chunk
*/
void release_K(struct TrainingCache *cache) {
	long i;
	const long chunk_size = cache->chunk_size;
	
	// Take mutex on in_use 
	pthread_mutex_lock(&(cache->kc->inuse_mutex));
		
	for (i=1;i<=chunk_size;i++) {
		cache->kc->in_use[cache->kc->rows_idx[cache->table_chunk[i]]]--;		
	}
	// Release mutex on in_use
	pthread_mutex_unlock(&(cache->kc->inuse_mutex));

}

/*
	Allocate memory and initialize mutexes for the kernel cache
	
	Returns the amount of memory truly allocated (in Bytes)
*/
unsigned long long kernel_initialize_cache(unsigned long long cache_size, struct KernelCache *kc, const struct Model *model) {
	
	long nb_data = model->nb_data;
	long i;

	unsigned long long size_row = sizeof(double) * (nb_data + 1);	// Size of a kernel row
	unsigned long long size_max = (size_row + 1) * (nb_data + 1); // Maximal necessary cache size
	
	
	if(cache_size < size_row)
		cache_size = 0;
		
	// Allign cache_size with size_row
	if(cache_size % size_row > 0)
		cache_size -= cache_size % size_row;
		
	// Limit cache_size to what is necessary
	if(cache_size > size_max)
		cache_size = size_max;
			
	// Allocate maximal cache memory
	kc->K = NULL;
	while (kc->K == NULL && cache_size>0) {
		kc->K = (double *)malloc(cache_size);
		if(kc->K == NULL) 
			cache_size -= size_row;
	}
	if(kc->K == NULL) {
		printf("no more memory, abording.\n");
		exit(1);
	}
	
	// How many rows allocated?
	long nrows = (cache_size/size_row) - 1; 
	
	// Pointers to rows 
	kc->rows_ptr = (double **)malloc(sizeof(double *) * (nrows + 1));
	for (i=0;i<=nrows;i++) {
		kc->rows_ptr[i] = NULL;	// NULL means row not available;
	}
	// Which cache_index for these rows?
	kc->rows_idx = (long *)calloc(nb_data+1, sizeof(long));
	kc->rows_idx_inv = (long *)calloc(nrows+1, sizeof(long));
	
	// Which rows are in use (and by how many threads)?
	kc->in_use = (int *)calloc(nrows+1, sizeof(int));
	
	if(kc->in_use == NULL) {
		printf("initializing kernel cache: not enough memory.\n");
		exit(1);
	}
	else {
		if(model->crossvalidation == 0) 
			printf("Allocated %llu MB of memory to cache %ld rows (%ld %%) of the kernel matrix.\n",cache_size/MBYTES + 1, nrows,100 * nrows/nb_data);
		/*else { 
			printf(" %lu MB for local cache of fold %d (%ld rows, %ld %% of the local kernel matrix).\n",cache_size/MBYTES + 1, model->crossvalidation, nrows,100 * nrows/nb_data);
		}*/
	}
	 
	kc->nrows = nrows;
	kc->max_idx = 1;	// first free row index
	kc->is_full = 0;
	
	kc->master = NULL; // Assume no master kernel 
	kc->master_model = NULL;
	kc->map = NULL;
		
	// Initialize mutexes and condition variables
	pthread_mutex_init(&(kc->inuse_mutex), NULL);
	kc->rowptr_mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t) * (nrows+1));
	kc->rowptr_cond = (pthread_cond_t *)malloc(sizeof(pthread_cond_t) * (nrows+1));	
	for(i=1;i<=nrows;i++) {
		pthread_mutex_init(&(kc->rowptr_mutex[i]), NULL);
		pthread_cond_init (&(kc->rowptr_cond[i]), NULL);
	}
	return cache_size;
}

void kernel_free_cache(struct KernelCache *kc) {
	free(kc->K);
	free(kc->rows_ptr);
	free(kc->rows_idx);
	free(kc->rows_idx_inv);	
	free(kc->in_use);
	long i;
	pthread_mutex_destroy(&(kc->inuse_mutex));
	for(i=1;i<=kc->nrows;i++) {
		pthread_mutex_destroy(&(kc->rowptr_mutex[i]));
		pthread_cond_destroy (&(kc->rowptr_cond[i]));
	}
	free(kc->rowptr_mutex);
	free(kc->rowptr_cond);
}

/*
	Fetch a pointer to the row in cache corresponding to X_index
*/
double *kernel_get_row(long index, struct KernelCache *kc, const struct Model *model) {
	long i=0;
	
	// Make sure no other thread modifies the in_use table during this check
	pthread_mutex_lock(&(kc->inuse_mutex));
	
	// is the row already available?
	if(kc->rows_idx[index] != 0  && kc->rows_ptr[kc->rows_idx[index]]  != NULL ) {
		
		// Yes, so mark it as "in use"
		kc->in_use[kc->rows_idx[index]]++;
		pthread_mutex_unlock(&(kc->inuse_mutex));
	
		// and return pointer to row data
		return kc->rows_ptr[kc->rows_idx[index]];
	}
	else if (kc->rows_idx[index] != 0  && kc->rows_ptr[kc->rows_idx[index]] == NULL) {
		// The row is being computed by another thread,
		// so mark it as "in use" by one more thread (this one)
		kc->in_use[kc->rows_idx[index]]++;				
		pthread_mutex_unlock(&(kc->inuse_mutex)); // do not block other threads while waiting

		// and wait for it to be ready		
		pthread_mutex_lock(&(kc->rowptr_mutex[kc->rows_idx[index]]));	
		while(kc->rows_ptr[kc->rows_idx[index]] == NULL) {
			pthread_cond_wait(&(kc->rowptr_cond[kc->rows_idx[index]]), &(kc->rowptr_mutex[kc->rows_idx[index]]));
		}
		pthread_mutex_unlock(&(kc->rowptr_mutex[kc->rows_idx[index]]));
		
		if(kc->rows_ptr[kc->rows_idx[index]] != NULL) {
			// return pointer to the computed row in cache
			return kc->rows_ptr[kc->rows_idx[index]];
		}
		else {
			printf("Error in kernel_get_row: row not really available.\n");
			printf("row_idx = %ld, index = %ld\n",kc->rows_idx[index], index);
			exit(1);
		}
	}
	
	// Row is not currently available, so compute kernel row
	if(kc->is_full) {
		// Cache already full, so search for the first row not currently in use
		i = 1;	
		while(i<=kc->nrows && kc->in_use[i] > 0) {
			i++;
		}
		if(i > kc->nrows) {
			printf("Not enough memory for kernel caching. Reduce the number of CPUs or increase kernel cache memory.\n");
			exit(0);
		}		
	}
	else {
		// Append row at the end of cache
		i = kc->max_idx;
		kc->max_idx++;
		if(kc->max_idx > kc->nrows){
			kc->is_full = 1;			
		}
	}
	
	kc->in_use[i]++;		// Mark row as in use
	kc->rows_ptr[i] = NULL;		// but as not available (going to overwrite it)	
	kc->rows_idx[index] = i;	// and currently computing
	kc->rows_idx[kc->rows_idx_inv[i]] = 0;	// mark previously cached row as unavailable
	
	// Release mutex on in_use
	pthread_mutex_unlock(&(kc->inuse_mutex));
	
	// Compute and cache the kernel row for X_index at location i in the kernel cache
	compute_kernel_row(index, i, kc, model);		// do the computation

	pthread_mutex_lock(&(kc->rowptr_mutex[i]));	// 
	pthread_cond_broadcast(&(kc->rowptr_cond[i]));	// signal that the row is ready
	pthread_mutex_unlock(&(kc->rowptr_mutex[i]));	// and unlock other waiting threads

	// Return the row_ptr to the row just computed
	return kc->rows_ptr[i];		

} 

/*
	Compute index-th kernel row and store it at position cache_index in cache kc
	
	If kc is a local cache, it has a master cache and the row is computed 
	by mapping the elements of the row in the master cache 
	(selection and permutation of columns). 
*/
void compute_kernel_row(const long index, const long cache_index, struct KernelCache *kc, const struct Model *model) {
	const long nb_data = model->nb_data;
	const long nature_kernel = model->nature_kernel;
	const long dim_input = model->dim_input;
	const double *kernel_par = model->kernel_par;
	double *row_ptr = &(kc->K[(cache_index-1)*(nb_data+1)]); // base address of row in cache memory
	long j;
	double *k = row_ptr + 1;
	
	double *full_row;

	if(kc->master == NULL) {
		// No master to get the row from, so compute row
		for(j=1; j<=nb_data; j++) {
		 	*k++ = ker(nature_kernel, model->X[index], model->X[j], dim_input, kernel_par);	 		
		}
	
		if(model->type == MSVM2) {
		  	row_ptr[index] += 1. /(2 * model->C[model->y[index]]);
		}
	}
	else {
		// Get the full row from the master cache 
		full_row = kernel_get_row(kc->map[index], kc->master, kc->master_model) ;
		
		// Copy the elements of the full row to the local row
		for(j=1; j<=nb_data; j++) {
		 	*k++ = full_row[kc->map[j]];
		}
		
		// Release row in master cache (we'll work with a copy from here)
		
		// Take mutex on in_use of master kc
		pthread_mutex_lock(&(kc->master->inuse_mutex));
		// decrement in_use
		kc->master->in_use[kc->master->rows_idx[kc->map[index]]]--;
		// Release mutex on in_use
		pthread_mutex_unlock(&(kc->master->inuse_mutex));

	}
	
	// Mark row as available 	
	kc->rows_idx_inv[cache_index] = index;
	kc->rows_ptr[cache_index] = row_ptr;
}


/*
	Switch working load between threads 
	from main (kernel) computations to gradient updates
	
	As as result, the number of threads used for gradient updates increases
	while main computing threads (XX_train_thread) are terminated

	returns 1 if the calling main thread must stop 
*/
int switch_thread(const int nprocs, int *numthreads_grad, int *next_numthreads_grad, long *percentage, int *percentage_step, struct ThreadGradient_data *grad_data, int thread_id, long nb_data) {	

	int k;

	// Keep (nprocs - next_numthreads_grad + 1) main computing threads
	// for kernel evaluations	
	if(thread_id < nprocs - *next_numthreads_grad + 1){
	
	
		//printf("*** Reached  %f %% of kernel rows, thread %d continues with %d subthreads for gradient updates (previsouly %d) ***\n ", 100.*(float)(*percentage) / (float)nb_data, thread_id, *next_numthreads_grad,*numthreads_grad);
		
		
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
			*percentage =  nb_data / 2; 		// 50% of kernel matrix
			*next_numthreads_grad = nprocs/2;	// -> 50% of CPUs for gradient
			break;
		case 2:
			*percentage = 3 * nb_data / 4; 		// 75%
			*next_numthreads_grad = 3*nprocs/4;	// -> 3/4 of CPUs
			break;
		case 3:
			*percentage = 19 * nb_data / 20; 	// 95%
			*next_numthreads_grad = nprocs-1;	// -> all CPUs - 1
			break;
		case 4:
			*percentage = nb_data;			// 100% (entire kernel matrix in memory)
			*next_numthreads_grad = nprocs;		// -> all CPUs for gradient updates
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

/*
	Randomly select a chunk of data (working set)
	
	At each training step, only the variables alpha_ik 
	corresponding to this chunk are optimized. 
*/
void select_random_chunk(struct TrainingCache *cache, const struct Model *model)
{
	long i,j,random_num,row;
	const long nb_data = model->nb_data;
	const long chunk_size = cache->chunk_size;
	
	for(i=1; i<=nb_data; i++)
	  {
	  cache->out_of_chunk[i] = i;
	  cache->in_chunk[i] = 0;	  
	  }
	  
	for(i=1; i<=chunk_size; i++)
	  {
	  random_num = nrand48(xi);
	  row = (random_num % (nb_data-i+1))+1;
	  cache->table_chunk[i] = cache->out_of_chunk[row];
	  for(j=row; j<=nb_data-i; j++)
	    cache->out_of_chunk[j] = cache->out_of_chunk[j+1];
	  cache->in_chunk[cache->table_chunk[i]] = 1;
	  }
	  
	  return;
}

/*
	Random permutation of an array of indexes from 1 to N

	using  Fisher and Yates (or Knuth) shuffle 
*/
long *randperm(long N) {
	long *array = (long*)malloc(sizeof(long) * (N + 1));
	long i,j,k;
	// init
	for(i=0;i<=N;i++)
		array[i] = i;
		
	// shuffle
	for(i=N;i>1;i--) {
		j = rand() % i + 1;
		k = array[j];
		array[j] = array[i];
		array[i] = k;
	}
	
	return array;
} 
 

/* 
	Write a vector/matrix to a file (used for the vector alpha) 
*/
void write_vector(double **vector, const long nb_data, const long Q, char *file)

{
	long i,k;
	FILE *fc;
	int rc;
	 
	char command[taille];
	
	if((fc=fopen(file, "w"))==NULL)
	  {
	  printf("\nThe file %s cannot be open.\n", file);
	  exit(0);
	  }

	for(i=1; i<=nb_data; i++)
	  for(k=1; k<=Q; k++)
	    fprintf(fc, "%15.10lf %c", vector[i][k], (k==Q) ? '\n': ' ');

	fclose(fc);	
}

 
/*
	Monitor the model while training:
	Save model in a temporary file
	on demand (do_save=1) and 
	every 'period' seconds at most.
*/

/*void usleep(__int64 usec) 
{ 
    HANDLE timer; 
    LARGE_INTEGER ft; 

    ft.QuadPart = -(10*usec); // Convert to 100 nanosecond interval, negative value indicates relative time

    timer = CreateWaitableTimer(NULL, TRUE, NULL); 
    SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0); 
    WaitForSingleObject(timer, INFINITE); 
    CloseHandle(timer); 
}*/


void *MSVM_monitor_model_thread(void *monitor_data) {
	struct MonitorData * md = (struct MonitorData *)monitor_data;
	char *model_tmp_file = md->model_tmp_file;
	int period = md->period;	
	struct Model * model = md->model;

	long nSV = 0;

	if ( model_tmp_file == NULL) {
		pthread_exit(EXIT_SUCCESS);
	}
#ifdef _WIN32	
	struct timespec   ts;
	struct timeval    tp;
	gettimeofday(&tp, NULL);
	/* Convert from timeval to timespec */
    ts.tv_sec  = tp.tv_sec;
    ts.tv_nsec = tp.tv_usec * 1000;
    ts.tv_sec += period;
	
	int old_type;
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,&old_type);
#endif
	
	while(!STOP) {
		// Wait for period
#ifdef _WIN32
		//Sleep(100 * period);
		pthread_mutex_lock(&monitor_mutex);
		pthread_cond_timedwait(&monitor_cond, &monitor_mutex, &ts);
		pthread_mutex_unlock(&monitor_mutex);
		
		gettimeofday(&tp, NULL);
		/* Convert from timeval to timespec */
		ts.tv_sec  = tp.tv_sec;
		ts.tv_nsec = tp.tv_usec * 1000;
		ts.tv_sec += period;
#else
		sleep (period) ;	
#endif		
		// Block training and updates of alpha
		// while writing model
#ifdef _WIN32		
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
#endif		
		pthread_mutex_lock(&(model->mutex)); 
		// Save model
		nSV = MSVM_save_model(model, model_tmp_file);
		//printf("Temporary model saved in %s\n",model_tmp_file);
		
		// Release training
		pthread_mutex_unlock(&(model->mutex));	
#ifdef _WIN32
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
		pthread_testcancel();
#endif
	}
	pthread_exit(EXIT_SUCCESS);
}

/*
	Print info during training
*/
void print_training_info(const long nb_SV, const struct Model *model) {
	char iter_str[20], nb_SV_str[20], fold_str[20];
	int i,num_spaces;
	
	const long nb_iter = model->iter; 
	const double ratio = model->ratio; 
	
	if(nb_iter == 0)  {
		if(model->crossvalidation > 0) 
			printf("\n Fold  ");			
		else 
			printf("\n ");
		
		if(model->type == MSVM2)
			printf("Iteration\t Number of Support Vectors\t Ratio dual/primal\n");
		else
			printf("Iteration\t Number of margin Sup. Vec.\t Ratio dual/primal\n");
			
		if(model->crossvalidation > 0) 
			printf("------------------------------------------------------------------------\n");
		else
			printf("-------------------------------------------------------------------\n");
	} 
		
	if(model->crossvalidation > 0) {
		sprintf(fold_str, " %d",model->crossvalidation);
		num_spaces = 5 - strlen(fold_str);
		for(i=0;i<num_spaces;i++)
			strcat(fold_str," "); 
	}	
	else 
		sprintf(fold_str, "");
		
	sprintf(iter_str, "%ld",nb_iter);
	num_spaces = (TRAIN_STEP/TRAIN_SMALL_STEP) - strlen(iter_str) - 1;
	for(i=0;i<num_spaces;i++)
		strcat(iter_str," "); 
		
	if(strlen(iter_str) < 7)
		strcat(iter_str, "\t\t");
	else if (strlen(iter_str) < 15)
		strcat(iter_str, "\t");

	sprintf(nb_SV_str, "%ld",nb_SV);
	if(strlen(nb_SV_str) < 5)
		sprintf(nb_SV_str, " %ld\t\t\t\t",nb_SV);
	else if (strlen(nb_SV_str) < 13)
		sprintf(nb_SV_str, " %ld\t\t\t",nb_SV);

	if(ratio == 0 || ratio == 1.0)
		printf("\r %s %s  %s  %1.0lf %%\n",fold_str, iter_str, nb_SV_str, 100*ratio);
	else if(ratio < 0.995 && ratio > 0.0001)
		printf("\r %s %s  %s  %3.2lf %%\n",fold_str, iter_str, nb_SV_str, 100*ratio);
	else if(ratio < 0.999999 && ratio > 0.000001)
		printf("\r %s %s  %s  %3.4lf %%\n",fold_str, iter_str, nb_SV_str, 100*ratio);
	else
		printf("\r %s %s  %s  %3.8lf %%\n",fold_str, iter_str, nb_SV_str, 100*ratio);
}

