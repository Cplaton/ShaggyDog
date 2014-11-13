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
/*  Name           : libevalMSVM.c                                          */
/*  Version        : 2.0                                                    */
/*  Creation       : 06/20/08                                               */
/*  Last update    : 02/04/11                                               */
/*  Subject        : Evaluation functions for MSVMpack                      */
/*  Module         : Wrapper functions                                      */
/*  Author         : Fabien Lauer and Yann Guermeur Yann.Guermeur@loria.fr  */
/*--------------------------------------------------------------------------*/

#include "libevalMSVM.h"
#include "libevalMSVM_WW.h"
#include "libevalMSVM_CS.h"
#include "libevalMSVM_LLW.h" 
#include "libevalMSVM_2.h"
 
/*
	 ratio = MSVM_eval(&ub, gradient, H_alpha, {H_tilde_alpha,NULL}, model, verbose, fp)
	 
	 Evaluate the ratio between the value of the dual objective function and 
	 the upper bound on the optimum.
	 
	 use verbose = 0 to avoid printed output
	 
	 fp: 	FILE_ptr to the log file recording the values of the upper bound and dual objective function
	 	(use fp = NULL to deactivate the log file)

	This is a wrapper function calling the appropriate eval function for the model type
*/
double MSVM_eval(double *best_primal_upper_bound, double **gradient, double **H_alpha, double **H_tilde_alpha, struct Model *model, const int verbose, FILE *fp) {
	double ratio;
	switch (model->type) {
		case WW:
			ratio = MSVM_WW_eval(best_primal_upper_bound, gradient, H_alpha, model, verbose, fp);
			break;
		
		case CS:
			ratio = MSVM_CS_eval(best_primal_upper_bound, H_alpha, model, verbose, fp);
			break;
		
		
		case LLW:
			ratio = MSVM_LLW_eval(best_primal_upper_bound, gradient, H_alpha, model, verbose, fp);
			break;
		
		case MSVM2:
			ratio = MSVM_2_eval(best_primal_upper_bound, gradient, H_alpha, H_tilde_alpha, model, verbose, fp);
			break;
		
		
		default:
			printf("MSVM_eval: unknown model type.\n");
			ratio = 0;
			break;			
	}
	return ratio;
}

/*
	Standardize the vector b such that sum(b_k) = 0
*/
void standardize_b(double *b_SVM, const long Q)
{
	long k;
	double sum_b = 0.0;

	for(k=1; k<=Q; k++)
	  sum_b += b_SVM[k];

	sum_b /= Q;

	for(k=1; k<=Q; k++)
	  b_SVM[k] -= sum_b;

}

/*
	label = MSVM_classify(x, model, real_outputs)
	
	Compute the label of x WITHOUT TAKING CARE OF NORMALIZATION.
	(use MSVM_classify_set() to include normalization)
	 
	If real_outputs is not NULL, also provide the Q real-valued outputs
	 of the model in (real_outputs[1]...real_outputs[Q]).
	
	This is a wrapper function calling the appropriate classification function for the model type	
*/
long MSVM_classify(void *x, const struct Model *model, double *real_outputs) {
	long label;
	
	switch(model->type) {
		case WW:
		  label = WW_classify(x, model, real_outputs);
		  break;		  
		case CS:		
		  label = CS_classify(x, model, real_outputs);
		  break;		  
		case LLW:
		  label = LLW_classify(x, model, real_outputs);
		  break;		  
		case MSVM2:
		    label = MSVM_2_classify(x, model, real_outputs) ;
		  break;
		
		default:
			printf("MSVM_classify: unknown model type.\n");
			return 0;
	}
	
	return label;
}

void MSVM_print_outputs(double **outputs, long *labels, long *y, long nb_data, long Q, char *outputs_file) {
	long i,k;
	
	FILE *fc;

	if(outputs_file != NULL) {
		if((fc=fopen(outputs_file, "w"))==NULL)
		  printf("\nFile of outputs %s cannot be open, using standard output.\n", outputs_file);	 
	}

	if(outputs_file == NULL || fc == NULL) {
	  for(i=1;i<=nb_data;i++) {	  
		  for(k=1; k<=Q; k++) 
		    printf("%12.6lf ", outputs[i][k]);
		  printf("\t-> %ld",labels[i]);
		  if(y != NULL) {
		  	if(labels[i] == y[i])
			  	printf("   OK");
			else
				printf("   WRONG: true label is %ld",y[i]);
		  }
		  printf("\n");		  
	    }
	}
	else {
  	  for(i=1;i<=nb_data;i++) {
		  for(k=1; k<=Q; k++) 
		    fprintf(fc, "%1.6lf ", outputs[i][k]);
		  fprintf(fc, "%ld\n",labels[i]);
	  }
	  fclose(fc);
	}
}	  

/*
	MSVM_classify_set(labels, X, y, m, outputs_file, model, nprocs)

	Compute the predicted labels of an M-SVM for a set of m data points X.
	Also compute the test error and some statistics if y is not NULL.
    	
    	Resulting output is saved into outputs_file or printed on screen only if outputs_file is NULL. 

    	If labels is NULL, the predicted labels are not stored in memory. 
    	
    	Note: this function takes care of data normalization if needed (X should not be normalized).
    	
    	The function launches nprocs threads to compute in parallel over nprocs CPUs.
    	The SET_SIZE constant defined in libevalMSVM.h defines the size of the data subsets
    	on which the computations are performed in parallel.
*/
void MSVM_classify_set(long *labels, void **X, long *y, long nb_data, char *outputs_file,  struct Model *model, int nprocs)
{
	// Check number of threads
	if (nprocs <= 0 || nprocs >= get_nprocessors())
		nprocs = get_nprocessors();
		
	int t;
	pthread_t *threads=(pthread_t *)malloc(sizeof(pthread_t) * nprocs); // threads id
	void *status; 			// for pthread_join
	int rc; 			// return code of pthread functions
	int running_threads;
	long **mat_conf, ***mat_confs;

	long l,k;
	const long Q = model->Q;
	double **outputs = matrix(nb_data, Q);
	
	int free_labels = false;
	if(labels==NULL) {
		labels = (long *)malloc(sizeof(long) * (nb_data+1));
		free_labels = true;
	}	
	
	// Prepare data for computing threads
	struct ThreadClassifyData thread_data;
	thread_data.model = model;

	if(y != NULL) {
		mat_conf = matrix_l(Q,Q);
		for(k=1; k<=Q; k++)
		  for(l=1; l<=Q; l++)
		    mat_conf[k][l] = 0;
		
		mat_confs = (long***)malloc(sizeof(long***) * nprocs);
		for(t=0;t<nprocs;t++) 
			mat_confs[t] = matrix_l(Q,Q);
	}
	else {
		thread_data.y = NULL;
		thread_data.mat_conf = NULL;
	}
	
	printf("\n*** Computing the outputs and labels...\n");
	
	// Launch computing threads
	long next_set_index = 0;
	
	do {
		t = 0;
		running_threads = 0;
		while(t<nprocs && next_set_index < nb_data) {
	
			pthread_mutex_lock(&thread_classify_data_mutex); // Wait for thread_data to be read by previous thread
	 		
	 		thread_data.nb_data = (nb_data-next_set_index); 
	 		if (thread_data.nb_data > SET_SIZE) {
	 			thread_data.nb_data = SET_SIZE;
	 		}
	 		
			thread_data.thread_id = t;
			thread_data.X = &X[next_set_index];
			thread_data.labels = &labels[next_set_index];	
			thread_data.outputs = &outputs[next_set_index];	
			if(y != NULL) {
				thread_data.mat_conf = mat_confs[t];
				thread_data.y = &y[next_set_index];			
			}
			
			rc = pthread_create(&threads[t], NULL, MSVM_classify_set_thread, (void *) &thread_data);	
		
			next_set_index += thread_data.nb_data;
			running_threads++;
			t++;
		}
	
		// Wait for threads to terminate
		for(t=0;t<running_threads;t++) {
			rc = pthread_join(threads[t],&status);	
			
			if(y!=NULL) {
				// Collect partial mat_conf results				
				for(k=1; k<=Q; k++)
				  for(l=1; l<=Q; l++)
				    mat_conf[k][l] += mat_confs[t][k][l];
				    
			}			
		}
		// printf("\nExample %ld",next_set_index);
	} while (next_set_index < nb_data);
	
	// Print or save outputs
	printf("\n");
	MSVM_print_outputs(outputs,labels,y,nb_data,Q,outputs_file);
	
	if(y != NULL) {		
		if(model->crossvalidation == 0)
			printf("\n*** Test performance\n");
		else
			printf("\n*** Test performance on fold %d\n",model->crossvalidation);
		display_full_stats(Q, mat_conf);
		
		free(mat_conf[1]);free(mat_conf);
		for(t=0;t<nprocs;t++) {
			free(mat_confs[t][1]);free(mat_confs[t]);
		}
		free(mat_confs);
	}
	
	if(free_labels)
		free(labels);
	
	free(outputs[1]);free(outputs);
	free(threads);
}

void *MSVM_classify_set_thread(void *th_data) {
	// Recover data
	struct ThreadClassifyData *data =  (struct ThreadClassifyData *)th_data;
	const int thread_id = data->thread_id;
	const struct Model *model = data->model;	
	void **X = data->X;
	long *labels = data->labels ;	
	const long nb_data =data->nb_data;
	long **mat_conf = data->mat_conf;
	long *y=data->y;
	double **outputs = data->outputs;
	
	pthread_mutex_unlock(&thread_classify_data_mutex);	// Release thread_data for next thread 
		
	// Local variables
	long i,k,l,label,dim_bit;
	const long dim_input = model->dim_input;
	void *x;
	double *xd, **Xd;
	float *xf, **Xf;
	int *xi, **Xi;
	short int *xs, **Xs;
	unsigned char *xb, **Xb;
	
	const enum Datatype datatype = model->datatype;
	
	// Allocate a temporary vector (aligned)
	switch(datatype) {
	case DATATYPE_DOUBLE:
		xd = MALLOC(double,dim_input+1); 
		Xd = (double**)X;
		x = (void*)(xd + ALIGNMENT(double));
		break;
	case DATATYPE_FLOAT:
		xf = MALLOC(float, dim_input+1);
		Xf = (float**)X;
		x = (void*)(xf + ALIGNMENT(float));
		break;
	case DATATYPE_INT:
		xi = MALLOC(int, dim_input+1);
		Xi = (int**)X;
		x = (void*)(xi + ALIGNMENT(int));		
		break;	
	case DATATYPE_SHORT:
		xs = MALLOC(short int, dim_input+1);
		Xs = (short int**)X;
		x = (void*)(xs + ALIGNMENT(short int));		
		break;	
	case DATATYPE_BYTE:
		xb = MALLOC(unsigned char, dim_input+1);
		Xb = (unsigned char**)X;
		x = (void*)(xb + ALIGNMENT(unsigned char));		
		break;	
		
	case DATATYPE_BIT:
		dim_bit = dim_input/32;
		if(dim_input%32)
			dim_bit++;
		xi = MALLOC(int, dim_bit+1);
		Xi = (int**)X;
		x = (void*)(xi + ALIGNMENT(int));		
		break;	
	 default:
	  	printf("MSVM_classify_set(): Unknown data type.\n");
	  	exit(0);			
	}
	
	int do_matconf = false;
	if(y != NULL && mat_conf != NULL) {
		for(k=1; k<=model->Q; k++)
		  for(l=1; l<=model->Q; l++)
		    mat_conf[k][l] = 0;
		
		do_matconf = true;
	}
		
	// For all data in test set
	for(i=1; i<=nb_data; i++)  {
	
	  // compute a temporary data vector with correct data type, normalization and alignment
	  // Note that in a cross validation, test data are already normalized
	  // (as a subset of the whole training set that is normalized by trainMSVM.c)
	  switch(datatype) {

	  case DATATYPE_DOUBLE:
		  if (model->normalization == NULL || model->crossvalidation > 0) {
		  	for (k=1;k<=dim_input;k++)
		  		xd[k + ALIGNMENT(double)] = Xd[i][k];
		  }
		  else {
		  	for (k=1;k<=dim_input;k++) {
		  	    if(model->normalization[2][k] != 0.0)
		  			xd[k + ALIGNMENT(double)] = (Xd[i][k] - model->normalization[1][k]) / model->normalization[2][k];
		  	    else
		  	    	xd[k + ALIGNMENT(double)] = Xd[i][k];
		  	}
		  		
		  }
		  break;
	  case DATATYPE_FLOAT:
		  if (model->normalization == NULL || model->crossvalidation > 0) {
		  	for (k=1;k<=dim_input;k++)
		  		xf[k + ALIGNMENT(float)] = Xf[i][k];	  
		  }
		  else {
		  	for (k=1;k<=dim_input;k++) {
		  	    if(model->normalization[2][k] != 0.0)
		  			xf[k + ALIGNMENT(float)] = (Xf[i][k] - (float)model->normalization[1][k]) / (float)model->normalization[2][k];
		  	    else
		  	    	xf[k + ALIGNMENT(float)] = Xf[i][k];
		  	}
		  }
		  break;
	  case DATATYPE_INT:
		  for (k=1;k<=dim_input;k++)
	  		xi[k + ALIGNMENT(int)] = Xi[i][k];	  
		  
		  break;
	  case DATATYPE_SHORT:
		 for (k=1;k<=dim_input;k++)
	  		xs[k + ALIGNMENT(short int)] = Xs[i][k];	  
		 
		  break;
	  case DATATYPE_BYTE:
		 for (k=1;k<=dim_input;k++)
		 	xb[k + ALIGNMENT(unsigned char)] = Xb[i][k];	  
		 
		  break; 
	  case DATATYPE_BIT:
 	   	  for (k=1;k<=dim_bit;k++)
		  	xi[k + ALIGNMENT(int)] = Xi[i][k];	  
		  
		  break;
	  default:
	  	break;
	  }
	   
	  // Classify this data instance
	  label = MSVM_classify(x, model, outputs[i]);

	  labels[i] = label;

  	  if(do_matconf)
  	  	mat_conf[y[i]][label]++;
  	  
	  if((i % step) == 0)
	    {
	    	printf(".");
	  	fflush(stdout);
	   }
	}
	
	switch(datatype) {
	case DATATYPE_DOUBLE:
		FREE(xd);
		break;
	case DATATYPE_FLOAT:
		FREE(xf);
		break;
	case DATATYPE_INT:
		FREE(xi);
		break;
	case DATATYPE_SHORT:
		FREE(xs);
		break;
	case DATATYPE_BYTE:
		FREE(xb);
		break;
	case DATATYPE_BIT:
		FREE(xi);
		break;
	default:
		break;
	}
	pthread_exit(EXIT_SUCCESS);
}

