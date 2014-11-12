/* Copyright 2010 Fabien Lauer                                              */

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
/*  Name           : trainMSVM.c                                            */
/*  Version        : 2.0                                                    */
/*  Creation       : 04/27/10                                               */
/*  Last update    : 02/25/12                                               */
/*  Subject        : MSVMpack: a multi-class SVM package		    */
/*  Algo.          : Training command-line tool			            */
/*  Author         : Fabien Lauer fabien.lauer@loria.fr                     */
/*--------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <time.h>

#include "libMSVM.h"
#include "libevalMSVM.h"
#include "libtrainMSVM.h"

/*
	Handling user interruption
*/ 
#ifdef _WIN32
BOOL ctrlC_handler(DWORD fdwCtrlType) {	
	switch (fdwCtrlType)
	{
		/* handle the CTRL-C signal */
	case CTRL_C_EVENT:
		//printf("CTRL-C event\n");
		if (EVAL) {
			printf(" Double Ctrl-C causes the program to break \n");
			printf("   (all is lost, nothing is saved). \n");
			exit(0);
		}
		else
			EVAL = 1;
		return TRUE;
	default:
		return FALSE;
	}
}
#else
void ctrlC_handler(int sig) {	
	if(EVAL) {
		printf(" Double Ctrl-C causes the program to break \n");
		printf("   (all is lost, nothing is saved). \n"); 		
		exit(0);
	}
	else
		EVAL = 1;
}
#endif

/*
	Print time info into a string
*/
char *print_time(time_t realtime, clock_t cputime) {
	char *timeinfo = (char*)malloc(sizeof(char) * 128);
	long unsigned int rt = (long unsigned int)realtime;
	
	if (rt <= 1) 
		sprintf(timeinfo, "%lu second (actual CPU time over all threads = %1.2lf sec.).", rt, (double) cputime / (double)CLOCKS_PER_SEC);
	else if ((long unsigned int)realtime < 120) 
		sprintf(timeinfo, "%lu seconds (actual CPU time over all processors = %1.2lf sec.).", rt, (double) cputime / (double)CLOCKS_PER_SEC);	
	else if ((long unsigned int)realtime < 3600) 
		sprintf(timeinfo, "%lum %lus (actual CPU time over all processors = %1.2lf sec.).", rt/60, rt%60, (double) cputime / (double)CLOCKS_PER_SEC);	
	else 
		sprintf(timeinfo, "%luh %lum %lus (actual CPU time over all processors = %1.2lf sec.).", rt/3600, (rt%3600)/60, (rt%3600)%60, (double) cputime / (double)CLOCKS_PER_SEC);
		
	return timeinfo;
}

/*
	Main
*/
int main(int argc, char *argv[]) {

	// Install the handler of SIGINT (Ctrl+C) signal, 
	// which will force the call to MSVM_eval on demand
#ifdef _WIN32
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)ctrlC_handler, TRUE);
#else	
	struct sigaction newhandler,oldhandler;
	memset(&newhandler, 0, sizeof(newhandler));
	newhandler.sa_handler = &ctrlC_handler;
	sigaction(SIGINT, &newhandler, &oldhandler);
#endif
	// Some local variables
	char com_file[taille], training_file[taille], model_file[taille], alpha0_file[taille], model_tmp_file[taille], alpha_file[taille], test_file[taille], outputs_file[taille], log_file[taille], yesno;
	
	long long training_status;
	int i, k, n_kernel_par;
	long Q;
	FILE *fstdout=NULL;
	FILE *fkp;
	time_t realtime,starttime = time(NULL);	
	clock_t cputime,startcputime = clock();	
	double **cv_error_rates;
	
	// Default parameter values
	enum MSVM_type model_type = MSVM2;
	double C = DEFAULT_C;
	int do_crossvalidation = false;
	int nFolds = 0;
	double accuracy = 0.98;			// 98% optimization accuracy level
	long chunk_size = 4;			// 4 points considered at each iteration
	int sparse_model_format = false;	// Do not use sparse model file format
										// this allows retraining from an existing model
	int retrain = false;			// Assumes an empty model
	int do_normalize = 0;			// Test if normalization needed
	int cache_memory = 0;			// Use max amount of available memory for kernel cache 
	int nprocs = get_nprocessors(); // Use all processors
	
	// Default filenames
	strcpy(model_file, "msvm.model");	// save model to "msvm.model"
	char *alpha0 = NULL;	// initialize with all alpha = 0 (except for CS-type)
	char *alpha = NULL;		// backward compatibility
	char *test = NULL;		// Do not test model on a test set (training only) 
	char *outputs = NULL;	// so no output on test set to record 
	char *log = NULL;		// Do not log the values of dual objective and upper bounds
	strcpy(log_file, "");
	
	
	/*
		Parse command line and initialize model for training
	*/
	if(argc<2 || check_argv(argc,argv, "-help") || check_argv_eq(argc,argv, "-h") )  {	
		printf("This is MSVMpack: a Multi-class Support Vector Machine package\n");
		printf("-- Version: %1.1f \n\n",VERSION);
		printf("Training tool used to train an M-SVM model from data. \n\n");
		printf("Usage: \n");
		printf("	trainmsvm training_file [file.model] [params]\n");
		printf(" or	trainmsvm comfile.com [file.model]\n\n");
		
		printf(" where 'params' is a list of parameters specified as e.g.: \n	-c 2.5 -k 1 \n\n");

		printf("Optional parameters: \n");		
		printf(" -m	: model type (WW,CS,LLW,MSVM2) (default is MSVM2)\n");
		printf(" -k	: nature of the kernel function (default is 1)\n");
		printf(" 	  1 -> linear kernel       k(x,z) = x^T z\n");
		printf(" 	  2 -> Gaussian RBF kernel k(x,z) = exp(-||x - z||^2 / 2*par^2)\n");
		printf(" 	  3 -> homogeneous polynomial kernel k(x,z) = (x^T z)^par \n");
		printf(" 	  4 -> non-homo. polynomial kernel k(x,z) = (x^T z + 1)^par \n");		
		printf(" 	  5 -> custom kernel 1\n");
		printf(" 	  6 -> custom kernel 2\n");
		printf(" 	  7 -> custom kernel 3\n");
		printf(" -p	: kernel parameter par (default is sqrt(5*dim(x)) (RBF) or 2 (POLY))\n");	
		printf(" -P     : list of space-separated kernel parameters starting with #parameters\n");
		printf(" -c	: soft-margin (or trade-off) hyperparameter (default C = 10.0)\n");
		printf(" -C	: list of space-separated hyperparameters C_k (default C_k = C)\n");		
		printf(" -cv k	: perform k-fold cross validation\n");		
		printf(" -n	: use normalized data for training (mean=0 and std=1 for all features)\n"); 
		printf(" -u	: use unnormalized data for training (bypass the normalization test) \n"); 
		printf(" -x	: maximal cache memory size in MB (default is max available)\n");
		printf(" -t	: number of working threads in [1, %d] (default is #CPUs=%d)\n",nprocs,nprocs);
		printf(" -f	: faster computations with single-precision floating-point data format\n");
		printf(" -I	: faster computations with integer data format\n");		
		printf(" -i	: faster computations with short integer data format\n");
		printf(" -B	: faster computations with byte data format\n");	
		printf(" -b	: faster computations with bit data format\n");	
		printf(" -o	: optimization method: 0 -> Franke-Wolfe (default), 1 -> Rosen\n");
		printf(" -w	: size of the chunk (default is 10 for WW and 4 for others)\n");
		printf(" -a	: optimization accuracy within [0,1] (default is 0.98)\n");
		printf(" -r	: retrain model (resume training)\n");
		printf(" -s	: save model using sparse format\n");
		printf(" -S	: convert existing model to sparse format without training\n");
		printf(" -q	: be quiet\n\n");
		
		printf("These optional files can be specified on the command line:\n");
		printf(" file.test	: Test data file\n");
		printf(" file.outputs	: Computed outputs on the test set\n");
		printf(" file.init	: Initialization file for alpha\n");
		printf(" file.alpha	: Save alpha in matrix format\n");
		printf(" file.log	: Log optimization information\n\n");	
		
		printf("See also 'predmsvm' to classify new data with a trained M-SVM. \n\n");
			
		exit(0);
	}

	i = check_argv_eq(argc,argv, "-q");
	if(i) {
		fstdout = freopen(".trainmsvm_quiet.log", "w", stdout);
		if(fstdout == NULL)
			printf("Cannot set quiet mode (error in stdout redirection)\n");
	}
	
	i = check_argv_eq(argc,argv, "-m");
	if(i) 
		model_type = str2model_type(argv[i+1]);
		
	// Make model
	struct Model *model = MSVM_make_model(model_type);
	if (model == NULL) {
		printf("Unknown model type -m %s.\n",argv[i+1]);
		exit(1);
	}
	// Set a different default chunk_size for WW models
	if(model_type == WW)
		chunk_size = 10;
	
	// check if first argument is a .com file
	if(strstr(argv[1], ".com") != NULL ) {
		// Read all info from the .com file
		strcpy(com_file, argv[1]);
		printf("Reading parameters from %s ...\n ", com_file);		
		chunk_size = MSVM_init_train_comfile(model, com_file, training_file, alpha0_file, alpha_file, log_file);
		alpha0 = alpha0_file;	
		alpha = alpha_file;		
	}	
	else {
		// First argument must be the training data file (if not a .com file)
		strcpy(training_file, argv[1]);
			
		// If a .com file is found at another position, use it to set the parameters only
		i = check_argv(argc,argv, ".com");
		if(i) {
			strcpy(com_file, argv[i]);
			chunk_size = MSVM_init_model(model, com_file);
		}		
		
	}
	// Parse filenames				
	i = check_argv(argc,argv, ".outputs");
	if(i) {
		strcpy(outputs_file,argv[i]);
		outputs = outputs_file;
	}
		
	i = check_argv(argc,argv, ".test");
	if(i) {			
		strcpy(test_file, argv[i]);
		test = test_file;
	}
	
	i = check_argv(argc,argv, ".alpha");
	if(i) {		
		strcpy(alpha_file,argv[i]);
		alpha = alpha_file;
	}

	i = check_argv(argc,argv, ".init");
	if(i) {		
		strcpy(alpha0_file,argv[i]);
		alpha0 = alpha0_file;
	}
	
	i = check_argv(argc,argv, ".model");
	if(i) {		
		strcpy(model_file,argv[i]);
	}

	i = check_argv(argc,argv, ".log");
	if(i) {		
		strcpy(log_file,argv[i]);
		log = log_file;
	}
	else if(strlen(log_file) > 0)
		log = log_file;

	// Parse parameters from the command line 
	// (this overrides values from a .com file)
	
	i = check_argv_eq(argc,argv, "-w");
	if(i)
		chunk_size = atoi(argv[i+1]);
		
	i = check_argv_eq(argc,argv, "-c");
	if(i)
		C = atof(argv[i+1]);	// see below for multiple C

	i = check_argv_eq(argc,argv, "-k");
	if(i) {
		model->nature_kernel = atoi(argv[i+1]);		
	}
	
	i = check_argv_eq(argc,argv, "-f");
	if(i) {
		model->datatype = DATATYPE_FLOAT;	
		model->nature_kernel += 10;
	}
	i = check_argv_eq(argc,argv, "-I");
	if(i) {
		model->datatype = DATATYPE_INT;
		model->nature_kernel += 20;
		do_normalize = 0;
	}
	i = check_argv_eq(argc,argv, "-i");
	if(i) {
		model->datatype = DATATYPE_SHORT;
		model->nature_kernel += 30;
		do_normalize = 0;
	}
	i = check_argv_eq(argc,argv, "-B");
	if(i) {
		model->datatype = DATATYPE_BYTE;
		model->nature_kernel += 40;
		do_normalize = 0;
	}
	i = check_argv_eq(argc,argv, "-b");
	if(i) {
		model->datatype = DATATYPE_BIT;
		model->nature_kernel += 50;
		do_normalize = 0;
	}

	
	i = check_argv_eq(argc,argv, "-p");
	if(i) {
		model->kernel_par = (double*)realloc(model->kernel_par, sizeof(double) * 2);
		model->kernel_par[0] = 1;
		model->kernel_par[1] = atof(argv[i+1]);
	}
	
	i = check_argv_eq(argc,argv, "-P");
	if(i) {
		if(argv[i+1][0] >= '0' && argv[i+1][0] <= '9') {
			n_kernel_par = atoi(argv[i+1]);
			model->kernel_par = (double*)realloc(model->kernel_par, sizeof(double) * (n_kernel_par + 1));
			model->kernel_par[0] = n_kernel_par;
			if(n_kernel_par > 0 && argv[i+2][0] >= '0' && argv[i+2][0] <= '9') {
				for(k=1;k<=n_kernel_par;k++)
					model->kernel_par[k] = atof(argv[i+1+k]);
			}
			else if (n_kernel_par > 0){
				// Kernel parameter values in file 
				fkp = fopen(argv[i+2], "r");
				if(fkp == NULL) {
					printf("Cannot read kernel parameter file %s\n",argv[i+2]);
					exit(1);
				}
				for(k=1;k<=n_kernel_par;k++)
					i=fscanf(fkp, "%lf", &model->kernel_par[k]);
				fclose(fkp);
			}
			
		}
		else {
			// Kernel parameter list is a file name
			fkp = fopen(argv[i+1], "r");
			if(fkp == NULL) {
				printf("Cannot read kernel parameter file %s\n",argv[i+1]);
				exit(1);
			}
			i=fscanf(fkp, "%d", &n_kernel_par);
			model->kernel_par = (double*)realloc(model->kernel_par, sizeof(double) * (n_kernel_par + 1));
			model->kernel_par[0] = n_kernel_par;
			for(k=1;k<=n_kernel_par;k++)
				i=fscanf(fkp, "%lf", &model->kernel_par[k]);
			fclose(fkp);
		}
	}
	
	i = check_argv_eq(argc,argv, "-a");
	if(i) {
		accuracy = atof(argv[i+1]);	
		if (accuracy < 0 || accuracy > 1) {
			printf("\n Wrong accuracy level -a %lf (not in [0,1])",accuracy);
			printf("\n Falling back to default (0.98).\n");
			accuracy = 0.98;
		}
	}
	
	i = check_argv_eq(argc,argv, "-n");
	if(i)
		do_normalize = 1;
		
	i = check_argv_eq(argc,argv, "-u");
	if(i)
		do_normalize = -1;
		
	i = check_argv_eq(argc,argv, "-x");
	if(i) {
		cache_memory = atoi(argv[i+1]);		
		if(cache_memory <= 0) {
			printf("Cache memory size set with -x must be a positive integer. Using default (max amount of available memory).\n");
		}
	}
	i = check_argv_eq(argc,argv, "-t");
	if(i) {
		if(atoi(argv[i+1]) > nprocs) {
			
			printf("Unattended request of a number of working threads above the number of available processors (%d)\n",nprocs);
		}
		else
			nprocs = atoi(argv[i+1]);
	}
	
	
	i = check_argv_eq(argc,argv, "-r");
	if(i) {
		printf("\nRetrain model %s ...\n", model_file); 
		if(!check_argv_eq(argc,argv, "-w"))
			printf("Using default chunk size: %ld\n (this may be different from what was previously used to train %s ).\n",chunk_size, model_file);
		retrain = true;
		MSVM_delete_model(model);
		model = MSVM_load_model(model_file);		
	}

	i = check_argv_eq(argc,argv, "-o");
	if(i) {
		if(atoi(argv[i+1]) == 1) {
			if(model->type == MSVM2)
				model->algorithm=Rosen;
			else
				printf("Rosen's algorithm is not available for this M-SVM, \n using Franke-Wolfe algorithm.\n");
		}	
	}
	
	i = check_argv_eq(argc,argv, "-s");
	if(i) {
		sparse_model_format = true;
	}
	
	i = check_argv_eq(argc,argv, "-S");
	if(i) {
		MSVM_delete_model(model);
		model = MSVM_load_model(model_file);
		if(model == NULL)
			exit(1);
			
		printf("\nConverting model %s in sparse format...\n", model_file); 
		printf("Note that the sparse format does not allow retraining.\n\n"); 
		MSVM_save_model_sparse(model, model_file);
		MSVM_delete_model_with_data(model);
		exit(0);
	}
	
	i = check_argv_eq(argc,argv, "-cv");
	if(i) {
		do_crossvalidation = true;
		nFolds = atoi(argv[i+1]);
	}
	
	// --- End of parsing the command line ---
	
	/*
		Load training data
	*/
	struct Data *training_set, *test_set;
	
	if(!retrain) {
		training_set = MSVM_make_dataset(training_file, model->datatype);
		if(training_set == NULL) {
			printf("Error: wrong or missing training data file. \n");
			printf("Note: the training data file should always be the first argument,\n");
			printf(" e.g.:	trainmsvm myTrainingData myMSVM.model\n\n");
			exit(0);
		}
		
		Q = training_set->Q;		
	}
	else {
		training_set = NULL;	// resume training with data already in model
		Q = model->Q;
	}
		
	/*
		Normalize data
	*/
	
	if(do_normalize==1) {
		// user asked for normalization 		
		MSVM_normalize_data(training_set, model);
	}
	
	else if(do_normalize==0){	
		// check if normalization needed
		if(MSVM_normalize_data(training_set, NULL) > 10) {
			printf("**\n** Columns of the data matrix show a large difference\n");
			printf("** between their standard deviations (> 10).\n");
			printf("** This could affect the performance of the classifier. \n\n");
			printf(" Do you want the data to be normalized ([y]/n)? \n");
			printf(" (Use the -n flag to force normalization from the command line\n");
			printf("  or the -u flag to bypass this test and use unnormalized data)\n");
			i=scanf("%c",&yesno);
			getchar();
			if(yesno != 'n') {
				MSVM_normalize_data(training_set, model);
			}
		}		
	}
	
	/*
		Set the list of C hyperparameters
	*/
	i = check_argv_eq(argc,argv, "-C");
	if(i) {
		MSVM_model_set_C(C, Q, model);
		for(k=1;k<=Q;k++) 
			model->C[k] = atof(argv[i+k]);
	}
	else {
		MSVM_model_set_C(C, Q, model);
	}
	
	/*
		Set filename for monitoring 
		(periodic saving of model in case something bad happens)
	*/
	sprintf(model_tmp_file, "%s.tmp", model_file);
	
	/* 
		Launch training		
	*/	
	if ( do_crossvalidation ) {
		// K-fold Cross validation
		cv_error_rates = MSVM_train_cv(model, training_set, nFolds, chunk_size, accuracy, cache_memory, nprocs, log);
		realtime = time(NULL) - starttime;
		cputime = clock() - startcputime;

		printf("\n================== Cross validation summary =======================\n");		
		printf(" Fold \t Training error rate \t Test error rate\n");
		
		for(k=1;k<=nFolds;k++)
			printf("  %d \t %3.2lf %% \t\t %3.2lf %%\n",k, 100*cv_error_rates[0][k], 100*cv_error_rates[1][k]);
		printf("\n====== %d-fold cross validation error rate estimate = %3.2lf %% =====", nFolds, 100*cv_error_rates[1][0]);
		if(cv_error_rates[1][0] < 0.1) 
			printf("=");
		if(nFolds < 10)
			printf("=");
		printf("\n\ncomputed in %s\n",print_time(realtime, cputime));
	
	}
	else {
		// Simple training (no cross validation)
		training_status = MSVM_train(model, training_set, chunk_size, accuracy, cache_memory, nprocs, alpha0, model_tmp_file, log);
		realtime = time(NULL) - starttime;
		cputime = clock() - startcputime;
	
		// Print output information
		if(training_status == 0) {
			if(model->ratio >= accuracy)
				printf("\n Optimum found with desired accuracy level (%3.2lf \%%)\n", 100*accuracy);
			else
				printf("\n Stopped training at accuracy level %3.2lf \%% \n(no improvement after %d iterations)\n", 100*model->ratio,TRAIN_STEP);
			printf("after %s\n\n", print_time(realtime, cputime));
		}
		else if(training_status == 1)
			printf("\n Stopped training after maximum number of iterations (%ld). \n", MSVM_TRAIN_MAXIT); 
		else if(training_status > 1)
			printf("\n Stopped by user after %lld iterations. \n", training_status);
		else {
			printf("Error in training MSVM. No model saved. \n");
			exit(0);
		}

		// Output training error rate
		if((double)((int)(10000 * model->training_error)) == 10000 * model->training_error)
			printf("Training error rate = %3.2lf %%\n",100*model->training_error);	
		else
			printf("Training error rate = %3.4lf %%\n",100*model->training_error);
		 
		// Write model to file
		if(sparse_model_format) {
			printf("\nSaving model in sparse format to %s...\n", model_file); 
			printf("Note that the sparse format does not allow retraining.\n\n"); 
			MSVM_save_model_sparse(model, model_file);
		}
		else {
			MSVM_save_model(model, model_file);
			printf("M-SVM model saved in %s.\n",model_file);
		}
		// and remove temporary file
		remove(model_tmp_file);
	
		// Save alpha into a matrix file if required
		if(alpha != NULL) {
			write_vector(model->alpha, model->nb_data,model->Q, alpha_file);
			printf("Values of alpha saved in %s.\n", alpha_file);
		}
	
		// Compute test error (if test_file is provided)
		if(test != NULL) {
			test_set = MSVM_make_dataset(test_file, model->datatype);
	
			// Compute output on test set
			MSVM_classify_set(NULL, test_set->X, test_set->y, test_set->nb_data, outputs, model, nprocs);
	
			MSVM_delete_dataset(test_set);
		}
	}
	
	// Free all
	if(retrain)
		MSVM_delete_model_with_data(model);
	else {
		MSVM_delete_model(model);
		MSVM_delete_dataset(training_set);
	}
	
	return 0;
}

