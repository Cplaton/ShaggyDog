/*
	MSVMpack  -  libMSVM.c
	
	General purpose functions for handling M-SVM models
	
*/

#include "libMSVM.h"
#include "kernel.h"

/*
	model_ptr = MSVM_make_model(type)
		
	Create an empty MSVM model of a given type (MSVM2, WW, CS, LLW)
		and returns a pointer to it or NULL in case of failure.
	
*/
struct Model *MSVM_make_model(enum MSVM_type type) {
	
	struct Model *model = (struct Model *)malloc(sizeof(struct Model));
	if(model == NULL)
		return NULL;
		
	switch(type) {

		case MSVM2 :
		case WW :
		case CS :
		case LLW :
			model->version = VERSION;
			model->type = type;		
			model->algorithm = FrankWolfe;				
			model->nature_kernel = LINEAR;
			model->kernel_par = NULL;
			model->datatype = DATATYPE_DOUBLE;
			model->Q = 0;
			model->C = NULL;
			model->training_error = 0.0;
			model->ratio = 0.0;
			model->iter = 0;
			model->crossvalidation = false;
			
			model->alpha = NULL;
			model->partial_average_alpha = NULL;
			model->b_SVM = NULL;
			model->training_set_name = NULL;			
			model->X = NULL;
			model->y = NULL;
			model->normalization = NULL;			
			model->W = NULL;
			model->X_double = NULL;
			model->X_float = NULL;
			model->X_int = NULL;
			model->X_short = NULL;	
			model->X_byte = NULL;

			pthread_mutex_init(&(model->mutex), NULL);
			break;
			
		default :
			printf("\n Sorry, this type of M-SVM (%d) is not included in MSVMpack.\n",type);
			return NULL;
	}
		
	// Return the pointer to the created model
	return model;
}

/*
	model_ptr = MSVM_make_model_copy(model)
		
	Create a new MSVM model as a copy of a given model
		and returns a pointer to it or NULL in case of failure.
		
	Warning: this does not copy the data fields X and y, 
				only the model parameters.
			
*/
struct Model *MSVM_make_model_copy(struct Model *model_src) {
	long i,j,k;
	int p,n_kernel_par;
	
	struct Model *model = (struct Model *)malloc(sizeof(struct Model));
	if(model == NULL)
		return NULL;
		
	model->type = model_src->type;	
	switch(model->type) {

		case MSVM2 :
		case WW :
		case CS :
		case LLW :
			model->version = model_src->version;
			
			model->algorithm = model_src->algorithm;				
			model->nature_kernel = model_src->nature_kernel;
			model->kernel_par = NULL;
			if(model_src->kernel_par != NULL) {
				n_kernel_par = (int)model_src->kernel_par[0];
				model->kernel_par = (double *)malloc(sizeof(double) * (n_kernel_par + 1));
				model->kernel_par[0] = (double)n_kernel_par;	
				for(p=1;p<=n_kernel_par;p++) 
					model->kernel_par[p] = model_src->kernel_par[p];
			}
			model->datatype = model_src->datatype;			
			model->Q = model_src->Q; 
			model->C = NULL;
			if(model_src->C != NULL) {
				MSVM_model_set_C(1.0,model_src->Q, model);
				for(k=1;k<=model->Q;k++) {
					model->C[k] = model_src->C[k];
				}
			}
			
			model->training_error = model_src->training_error;
			model->ratio = model_src->ratio;
			model->iter = model_src->iter;			
			model->crossvalidation = model_src->crossvalidation;
						
			model->alpha = NULL;
			model->partial_average_alpha = NULL;
			if(model_src->alpha != NULL) {
				model->alpha = matrix(model_src->nb_data, model_src->Q);
				model->partial_average_alpha = (double *) malloc(sizeof(double) * (model_src->nb_data+1));	
				for(i=1; i<=model_src->nb_data; i++) {
				  model->partial_average_alpha[i] = model_src->partial_average_alpha[i];
				  for(k=1; k<=model_src->Q; k++)
					model->alpha[i][k] = model_src->alpha[i][k] ;
				}
			}	
			model->b_SVM = NULL;
			if(model_src->b_SVM != NULL) {
				model->b_SVM = (double *)malloc((model_src->Q+1) *  sizeof(double));
				for(k=1; k<=model_src->Q; k++)
					model->b_SVM[k] = model_src->b_SVM[k];
			}
			
			model->training_set_name = NULL;
			if( model->training_set_name != NULL) {
				model->training_set_name = (char*)malloc(sizeof(char) * (strlen(model_src->training_set_name) + 1));
				strcpy(model->training_set_name, model_src->training_set_name);
			}
					 
			model->X = NULL;
			model->y = NULL;
			
			model->X_double = NULL;
			model->X_float = NULL;
			model->X_int = NULL;
			model->X_short = NULL;
			model->X_byte = NULL;

			model->normalization = NULL;
			if(model_src->normalization != NULL) {
				model->normalization = matrix(2,model_src->dim_input); 
				for(j=1; j<=model_src->dim_input; j++) {
					model->normalization[1][j] = model_src->normalization[1][j] ;
					model->normalization[2][j] = model_src->normalization[2][j] ;
				}
			}
						
			model->W = NULL; 
			if(model_src->W != NULL) {
				for(j=1;j<=model_src->dim_input;j++) {
					for(k=1; k<=model_src->Q; k++)
						model->W[j][k] = model_src->W[j][k];
				}
			}	
			
			pthread_mutex_init(&(model->mutex), NULL);			
			break;
			
		default :
			printf("\n Sorry, this type of M-SVM (%d) is not included in MSVMpack.\n",model->type);
			return NULL;
	}
		
	// Return the pointer to the created model
	return model;
}
 
/*
	MSVM_init_model(model_ptr, fichcom_file)
	
	Sets the parameters Q, C and nature_kernel from a file
	and returns chunk_size
*/
long MSVM_init_model(struct Model *model, char *com_file) {

	long k,chunk_size;
	unsigned int kernel_type;
	
	// Read com_file for hyperparameters
	FILE *fs;
	long nr = 0;
	
	if((fs=fopen(com_file, "r"))==NULL)
	  {
	  printf("\nFile of parameters %s cannot be open.\n", com_file);
	  exit(0);
	  }

	nr += fscanf(fs, "%ld", &(model->Q));
	nr += fscanf(fs, "%d", &kernel_type);
	model->nature_kernel = kernel_type;
	set_default_kernel_par(model);

	model->C = (double*)realloc(model->C, sizeof(double) * (model->Q + 1));
	for(k=1;k<=model->Q;k++)
		nr += fscanf(fs, "%lf", &(model->C[k]));
	if (fscanf(fs, "%ld", &chunk_size) == EOF)
		chunk_size = 0;	// eval-type com_file, no chunk size

	if(nr < 3)
		printf("\nError in reading com file, not enough data.\n");
	return chunk_size;
}

void allocate_model_data(struct Model *model, const long nb_data, const long dim_input) {	

	// Data type can be deduced from the kernel type:
	switch(model->nature_kernel / 10) {
	case 1: 
		model->datatype = DATATYPE_FLOAT;
		break;
	case 2:
		model->datatype = DATATYPE_INT;
		break;
	case 3:
		model->datatype = DATATYPE_SHORT;
		break;
	case 4:
		model->datatype = DATATYPE_BYTE;
		break;
	case 5:
		model->datatype = DATATYPE_BIT;
		break;
	case 0:
	default:
		model->datatype = DATATYPE_DOUBLE;
		break;
	}
	
	// Allocate the corresponding storage: 
	switch (model->datatype) {
	case DATATYPE_FLOAT:
		model->X_float = matrix_f_aligned(nb_data,dim_input);	
		model->X = (void **)model->X_float;	
		break;
	case DATATYPE_INT:
		model->X_int = matrix_i_aligned(nb_data,dim_input);
		model->X = (void **)model->X_int;		
		break;
	case DATATYPE_SHORT:
		model->X_short = matrix_s_aligned(nb_data,dim_input);
		model->X = (void **)model->X_short;		
		break;
	case DATATYPE_BYTE:
		model->X_byte = matrix_b_aligned(nb_data,dim_input);
		model->X = (void **)model->X_byte;		
		break;
	case DATATYPE_BIT:
		model->X_int = matrix_i_aligned(nb_data,dim_input/32 + (dim_input%32 != 0) );
		model->X = (void **)model->X_int;		
		break;
		
	case DATATYPE_DOUBLE:
		model->X_double = matrix_aligned(nb_data,dim_input);
		model->X = (void **)model->X_double;		
		break;	
	default:
		printf("Unknown data format in allocate_model_data().\n");
		exit(1);
	}

}

void free_model_data(struct Model *model) {
	
	if(model->X_double != NULL) 
		free_mat_aligned(model->X_double);
	
	if(model->X_float != NULL) 
		free_mat_f_aligned(model->X_float);
	
	if(model->X_int != NULL) 
		free_mat_i_aligned(model->X_int);
	
	if(model->X_short != NULL) 
		free_mat_s_aligned(model->X_short);

	if(model->X_byte != NULL) 
		free_mat_b_aligned(model->X_byte);
	
	
	model->X = NULL;
}	

/*
	MSVM_delete_model(model_ptr)
		
	Delete the model pointed by model_ptr
*/
void MSVM_delete_model(struct Model *model) {
	
	if (model == NULL)
		return;
		
	switch(model->type) {

		case MSVM2 :
		case WW :
		case CS :
		case LLW :
			if(model->C != NULL) {
				free(model->C); 
			}
			if(model->alpha != NULL) {
				free(model->alpha[1]); free(model->alpha); 
			}
			if(model->partial_average_alpha != NULL) {
				free(model->partial_average_alpha); 
			}
			if(model->b_SVM != NULL) {
				free(model->b_SVM); 
			}
			if(model->normalization != NULL){
				free(model->normalization[1]); free(model->normalization);
			}
			if(model->W != NULL) {
				free(model->W[1]); free(model->W);
			}
			if(model->training_set_name != NULL) {
				free(model->training_set_name);
			}
			if(model->kernel_par != NULL) {
				free(model->kernel_par);
			}
			pthread_mutex_destroy(&(model->mutex));
			
			break;
		default :
			printf("\n Unknown MSVM type. Nothing to delete.\n");
			break;
	}
	
	free(model);
	return;
}

/*
	MSVM_delete_model_with_data(model_ptr)
		
	Delete the model pointed by model_ptr 
	and the data in model_ptr->(X, y)
*/
void MSVM_delete_model_with_data(struct Model *model) {
	
	if (model == NULL)
		return;
		
	switch(model->type) {

		case MSVM2 :
		case WW :
		case CS :
		case LLW :		
			if(model->C != NULL) {
				free(model->C); 
			}
			if(model->alpha != NULL) {
				free(model->alpha[1]); free(model->alpha); 				
			}
			if(model->partial_average_alpha != NULL) {
				free(model->partial_average_alpha); 
			}
			if(model->b_SVM != NULL) {
				free(model->b_SVM); 				
			}
			if(model->normalization != NULL){
				free(model->normalization[1]); 
				free(model->normalization);
			}
			if(model->W != NULL) {
				free(model->W[1]); 
				free(model->W);
			}
			if(model->training_set_name != NULL) {
				free(model->training_set_name);
			}
			if(model->kernel_par != NULL) {
				free(model->kernel_par);
			}
			
			
			pthread_mutex_destroy(&(model->mutex));
			
			break;		
			
		default :
			printf("\n Unknown MSVM type. Nothing to delete.\n");
			break;
	}
	// Free data memory
	free_model_data(model);
	
	if(model->y != NULL) {
		free(model->y);
	}
	free(model);
	return;
}

/*
	Save an MSVM model to a file (with all training data as SVs)
	Returns the number of SVs written to model_file 
		(0 usually means failure)
*/
long MSVM_save_model(const struct Model *model, char *model_file) {
	
	long i,k, dim_bit;
	long nb_data = model->nb_data;
	long dim_input = model->dim_input;
	long Q = model->Q;
	long nSV = 0;
	FILE *fp;
	char training_set[255];
	
	if((fp=fopen(model_file, "w"))==NULL)
	  {
	  printf("\n  Model file %s cannot be open.\n", model_file);
	  return 0;
	  }
	  
	// Write model information
	fprintf(fp, "%1.1f\n",model->version);
	remove_spaces(model->training_set_name, training_set,255);
	fprintf(fp, "%d\n", model->type);
	fprintf(fp, "%ld\n", Q);
	fprintf(fp, "%d\n", model->nature_kernel);
	if(model->kernel_par != NULL) {
		fprintf(fp, "%d ", (int) (model->kernel_par[0]));
		for(i=1;i<=(int) (model->kernel_par[0]); i++)
			fprintf(fp, "%lf ", model->kernel_par[i]);
		fprintf(fp,"\n");
	}
	else 
		fprintf(fp, "0\n");

	fprintf(fp, "%s\n", training_set);
	fprintf(fp, "%lf\n", model->training_error);
	fprintf(fp, "%ld\n", nb_data);
	fprintf(fp, "%ld\n", dim_input);
	for(k=1;k<=Q;k++)	
		fprintf(fp, "%1.10lf ", model->C[k]);
	fprintf(fp, "\n");
	
	// Write data normalization information
	if(model->normalization != NULL) {
		for(k=1;k<=dim_input;k++)
			fprintf(fp, "%1.10lf ", model->normalization[2][k]); // std
		fprintf(fp, "\n");
		for(k=1;k<=dim_input;k++)
			fprintf(fp, "%1.10lf ", model->normalization[1][k]); // mean
		fprintf(fp, "\n");
	} 
	else {
		// Do not use data normalization
		fprintf(fp, "-1\n");
	}
	
	// Write vector b
	if(model->type != CS) {	
		for(k=1; k<=Q; k++)
			fprintf(fp, "%1.10lf ", model->b_SVM[k]);
		fprintf(fp,"\n");
	}
	// True dimension for BIT data
	dim_bit = dim_input/32;
	if(dim_input%32)
		dim_bit++;
		
	// Write support vectors and corresponding alpha	
	for(i=1; i <= nb_data; i++) {
		// Write [alpha_i1 ... alpha_iQ]
		for(k = 1; k <= Q; k++) {
			if(model->alpha[i][k] == 0.0)
				fprintf(fp,"0.0 ");
			else if (model->alpha[i][k] < 1e-8)
				fprintf(fp,"%1.10e ", model->alpha[i][k]);
			else
				fprintf(fp,"%1.10lf ", model->alpha[i][k]);
		}
		fprintf(fp, "\n");

		// Write x_i
		switch(model->datatype) {
		case DATATYPE_DOUBLE:
			for(k=1;k<=dim_input;k++)
			  fprintf(fp,"%1.10lf ", model->X_double[i][k]);			  
			break;
		case DATATYPE_FLOAT:
			for(k=1;k<=dim_input;k++)
			  fprintf(fp,"%1.8f ", model->X_float[i][k]);
			break;
		case DATATYPE_INT:
			for(k=1;k<=dim_input;k++)
			  fprintf(fp,"%d ", model->X_int[i][k]);
			break;
		case DATATYPE_SHORT:
			for(k=1;k<=dim_input;k++)
			  fprintf(fp,"%hd ", model->X_short[i][k]);
			break;
		case DATATYPE_BYTE:
			for(k=1;k<=dim_input;k++)
			  fprintf(fp,"%hhu ", model->X_byte[i][k]);
			break;
		case DATATYPE_BIT:
			// Write compressed format			
			for(k=1;k<=dim_bit;k++)
			  fprintf(fp,"%d ", model->X_int[i][k]);
			break;

		default:
			printf("Error in MSVM_model_save(): unknown data format (%d).\n",model->datatype);
			exit(1);
		}

		// Write y_i
		fprintf(fp, "%ld\n", model->y[i]);
		
		nSV++;
	}

	fclose(fp);
	return nSV;
}

/*
	Save only the SVs of an MSVM model to a file
	Returns the number of SVs written to model_file 
		(0 usually means failure)
*/
long MSVM_save_model_sparse(const struct Model *model, char *model_file) {
	
	long i,k,dim_bit;
	long nb_data = model->nb_data;
	long dim_input = model->dim_input;
	long Q = model->Q;
	long nSV = 0;
	int is_SV;
	int linear_model = false;
	FILE *fp; 
	char training_set[255];
	
	if((fp=fopen(model_file, "w"))==NULL)
	  {
	  printf("\n  Model file %s: cannot be open.\n", model_file);
	  return 0;
	  }
	  
	if(model->nature_kernel == LINEAR && model->type == WW)
		linear_model = true;
		
	// Find the number of SVs
	if(linear_model) 
		nSV = Q;	
	else {
		for(i=1; i <= nb_data; i++) {
			if(model->type == CS) {		
				for(k = 1; k <= Q; k++) 
					if(k != model->y[i] && model->alpha[i][k] != 0.0) {
						nSV++;
						break;
					}
			}
			else {
				for(k = 1; k <= Q; k++) 
					if(model->alpha[i][k] != 0.0) {
						nSV++;
						break;
					}
			}
		}
	}
	
	// Write model information
	fprintf(fp, "%1.1f\n",model->version);
	remove_spaces(model->training_set_name, training_set,255);
	fprintf(fp, "%d\n", model->type);
	fprintf(fp, "%ld\n", Q);
	fprintf(fp, "%d\n", model->nature_kernel);
	if(model->kernel_par != NULL) {
		fprintf(fp, "%d ", (int) (model->kernel_par[0]));
		for(i=1;i<=(int) (model->kernel_par[0]); i++)
			fprintf(fp, "%lf ", model->kernel_par[i]);
		fprintf(fp,"\n");
	}
	else 
		fprintf(fp, "0\n");
	fprintf(fp, "%s\n", training_set);
	fprintf(fp, "%lf\n", model->training_error);	
	fprintf(fp, "%ld\n", nSV);
	fprintf(fp, "%ld\n", dim_input);
	for(k=1;k<=Q;k++)	
		fprintf(fp, "%1.10lf ", model->C[k]);
	fprintf(fp, "\n");
	
	// Write data normalization information
	if(model->normalization != NULL) {
		for(k=1;k<=dim_input;k++)
			fprintf(fp, "%1.10lf ", model->normalization[2][k]); // std
		fprintf(fp, "\n");
		for(k=1;k<=dim_input;k++)
			fprintf(fp, "%1.10lf ", model->normalization[1][k]); // mean
		fprintf(fp, "\n");
	} 
	else {
		// Do not use data normalization
		fprintf(fp, "-1\n");
	}
	
	// Write vector b
	if(model->type != CS) {	
		for(k=1; k<=Q; k++)
			fprintf(fp, "%1.10lf ", model->b_SVM[k]);
		fprintf(fp,"\n");
	}
	
	// True dimension for BIT data
	dim_bit = dim_input/32;
	if(dim_input%32)
		dim_bit++;
		
	// Write support vectors and corresponding alpha	
	if(linear_model) {
		nSV = 0;
		WW_compute_W(model);
		for(i=1; i<=Q; i++) {
			for(k = 1; k <= Q; k++) {
				if(i==k)
					fprintf(fp,"1.0 ");
				else
					fprintf(fp,"0.0 ");
			}
			fprintf(fp, "\n");
			
			for(k=1;k<=dim_input;k++) {
				fprintf(fp,"%1.10lf ", model->W[k][i]);
			}
			fprintf(fp,"%ld\n ", i);
			nSV++;
		}
	}
	else{
		nSV = 0;
		for(i=1; i <= nb_data; i++) {
			is_SV = 0;
			if(model->type == CS) {		
				for(k = 1; k <= Q; k++) 
					if(k != model->y[i] && model->alpha[i][k] != 0.0) {
						is_SV = 1;
						break;
					}
			}
			else {
				for(k = 1; k <= Q; k++) 
					if(model->alpha[i][k] != 0.0) {
						is_SV = 1;
						break;
					}
			}
			
			if(is_SV) {
				// Write [alpha_i1 ... alpha_iQ]
				for(k = 1; k <= Q; k++) {
					if(model->alpha[i][k] == 0.0)
						fprintf(fp,"0.0 ");
					else if (model->alpha[i][k] < 1e-8)
						fprintf(fp,"%1.10e ", model->alpha[i][k]);
					else
						fprintf(fp,"%1.10lf ", model->alpha[i][k]);
				}
				fprintf(fp, "\n");
			
				// Write x_i
				switch(model->datatype) {
				case DATATYPE_FLOAT:
					for(k=1;k<=dim_input;k++)
					  fprintf(fp,"%1.8f ", model->X_float[i][k]);
					break;
				case DATATYPE_INT:
					for(k=1;k<=dim_input;k++)
					  fprintf(fp,"%d ", model->X_int[i][k]);
					break;
				case DATATYPE_SHORT:
					for(k=1;k<=dim_input;k++)
					  fprintf(fp,"%hd ", model->X_short[i][k]);
					break;
				case DATATYPE_BYTE:
					for(k=1;k<=dim_input;k++)
					  fprintf(fp,"%hhu ", model->X_byte[i][k]);
					break;
				case DATATYPE_BIT:
					// Write compressed format					
					for(k=1;k<=dim_bit;k++)
					  fprintf(fp,"%d ", model->X_int[i][k]);
					break;
				case DATATYPE_DOUBLE:
				default:
					for(k=1;k<=dim_input;k++)
					  fprintf(fp,"%1.10lf ", model->X_double[i][k]);
					break;
				}
				// Write y_i
				fprintf(fp, "%ld\n", model->y[i]);
			 
				nSV++;
			}	
		}
	}
	fclose(fp);
	return nSV;
}

/*
	Load an MSVM model from a file
*/
struct Model *MSVM_load_model(char *model_file) {

	long i,k, nb_data, Q, dim_input, dim_bit;
	double alpha,normalization;
	FILE *fp;
	long nr=0;
	float version;
	unsigned int type, kernel_type;
	char buffer[200];
	int n_kernel_par;
	
	if((fp=fopen(model_file, "r"))==NULL)
	  {
	  printf("\n  Model file %s cannot be open.\n", model_file);
	  return NULL;
	  }

	struct Model *model = MSVM_make_model(MSVM2);
	
	if(model == NULL)
		return NULL;
		
/*
	model->alpha = NULL;
	model->y = NULL;
	model->partial_average_alpha = NULL;
	model->b_SVM = NULL;
	model->W = NULL; 
	model->X_double = NULL;
	model->X_float = NULL;
	model->X_int = NULL;
	model->X_short = NULL;
	model->X_byte = NULL;
*/

	nr += fscanf(fp, "%s", buffer);
	if(strlen(buffer) < 3 ){
		version = 1.0;	// no version number = version 1.0
		type = atoi(buffer);
	}
	else {
		version = atof(buffer);
		nr += fscanf(fp, "%d", &type);
	}
	model->version = version;
	model->type = type;
	model->algorithm = FrankWolfe;	// default back to Frank-Wolfe method
					// use -o 1 to retrain with Rosen's method

	nr += fscanf(fp, "%ld", &Q);
	model->Q = Q;

	nr += fscanf(fp, "%d", &kernel_type);
	model->nature_kernel = kernel_type;

	nr += fscanf(fp, "%d", &n_kernel_par);
	model->kernel_par = (double *)malloc(sizeof(double) * (n_kernel_par + 1));
	model->kernel_par[0] = (double)n_kernel_par;	
	for(i=1;i<=n_kernel_par;i++) 
		nr += fscanf(fp, "%lf", &(model->kernel_par[i]));
	
	nr += fscanf(fp,"%s", buffer);
	model->training_set_name = (char*)malloc(sizeof(char) * (strlen(buffer) + 1));
	strcpy(model->training_set_name, buffer);
	
	nr += fscanf(fp, "%lf", &(model->training_error));
	
	model->ratio = 0.0; // (information not saved)
	model->iter = 0;	// (information not saved)
	model->crossvalidation = 0;
				
	nr += fscanf(fp, "%ld", &nb_data);
	model->nb_data = nb_data;
	nr += fscanf(fp, "%ld", &dim_input);
	model->dim_input = dim_input;
	dim_bit = dim_input/32; // true dimension for BIT data
	if(dim_input%32)
		dim_bit++;

	// C hyperparameters
	model->C = (double*) malloc(sizeof(double) * (Q+1));
	if (version > 1.0) {
		for(k=1;k<=Q;k++)	
			nr += fscanf(fp, "%lf", &(model->C[k]));
	}
	else {
		nr += fscanf(fp, "%lf", &(model->C[1]));
		for(k=2;k<=Q;k++)	
			model->C[k] = model->C[1];
	}
	
	nr += fscanf(fp, "%lf", &normalization);
	
	if(nr < 10){
		printf("Error, cannot read model file %s.\n",model_file);
		exit(0);
	}

	if(normalization >= 0.0) {
		model->normalization = matrix(2,dim_input);
		model->normalization[2][1] = normalization;
		for(k=2;k<=dim_input;k++)
			nr = fscanf(fp, "%lf", &(model->normalization[2][k]));
		for(k=1;k<=dim_input;k++)
			nr = fscanf(fp, "%lf", &(model->normalization[1][k]));
	}
	else
		model->normalization = NULL;
	
	nr = 0;
	
	// Model file format could change with MSVM type
	// The following loads the model accordingly
	switch(type) { 
		case MSVM2 :
			// Allocate memory for model parameters
			model->alpha = matrix(nb_data,Q); 
			model->partial_average_alpha = (double *) malloc(sizeof(double) * (nb_data+1));
			model->b_SVM = (double *)malloc((Q+1) *  sizeof(double));
	
			// Allocate model->X and set model->datatype
			allocate_model_data(model, nb_data, dim_input); 
			
			model->y = (long*)malloc(sizeof(long) * (nb_data+1));
	
			// Read b
			for(k=1; k<=Q; k++)
				nr += fscanf(fp, "%lf", &(model->b_SVM[k]));
		
			for(i=1; i <= nb_data; i++) {
				model->partial_average_alpha[i] = 0.0;
		
				// Read [alpha_i1 ... alpha_iQ]
				for(k = 1; k <= Q; k++) {
					nr += fscanf(fp, "%lf", &alpha);
					model->alpha[i][k] = alpha;
					model->sum_all_alpha += alpha;
					model->partial_average_alpha[i] += alpha;
				}	    
				model->partial_average_alpha[i] /= Q;	

				// Read x_i
				switch (model->datatype) {
				case DATATYPE_DOUBLE:
					for(k=1;k<=dim_input;k++)
					  nr += fscanf(fp, "%lf", &(model->X_double[i][k]));
					break;
				case DATATYPE_FLOAT:
					for(k=1;k<=dim_input;k++)
					  nr += fscanf(fp, "%f", &(model->X_float[i][k]));
					break;
				case DATATYPE_INT:
					for(k=1;k<=dim_input;k++)
					  nr += fscanf(fp, "%d", &(model->X_int[i][k]));
					break;
				case DATATYPE_SHORT:
					for(k=1;k<=dim_input;k++)
					  nr += fscanf(fp, "%hd", &(model->X_short[i][k]));
					break;
				case DATATYPE_BYTE:
					for(k=1;k<=dim_input;k++)
					  nr += fscanf(fp, "%hhu", &(model->X_byte[i][k]));
					break;
				case DATATYPE_BIT:
					// Load compressed format					
					for(k=1;k<=dim_bit;k++)
					  nr += fscanf(fp, "%d", &(model->X_int[i][k]));
					break;	
				default:
					printf("Error: unknown data format (%d) in MSVM_load_model().\n",model->datatype);
					exit(1);
				}
				
				// Read y_i
				nr += fscanf(fp, "%ld", &(model->y[i]));		
			}
			if ((model->datatype != DATATYPE_BIT && nr < Q + (Q + dim_input + 1)*nb_data) || (model->datatype == DATATYPE_BIT && nr < Q + (Q + dim_bit + 1)*nb_data))
				printf("\nError in reading model file, not enough data.\n");
			break;
			
		case WW :
			// Allocate memory for model parameters
			model->alpha = matrix(nb_data,Q); 
			model->b_SVM = (double *)malloc((Q+1) *  sizeof(double));
			
			// Allocate model->X and set model->datatype
			allocate_model_data(model, nb_data, dim_input); 

			model->y = (long*)malloc(sizeof(long) * (nb_data+1));
	
			// Read b
			for(k=1; k<=Q; k++)
				nr += fscanf(fp, "%lf", &(model->b_SVM[k]));
		
			for(i=1; i <= nb_data; i++) {
		
				// Read [alpha_i1 ... alpha_iQ]
				for(k = 1; k <= Q; k++) {
					nr += fscanf(fp, "%lf", &alpha);
					model->alpha[i][k] = alpha;
					model->sum_all_alpha += alpha;				
				}	    
				
				// Read x_i
				switch (model->datatype) {
				case DATATYPE_DOUBLE:
					for(k=1;k<=dim_input;k++)
					  nr += fscanf(fp, "%lf", &(model->X_double[i][k]));
					break;
				case DATATYPE_FLOAT:
					for(k=1;k<=dim_input;k++)
					  nr += fscanf(fp, "%f", &(model->X_float[i][k]));
					break;
				case DATATYPE_INT:
					for(k=1;k<=dim_input;k++)
					  nr += fscanf(fp, "%d", &(model->X_int[i][k]));
					break;
				case DATATYPE_SHORT:
					for(k=1;k<=dim_input;k++)
					  nr += fscanf(fp, "%hd", &(model->X_short[i][k]));
					break;
				case DATATYPE_BYTE:
					for(k=1;k<=dim_input;k++)
					  nr += fscanf(fp, "%hhu", &(model->X_byte[i][k]));
					break;
				case DATATYPE_BIT:
					// Load compressed format					
					for(k=1;k<=dim_bit;k++)
					  nr += fscanf(fp, "%d", &(model->X_int[i][k]));
					break;	
				default:
					printf("Error: unknown data format (%d) in MSVM_load_model().\n",model->datatype);
					exit(1);
				}
				
				// Read y_i
				nr += fscanf(fp, "%ld", &(model->y[i]));		
			}
			if ((model->datatype != DATATYPE_BIT && nr < Q + (Q + dim_input + 1)*nb_data) || (model->datatype == DATATYPE_BIT && nr < Q + (Q + dim_bit + 1)*nb_data))
				printf("\nError in reading model file, not enough data.\n");
			break;
		case CS :
			// Allocate memory for model parameters			
			model->alpha = matrix(nb_data,Q); 
						
			// Allocate model->X and set model->datatype
			allocate_model_data(model, nb_data, dim_input); 
			
			model->y = (long*)malloc(sizeof(long) * (nb_data+1));

			for(i=1; i <= nb_data; i++) {
		
				// Read [alpha_i1 ... alpha_iQ]
				for(k = 1; k <= Q; k++) {
					nr += fscanf(fp, "%lf", &alpha);
					model->alpha[i][k] = alpha;
					model->sum_all_alpha += alpha;				
				}	    
				
				// Read x_i
				switch (model->datatype) {
				case DATATYPE_DOUBLE:
					for(k=1;k<=dim_input;k++)
					  nr += fscanf(fp, "%lf", &(model->X_double[i][k]));
					break;
				case DATATYPE_FLOAT:
					for(k=1;k<=dim_input;k++)
					  nr += fscanf(fp, "%f", &(model->X_float[i][k]));
					break;
				case DATATYPE_INT:
					for(k=1;k<=dim_input;k++)
					  nr += fscanf(fp, "%d", &(model->X_int[i][k]));
					break;
				case DATATYPE_SHORT:
					for(k=1;k<=dim_input;k++)
					  nr += fscanf(fp, "%hd", &(model->X_short[i][k]));
					break;
				case DATATYPE_BYTE:
					for(k=1;k<=dim_input;k++)
					  nr += fscanf(fp, "%hhu", &(model->X_byte[i][k]));
					break;
				case DATATYPE_BIT:
					// Load compressed format
					
					for(k=1;k<=dim_bit;k++)
					  nr += fscanf(fp, "%d", &(model->X_int[i][k]));
					break;	
				default:
					printf("Error: unknown data format (%d) in MSVM_load_model().\n",model->datatype);
					exit(1);
				}
				
				// Read y_i
				nr += fscanf(fp, "%ld", &(model->y[i]));		
			}
			if ((model->datatype != DATATYPE_BIT && nr < (Q + dim_input + 1)*nb_data) || (model->datatype == DATATYPE_BIT && nr < (Q + dim_bit + 1)*nb_data))
				printf("\nError in reading model file, not enough data.\n");
			break;
		case LLW :
			// Allocate memory for model parameters
			model->alpha = matrix(nb_data,Q); 
			model->b_SVM = (double *)malloc((Q+1) *  sizeof(double));

			// Allocate model->X and set model->datatype
			allocate_model_data(model, nb_data, dim_input); 

			model->y = (long*)malloc(sizeof(long) * (nb_data+1));
	
			// Read b
			for(k=1; k<=Q; k++)
				nr += fscanf(fp, "%lf", &(model->b_SVM[k]));
		
			for(i=1; i <= nb_data; i++) {
		
				// Read [alpha_i1 ... alpha_iQ]
				for(k = 1; k <= Q; k++) {
					nr += fscanf(fp, "%lf", &alpha);
					model->alpha[i][k] = alpha;
					model->sum_all_alpha += alpha;				
				}	    
				
				// Read x_i
				switch (model->datatype) {
				case DATATYPE_DOUBLE:
					for(k=1;k<=dim_input;k++)
					  nr += fscanf(fp, "%lf", &(model->X_double[i][k]));
					break;
				case DATATYPE_FLOAT:
					for(k=1;k<=dim_input;k++)
					  nr += fscanf(fp, "%f", &(model->X_float[i][k]));
					break;
				case DATATYPE_INT:
					for(k=1;k<=dim_input;k++)
					  nr += fscanf(fp, "%d", &(model->X_int[i][k]));
					break;
				case DATATYPE_SHORT:
					for(k=1;k<=dim_input;k++)
					  nr += fscanf(fp, "%hd", &(model->X_short[i][k]));
					break;
				case DATATYPE_BYTE:
					for(k=1;k<=dim_input;k++)
					  nr += fscanf(fp, "%hhu", &(model->X_byte[i][k]));
					break;
				case DATATYPE_BIT:
					// Load compressed format					
					for(k=1;k<=dim_bit;k++)
					  nr += fscanf(fp, "%d", &(model->X_int[i][k]));
					break;	
				default:
					printf("Error: unknown data format (%d) in MSVM_load_model().\n",model->datatype);
					exit(1);
				}
				
				// Read y_i
				nr += fscanf(fp, "%ld", &(model->y[i]));		
			}
			if ((model->datatype != DATATYPE_BIT && nr < Q + (Q + dim_input + 1)*nb_data) || (model->datatype == DATATYPE_BIT && nr < Q + (Q + dim_bit + 1)*nb_data))
				printf("\nError in reading model file, not enough data.\n");


			// Initialize a new mutex
			//pthread_mutex_init(&(model->mutex), NULL);
			
			break;
									
		default :
			printf("Unknown MSVM model type in file %s . \n", model_file);
			return NULL;
			break;
	}
	// Initialize a new mutex
	// pthread_mutex_init(&(model->mutex), NULL); 
	fclose(fp);
	
	return model;
}

/*
	Load a dataset into a model
*/
void model_load_data(struct Model *model, const struct Data *training_set) {
	long k;
	if(training_set->name != NULL) {
		model->training_set_name = (char *)malloc(sizeof(char) * (strlen(training_set->name) + 1));
		strcpy(model->training_set_name, training_set->name);
	}
	model->nb_data = training_set->nb_data;
	model->dim_input = training_set->dim;
	model->y = training_set->y;	
	model->Q = training_set->Q;

	if(model->C == NULL) {
		model->C = (double*)malloc(sizeof(double) * (model->Q + 1));
		for (k=1;k<=model->Q;k++)
			model->C[k] = DEFAULT_C;
	}
	
	model->datatype = training_set->datatype;		
	
	switch(model->datatype) {
	case DATATYPE_DOUBLE:
		model->X_double = training_set->X_double;
		break;		
	case DATATYPE_FLOAT:
		model->X_float = training_set->X_float;
		break;
	case DATATYPE_INT:
		model->X_int = training_set->X_int;
		break;
	case DATATYPE_SHORT:
		model->X_short = training_set->X_short;
		break;
	case DATATYPE_BYTE:
		model->X_byte = training_set->X_byte;
		break;
	case DATATYPE_BIT:
		model->X_int = training_set->X_int;
		break;

	default:
		printf("Error: Unknown training set data format (%d).\n",model->datatype);
		exit(1);
		break;
	}
	model->X = training_set->X;
}

/*
	dataset_ptr = MSVM_make_dataset(data_file)
	
	Create an empty dataset and load data from data_file if data_file is not NULL
*/
struct Data *MSVM_make_dataset(char *data_file, enum Datatype datatype) {
	
	struct Data *dataset = (struct Data *)malloc(sizeof(struct Data));

	if(dataset != NULL) {
		dataset->name = NULL;
		dataset->X = NULL;
		dataset->y = NULL;
		dataset->dim = 0;
		dataset->nb_data = 0;
		dataset->Q = 0;
		
		dataset->X_double = NULL;
		dataset->X_float = NULL;
		dataset->X_int = NULL;
		dataset->X_short = NULL;
		dataset->X_byte = NULL;
		
		dataset->datatype = datatype;
	}
	
	if(data_file != NULL) {
		dataset->name = (char *)malloc(sizeof(char) * (strlen(data_file)+1));
		strcpy(dataset->name, data_file);
		dataset->Q = read_data(dataset, datatype, data_file);
	}

	// Return the pointer to the created dataset
	return dataset;
}

void free_dataset_data(struct Data *dataset) {
	if(dataset->X_double != NULL) 
		free_mat_aligned(dataset->X_double);
	
	if(dataset->X_float != NULL) 
		free_mat_f_aligned(dataset->X_float);

	if(dataset->X_int != NULL) 
		free_mat_i_aligned(dataset->X_int);

	if(dataset->X_short != NULL) 
		free_mat_s_aligned(dataset->X_short);

	if(dataset->X_byte != NULL) 
		free_mat_b_aligned(dataset->X_byte);
	
	dataset->X = NULL;
}	


void allocate_dataset(struct Data *dataset, const long nb_data, const long dim_input) {	
	
	if(dataset != NULL) {
		free_dataset_data(dataset);
			
		dataset->nb_data = nb_data;
		dataset->dim = dim_input;
		
		switch (dataset->datatype) {
		case DATATYPE_FLOAT:
				dataset->X_float = matrix_f_aligned(nb_data,dim_input);	
				dataset->X = (void **)dataset->X_float;	
				break;
		case DATATYPE_INT:
				dataset->X_int = matrix_i_aligned(nb_data,dim_input);
				dataset->X = (void **)dataset->X_int;		
				break;
		case DATATYPE_SHORT:
				dataset->X_short = matrix_s_aligned(nb_data,dim_input);
				dataset->X = (void **)dataset->X_short;		
				break;
		case DATATYPE_BYTE:
				dataset->X_byte = matrix_b_aligned(nb_data,dim_input);
				dataset->X = (void **)dataset->X_byte;		
				break;
		case DATATYPE_BIT:
				dataset->X_int = matrix_i_aligned(nb_data,dim_input/32 + (dim_input%32 != 0) );
				dataset->X = (void **)dataset->X_int;		
				break;
		
		case DATATYPE_DOUBLE:
				dataset->X_double = matrix_aligned(nb_data,dim_input);
				dataset->X = (void **)dataset->X_double;		
				break;	
			default:
				printf("Unknown data format in allocate_dataset().\n");
				exit(1);
		}

		//allocate_model_data(model_cv, nb_data_train[cv], training_set->dim);	
		dataset->y = (long *)malloc(sizeof(long) * (nb_data + 1));
	}
}

/*
	MSVM_delete_dataset(dataset_ptr)
	
	Delete a dataset
*/
void MSVM_delete_dataset(struct Data *dataset) {

	if(dataset == NULL)
		return;

	if(dataset->name != NULL) {
		free(dataset->name); 
	}	
	if(dataset->y != NULL) {
		free(dataset->y); 	
	}
		
	free_dataset_data(dataset);
	
	free(dataset);
	return;
}

/*
	Normalize a dataset by setting the mean and std of all columns of X to 0 and 1
	or according to an already defined model->normalization
	and returns the difference between the largest and smallest std.
	
	On output, model->normalization[1][j] contains the mean of the jth column
	       and model->normalization[2][j] contains the std of the jth column.
	
	If model is NULL the data are left unnormalized 
	(this can be used to check if normalization is needed). 	
	
*/
double MSVM_normalize_data(struct Data *dataset, struct Model *model) {
	
	if(dataset == NULL)
		return 0;
	
	const long nb_data = dataset->nb_data;
	const long dim = dataset->dim;
	double **X = dataset->X_double;
	float **Xf = dataset->X_float;
	long i,j;
	double mean,std, large_std=0, small_std=0;
	int compute_mean_std = 1;	
	
	switch(dataset->datatype) {
	case DATATYPE_FLOAT:
		if(model != NULL) {	
			if(model->normalization == NULL) {
				model->normalization = matrix(2,dim); 
				model->dim_input = dim;
			}
			else 
				compute_mean_std = 0;
		}
		if(compute_mean_std) {
			// Compute mean and std before normalizing		
			for(j=1;j<=dim;j++) {
				mean = 0.;
				std = 0.;
				for(i=1;i<=nb_data; i++) {
					mean += (double)Xf[i][j];
					std += (double)Xf[i][j]*Xf[i][j];
				}	
	
				mean /= (double)nb_data;
				std = sqrt( (std/(double)nb_data) - mean*mean);
				if(j==1 || std > large_std)
					large_std = std;
				if(j==1 || std < small_std)
					small_std = std;
				if(std == 0.0) 
					printf("Warning: std of feature #%ld = 0\n",j);
		
				if (model != NULL && std > 0.0) {
					// Do normalize
					for(i=1;i<=nb_data; i++) {
						Xf[i][j] = (Xf[i][j] - (float)mean) / (float)std;
					}
					model->normalization[1][j] = mean;
					model->normalization[2][j] = std;			
				}
			}
		}
		else {
			// Use given normalization coefficients 
			for(i=1;i<=nb_data; i++) {
				for(j=1;j<=dim;j++)
				    if(model->normalization[2][j] != 0.0)
					Xf[i][j] = (Xf[i][j] - (float)model->normalization[1][j]) / (float)model->normalization[2][j];
			}
		}
			
		break;
	case DATATYPE_INT:
		return 0;
		break;
	case DATATYPE_SHORT:
		return 0;
		break;
	case DATATYPE_BYTE:
		return 0;
		break;

	case DATATYPE_BIT:
	
		return 0;
		break;
		
	case DATATYPE_DOUBLE:
	default:
		if(model != NULL) {	
			if(model->normalization == NULL) {
				model->normalization = matrix(2,dim); 
				model->dim_input = dim;
			}
			else 
				compute_mean_std = 0;
		}
		if(compute_mean_std) {
			// Compute mean and std before normalizing		
			for(j=1;j<=dim;j++) {
				mean = 0.;
				std = 0.;
				for(i=1;i<=nb_data; i++) {
					mean += X[i][j];
					std += X[i][j]*X[i][j];
				}	
	
				mean /= (double)nb_data;
				std = sqrt( (std/(double)nb_data) - mean*mean);
				if(j==1 || std > large_std)
					large_std = std;
				if(j==1 || std < small_std)
					small_std = std;
				if(std == 0.0) 
					printf("Warning: std of feature #%ld = 0\n",j);
		
				if (model != NULL && std > 0.0) {
					// Do normalize
					for(i=1;i<=nb_data; i++) {
						X[i][j] = (X[i][j] - mean) / std;
					}
					model->normalization[1][j] = mean;
					model->normalization[2][j] = std;			
				}
			}
		}
		else {
			// Use given normalization coefficients 
			for(i=1;i<=nb_data; i++) {
				for(j=1;j<=dim;j++)
				    if(model->normalization[2][j] != 0.0) {
					X[i][j] = (X[i][j] - model->normalization[1][j]) / model->normalization[2][j];					
				}
			}
		}
		break;
	}	
			
	return (large_std - small_std);	
}


/*
	Load a dataset from a file
	Returns the number of classes computed as max(y_i)
*/
long read_data(struct Data *dataset, enum Datatype datatype, char *data_file)
{
	FILE *fs;
	long i,j,nb_data,dim_input;
	double value_y,value_X;
	float value_yf,value_Xf;
	int value_yi,value_Xi;
	short int value_Xs;
	unsigned char value_Xb;	
	long Q=0;
	long min_y = 1000;
	char buf[taille];
	long nr = 0;
	long dim;
	if((fs=fopen(data_file, "r"))==NULL)
	  {
	  printf("\nFile of data %s cannot be open.\n", data_file);
	  exit(0);
	  }

	nr += fscanf(fs, "%ld", &nb_data);
	nr += fscanf(fs, "%ld", &dim_input);

	switch(datatype) {
	
	case DATATYPE_FLOAT:
		// (Re)allocate memory
		free_dataset_data(dataset);
		dataset->X_float = matrix_f_aligned(nb_data, dim_input);

		// Read first line with data
		for(j=1; j<=dim_input; j++) {
		    nr += fscanf(fs, "%f", &value_Xf);
		    dataset->X_float[1][j] = value_Xf;
		}
		// Check if there is a label at the end of the line
		if(fgets(buf, taille, fs) == NULL) {
			printf("Error while reading data file.\n");
			exit(1);
		}
		if(sscanf(buf, "%f", &value_yf) > 0) {
			dataset->y = (long *) realloc(dataset->y, (nb_data+1) * sizeof(long));
			dataset->y[1] = (long) value_yf;
		}
		else if (dataset->y != NULL) {
			free(dataset->y);
			dataset->y = NULL;
		}
	
		// Read the rest of the file
		for(i=2; i<=nb_data; i++) {
		  for(j=1; j<=dim_input; j++) {
		    nr += fscanf(fs, "%f", &value_Xf);
		    dataset->X_float[i][j] = value_Xf;
		  }
		  if(dataset->y != NULL) {
			  nr += fscanf(fs, "%f", &value_yf);
			  dataset->y[i] = (long) value_yf;
			  if(dataset->y[i] > Q)
			  	Q = dataset->y[i];		  
			  if(dataset->y[i] < min_y)
			  	min_y = dataset->y[i];
		  }
		}
		// Set X to point to the FLOAT storage
		dataset->X = (void**)dataset->X_float;
		break;
		
			
	case DATATYPE_INT:
		// (Re)allocate memory
		free_dataset_data(dataset);
		dataset->X_int = matrix_i(nb_data, dim_input);

		// Read first line with data
		for(j=1; j<=dim_input; j++) {
		    nr += fscanf(fs, "%d", &value_Xi);
		    dataset->X_int[1][j] = value_Xi;
		}
		// Check if there is a label at the end of the line
		if(fgets(buf, taille, fs) == NULL) {
			printf("Error while reading data file.\n");
			exit(1);
		}
		if(sscanf(buf, "%d", &value_yi) > 0) {
			dataset->y = (long *) realloc(dataset->y, (nb_data+1) * sizeof(long));
			dataset->y[1] = (long) value_yi;
		}
		else if (dataset->y != NULL) {
			free(dataset->y);
			dataset->y = NULL;
		}
	
		// Read the rest of the file
		for(i=2; i<=nb_data; i++) {
		  for(j=1; j<=dim_input; j++) {
		    nr += fscanf(fs, "%d", &value_Xi);
		    dataset->X_int[i][j] = value_Xi;
		  }
		  if(dataset->y != NULL) {
			  nr += fscanf(fs, "%d", &value_yi);
			  dataset->y[i] = (long) value_yi;
			  if(dataset->y[i] > Q)
			  	Q = dataset->y[i];		  
			  if(dataset->y[i] < min_y)
			  	min_y = dataset->y[i];
		  }
		}
		// Set X to point to the INT storage
		dataset->X = (void**)dataset->X_int;
		break;
	case DATATYPE_SHORT:
		// (Re)allocate memory
		free_dataset_data(dataset);
		dataset->X_short = matrix_s_aligned(nb_data, dim_input);

		// Read first line with data
		for(j=1; j<=dim_input; j++) {
		    nr += fscanf(fs, "%hd", &value_Xs);
		    dataset->X_short[1][j] = value_Xs;
		}
		// Check if there is a label at the end of the line
		if(fgets(buf, taille, fs) == NULL) {
			printf("Error while reading data file.\n");
			exit(1);
		}
		if(sscanf(buf, "%d", &value_yi) > 0) {
			dataset->y = (long *) realloc(dataset->y, (nb_data+1) * sizeof(long));
			dataset->y[1] = (long) value_yi;
		}
		else if (dataset->y != NULL) {
			free(dataset->y);
			dataset->y = NULL;
		}
	
		// Read the rest of the file
		for(i=2; i<=nb_data; i++) {
		  for(j=1; j<=dim_input; j++) {
		    nr += fscanf(fs, "%hd", &value_Xs);
		    dataset->X_short[i][j] = value_Xs;
		  }
		  if(dataset->y != NULL) {
			  nr += fscanf(fs, "%d", &value_yi);
			  dataset->y[i] = (long) value_yi;
			  if(dataset->y[i] > Q)
			  	Q = dataset->y[i];		  
			  if(dataset->y[i] < min_y)
			  	min_y = dataset->y[i];
		  }
		}
		// Set X to point to the INT storage
		dataset->X = (void**)dataset->X_short;
		break;
		
	case DATATYPE_BYTE:
		// (Re)allocate memory
		free_dataset_data(dataset);
		dataset->X_byte = matrix_b_aligned(nb_data, dim_input);

		// Read first line with data
		for(j=1; j<=dim_input; j++) {
		    nr += fscanf(fs, "%hhu", &value_Xb);
		    dataset->X_byte[1][j] = value_Xb;
		}
		// Check if there is a label at the end of the line
		if(fgets(buf, taille, fs) == NULL) {
			printf("Error while reading data file.\n");
			exit(1);
		}
		if(sscanf(buf, "%d", &value_yi) > 0) {
			dataset->y = (long *) realloc(dataset->y, (nb_data+1) * sizeof(long));
			dataset->y[1] = (long) value_yi;
		}
		else if (dataset->y != NULL) {
			free(dataset->y);
			dataset->y = NULL;
		}
	
		// Read the rest of the file
		for(i=2; i<=nb_data; i++) {
		  for(j=1; j<=dim_input; j++) {
		    nr += fscanf(fs, "%hhu", &value_Xb);
		    dataset->X_byte[i][j] = value_Xb;
		  }
		  if(dataset->y != NULL) {
			  nr += fscanf(fs, "%d", &value_yi);
			  dataset->y[i] = (long) value_yi;
			  if(dataset->y[i] > Q)
			  	Q = dataset->y[i];		  
			  if(dataset->y[i] < min_y)
			  	min_y = dataset->y[i];
		  }
		}
		// Set X to point to the INT storage
		dataset->X = (void**)dataset->X_byte;		
		break;
		
	case DATATYPE_BIT:
		/*
			BIT data are loaded in INT storage
			(32 bits per int, completed with zeros)
		*/
		
		// How many INT needed to store the BITS ? 		
		dim = dim_input/32;
		if (dim_input%32 != 0)
			dim++;
					
		long j_int=0;
		unsigned int mask;
		
		// (Re)allocate memory
		free_dataset_data(dataset);
		dataset->X_int = matrix_i_aligned(nb_data, dim);

		// Read first line with data
		for(j=1; j<=dim_input; j++) {
		    if(j%32 == 1) {
		    	j_int++;
		    	dataset->X_int[1][j_int] = 0;
		    }
		    nr += fscanf(fs, "%d", &value_Xi);
		    if(value_Xi) {
		    	mask = 0x1;
	    		mask <<= (j%32) - 1;
		    	dataset->X_int[1][j_int] |= mask;
		    }
		}
		
		// Fill in rest of row with zeros
		for (j=0; j< 4 - j_int;j++ ) {
			dataset->X_int[1][j_int] = 0;
		}
		
		
		// Check if there is a label at the end of the line
		if(fgets(buf, taille, fs) == NULL) {
			printf("Error while reading data file.\n");
			exit(1);
		}
		if(sscanf(buf, "%d", &value_yi) > 0) {
			dataset->y = (long *) realloc(dataset->y, (nb_data+1) * sizeof(long));
			dataset->y[1] = (long) value_yi;
		}
		else if (dataset->y != NULL) {
			free(dataset->y);
			dataset->y = NULL;
		}
	
		// Read the rest of the file
		for(i=2; i<=nb_data; i++) {
		  j_int = 0;
		  for(j=1; j<=dim_input; j++) {
		      if(j%32 == 1) {
		    	j_int++;
		    	dataset->X_int[i][j_int] = 0;
		      }
		    	
		      nr += fscanf(fs, "%d", &value_Xi);
		      if(value_Xi) {
		    	mask = 0x1;
	    		mask <<= (j%32) - 1;
		    	dataset->X_int[i][j_int] |= mask;
		      }
		   
		  }
		  // Fill in rest of row with zeros
		  for (j=0; j< 4 - j_int;j++ ) {
			dataset->X_int[1][j_int] = 0;
		  }

		  if(dataset->y != NULL) {
			  nr += fscanf(fs, "%d", &value_yi);
			  dataset->y[i] = (long) value_yi;
			  if(dataset->y[i] > Q)
			  	Q = dataset->y[i];		  
			  if(dataset->y[i] < min_y)
			  	min_y = dataset->y[i];
		  }
		}
		// Set X to point to the INT storage
		dataset->X = (void**)dataset->X_int;
		break;
		
	case DATATYPE_DOUBLE:
	default:
		// (Re)allocate memory
		free_dataset_data(dataset);
		dataset->X_double = matrix_aligned(nb_data, dim_input);

		// Read first line with data
		for(j=1; j<=dim_input; j++) {
		    nr += fscanf(fs, "%lf", &value_X);
		    dataset->X_double[1][j] = value_X;
		}
		// Check if there is a label at the end of the line
		if(fgets(buf, taille, fs) == NULL) {
			printf("Error while reading data file.\n");
			exit(1);
		}
		if(sscanf(buf, "%lf", &value_y) > 0) {
			dataset->y = (long *) realloc(dataset->y, (nb_data+1) * sizeof(long));
			dataset->y[1] = (long) value_y;
		}
		else if (dataset->y != NULL) {
			free(dataset->y);
			dataset->y = NULL;
		}
	
		// Read the rest of the file
		for(i=2; i<=nb_data; i++) {
		  for(j=1; j<=dim_input; j++) {
		    nr += fscanf(fs, "%lf", &value_X);
		    dataset->X_double[i][j] = value_X;
		  }
		  if(dataset->y != NULL) {
			  nr += fscanf(fs, "%lf", &value_y);
			  dataset->y[i] = (long) value_y;
			  if(dataset->y[i] > Q)
			  	Q = dataset->y[i];		  
			  if(dataset->y[i] < min_y)
			  	min_y = dataset->y[i];
		  }
		}
		
		// Set X to point to the DOUBLE storage
		dataset->X = (void**)dataset->X_double;
		datatype = DATATYPE_DOUBLE;
		break;
	}
	fclose(fs);
	
	// Labels must be in the range [1,Q]
	if( min_y == 0 ) {
		for(i=1; i<=nb_data; i++)
			dataset->y[i]++;
		Q++;
	}
	else if (min_y < 0) {
		printf("\nWrong numbering of the categories in %s, minimal value is: %ld\n", data_file, min_y);
		exit(0);
	}

	if ((datatype != DATATYPE_BIT && nr < dim_input * nb_data) || (datatype == DATATYPE_BIT && nr < dim * nb_data)) {
		printf("\nError in reading file %s, not enough data (%ld < dim * nb_data).\n", data_file,nr);
		exit(0);
	}
		
	// Set the dimensions of the dataset
	dataset->nb_data = nb_data;
	dataset->dim = dim_input;
	
	dataset->datatype = datatype;
	
	return Q;
}

/* 
	Copy a data vector and its label from a data set to another
*/
void copy_data_point(struct Data *src, long src_idx, struct Data *dest, long dest_idx) {
	if (src == NULL || dest == NULL) {
		printf("Cannot copy data to or from a null data set.\n");
		return;
	}
	long dim = src->dim;
	enum Datatype datatype = src->datatype;
	if(dest->dim != dim || dest->datatype != datatype) {
		printf("Cannot copy data vector: inconsistent dimensions or datatype.\n");
		return;
	}
	
	long k;
	for (k=1; k<=dim; k++) {
		switch(datatype) {
		case DATATYPE_DOUBLE:
			dest->X_double[dest_idx][k] = src->X_double[src_idx][k];
			break;		
		case DATATYPE_FLOAT:
			dest->X_float[dest_idx][k] = src->X_float[src_idx][k];
			break;
		case DATATYPE_INT:
			dest->X_int[dest_idx][k] = src->X_int[src_idx][k];
			break;
		case DATATYPE_SHORT:
			dest->X_short[dest_idx][k] = src->X_short[src_idx][k];
			break;
		case DATATYPE_BYTE:
			dest->X_byte[dest_idx][k] = src->X_byte[src_idx][k];
			break;
		case DATATYPE_BIT:
			dest->X_int[dest_idx][k] = src->X_int[src_idx][k];
			break;

		default:
			printf("Error: Unknown data format (%d).\n",datatype);
			exit(1);
			break;
		}
	}
	dest->y[dest_idx] = src->y[src_idx];				
}

/*
	Load alpha from a file into a model
		and returns the maximum value of alpha
*/
double read_alpha(struct Model *model, char *alpha_file) {
	FILE *fs;
	long nr = 0;
	long i,k;
	long Q = model->Q;
	long nb_data = model->nb_data;
	double alpha, max_alpha=0.0;
	model->sum_all_alpha = 0;

	if((fs=fopen(alpha_file, "r"))==NULL)
	  {
	  printf("\nError: File of dual variables %s cannot be open.\n", alpha_file);
	  exit(0);
	  } 
	  
	if (model->alpha != NULL) {
		free(model->alpha[1]); free(model->alpha);
	}
	model->alpha = matrix(nb_data, Q);
	model->partial_average_alpha = (double *) realloc(model->partial_average_alpha, sizeof(double) * (nb_data+1));	

	for(i=1; i<=nb_data; i++)
	  {
	  model->partial_average_alpha[i] = 0.0;
	  for(k=1; k<=Q; k++)
	    {
	    nr += fscanf(fs, "%lf", &alpha);
	    model->alpha[i][k] = alpha;
	    model->sum_all_alpha += alpha;
	    model->partial_average_alpha[i] += alpha;

	    if(alpha > max_alpha)
	      max_alpha = alpha;
	    }
	  model->partial_average_alpha[i] /= Q;
	  }

	fclose(fs);
	if(nr < Q*nb_data)
		printf("\nError in reading alpha file %s, not enough data.\n", alpha_file);
		
	return max_alpha;
}

/*
	Converts a string to an MSVM type 
*/
enum MSVM_type str2model_type(char *str) {
	enum MSVM_type type;
	
	if(strcmp(str, "MSVM2") == 0)
		type = MSVM2;
	else if (strcmp(str, "WW") == 0)
		type = WW;
	else if (strcmp(str, "CS") == 0)
		type = CS;
	else if (strcmp(str, "LLW") == 0)
		type = LLW;
	else {
		printf("\nWarning: -m argument is not in the list of known MSVM types");
		printf("\n     (WW, CS, LLW, MSVM2). Falling back to MSVM2. \n");
		type = MSVM2;
	}
	
	return type;
}

/*

	k = ker(kernel_type, x1, x2, dim, kernel_par)

	General wrapper for the kernel functions	
	(which are defined in kernel.c)
*/
double ker(const enum Kernel_type type, const void *x1, const void *x2, const long dim, const double *par) {

	double res;
 
	switch(type)
	  {
	  case LINEAR: 
	  	res = dot_double((double *) x1, (double *) x2, dim);	  	
	  	break;
	  case RBF:
	  	res = rbf_double((double *) x1, (double *) x2, dim, par[1]); 
	  	break;
	  case POLY_H: 
	  	res = polynomial_homo_double((double *) x1, (double *) x2, dim, (int)par[1]);
	  	break;
	  case POLY: 
	  	res = polynomial_double((double *) x1, (double *) x2, dim, (int)par[1]);
	  	break;
	  case CUSTOM1:
	  	res = custom_kernel_1((double *) x1, (double *) x2, dim, par);
	  	break;	  	
	  case CUSTOM2:
	  	res = custom_kernel_2((double *) x1, (double *) x2, dim, par);
	  	break;	  	
	  case CUSTOM3:
	  	res = custom_kernel_3((double *) x1, (double *) x2, dim, par);
	  	break;
	  
	  // Kernel functions for FLOAT data type
  	  case LINEAR_FLOAT: 
	  	res = dot_float((float *) x1, (float *) x2, dim);	  	
	  	break;
	  case RBF_FLOAT:
	  	res = rbf_float((float *) x1, (float *) x2, dim, par[1]); 
	  	break;
	  case POLY_H_FLOAT: 
	  	res = polynomial_homo_float((float *) x1, (float *) x2, dim, (int)par[1]);
	  	break;
	  case POLY_FLOAT: 
	  	res = polynomial_float((float *) x1, (float *) x2, dim, (int)par[1]);
	  	break;
	  case CUSTOM1_FLOAT:
	  	res = custom_kernel_1_float((float *) x1, (float *) x2, dim, par);
	  	break;	  	
	  case CUSTOM2_FLOAT:
	  	res = custom_kernel_2_float((float *) x1, (float *) x2, dim, par);
	  	break;	  	
	  case CUSTOM3_FLOAT:
	  	res = custom_kernel_3_float((float *) x1, (float *) x2, dim, par);
	  	break;

	  // Kernel functions for INT data type
  	  case LINEAR_INT: 
	  	res = dot_int((int *) x1, (int *) x2, dim);	  	
	  	break;
	  case RBF_INT:
	  	res = rbf_int((int *) x1, (int *) x2, dim, par[1]); 
	  	break;
	  case POLY_H_INT: 
	  	res = polynomial_homo_int((int *) x1, (int *) x2, dim, (int)par[1]);
	  	break;
	  case POLY_INT: 
	  	res = polynomial_int((int *) x1, (int *) x2, dim, (int)par[1]);
	  	break;
	  case CUSTOM1_INT:
	  	res = custom_kernel_1_int((int *) x1, (int *) x2, dim, par);
	  	break;	  	
	  case CUSTOM2_INT:
	  	res = custom_kernel_2_int((int *) x1, (int *) x2, dim, par);
	  	break;	  	
	  case CUSTOM3_INT:
	  	res = custom_kernel_3_int((int *) x1, (int *) x2, dim, par);
	  	break;
	
	  // Kernel functions for SHORT INT data type
  	  case LINEAR_SHORT: 
	  	res = dot_short_int((short int *) x1, (short int *) x2, dim);	  	
	  	break;
  	  case RBF_SHORT: 
	  	res = rbf_short_int((short int *) x1, (short int *) x2, dim, par[1]);	  	
	  	break;
	  case POLY_H_SHORT: 
	  	res = polynomial_homo_short_int((short int *) x1, (short int *) x2, dim, (int)par[1]);
	  	break;
	  case POLY_SHORT: 
	  	res = polynomial_short_int((short int *) x1, (short int *) x2, dim, (int)par[1]);
	  	break;
	  case CUSTOM1_SHORT:
	  	res = custom_kernel_1_short_int((short int *) x1, (short int *) x2, dim, par);
	  	break;	  	
	  case CUSTOM2_SHORT:
	  	res = custom_kernel_2_short_int((short int *) x1, (short int *) x2, dim, par);
	  	break;	  	
	  case CUSTOM3_SHORT:
	  	res = custom_kernel_3_short_int((short int *) x1, (short int *) x2, dim, par);
	  	break;

	  // Kernel functions for BYTE data type
  	  case LINEAR_BYTE: 
	  	res = dot_byte((unsigned char *) x1, (unsigned char *) x2, dim);	  	
	  	break;
	  case RBF_BYTE:
	  	res = rbf_byte((unsigned char *) x1, (unsigned char *) x2, dim, par[1]); 
	  	break;
	  case POLY_H_BYTE: 
	  	res = polynomial_homo_byte((unsigned char *) x1, (unsigned char *) x2, dim, (int)par[1]);
	  	break;
	  case POLY_BYTE: 
	  	res = polynomial_byte((unsigned char *) x1, (unsigned char *) x2, dim, (int)par[1]);
	  	break;
	  case CUSTOM1_BYTE:
	  	res = custom_kernel_1_byte((unsigned char *) x1, (unsigned char *) x2, dim, par);
	  	break;	  	
	  case CUSTOM2_BYTE:
	  	res = custom_kernel_2_byte((unsigned char *) x1, (unsigned char *) x2, dim, par);
	  	break;	  	
	  case CUSTOM3_BYTE:
	  	res = custom_kernel_3_byte((unsigned char *) x1, (unsigned char *) x2, dim, par);
	  	break;
	  		  		
	  // Kernel functions for BIT data type
  	  case LINEAR_BIT: 
	  	res = dot_bit((int *) x1, (int *) x2, dim);	  	
	  	break;
	  case RBF_BIT:
	  	res = rbf_bit((int *) x1, (int *) x2, dim, par[1]); 
	  	break;
	  case POLY_H_BIT: 
	  	res = polynomial_homo_bit((int *) x1, (int *) x2, dim, (int)par[1]);
	  	break;
	  case POLY_BIT: 
	  	res = polynomial_bit((int *) x1, (int *) x2, dim, (int)par[1]);
	  	break;
	  case CUSTOM1_BIT:
	  	res = custom_kernel_1_bit((int *) x1, (int *) x2, dim, par);
	  	break;	  	
	  case CUSTOM2_BIT:
	  	res = custom_kernel_2_bit((int *) x1, (int *) x2, dim, par);
	  	break;	  	
	  case CUSTOM3_BIT:
	  	res = custom_kernel_3_bit((int *) x1, (int *) x2, dim, par);
	  	break;
	  				
	  default: 
	  	printf("\n\nUnknown kernel type (%d).\n\n", type); 
	  	exit(0);
	  }

	return res;
}


/*
	Set default kernel parameter values
	
	See kernel.h for the definition of these values
*/
void set_default_kernel_par(struct Model *model) {
	enum Kernel_type kernel_type = model->nature_kernel;
	
	// Use same defaults for DOUBLE, FLOAT, INT... data types
	kernel_type %= 10; 
		
	// But different defaults depending on the kernel type
	switch (kernel_type) {
		case LINEAR: 
			model->kernel_par = NULL;
			break;
		case RBF:
			model->kernel_par = (double*)realloc(model->kernel_par, sizeof(double) * 2);
			model->kernel_par[0] = 1;
			model->kernel_par[1] = DEFAULT_RBF_KERNEL_PAR;
			break;
		case POLY_H:
			model->kernel_par = (double*)realloc(model->kernel_par, sizeof(double) * 2);
			model->kernel_par[0] = 1;
			model->kernel_par[1] = DEFAULT_POLY_KERNEL_PAR;
			break;
		case POLY:
			model->kernel_par = (double*)realloc(model->kernel_par, sizeof(double) * 2);
			model->kernel_par[0] = 1;
			model->kernel_par[1] = DEFAULT_POLY_KERNEL_PAR;
			break;
		case CUSTOM1:
			model->kernel_par = (double*)realloc(model->kernel_par, sizeof(double) * 2);
			model->kernel_par[0] = 1;
			model->kernel_par[1] = DEFAULT_CUSTOM_KERNEL_1_PAR;
			break;
		case CUSTOM2:
			model->kernel_par = (double*)realloc(model->kernel_par, sizeof(double) * 2);
			model->kernel_par[0] = 1;
			model->kernel_par[1] = DEFAULT_CUSTOM_KERNEL_2_PAR;
			break;
		case CUSTOM3:
			model->kernel_par = (double*)realloc(model->kernel_par, sizeof(double) * 2);
			model->kernel_par[0] = 1;
			model->kernel_par[1] = DEFAULT_CUSTOM_KERNEL_3_PAR;
			break;

		default:
			// do nothing for an unknown kernel
			break;
	}	
}

/*
	Set the values of C_k in a model to C
	for the Q classes
	
	also set Q in the model
*/
void MSVM_model_set_C(double C, long Q, struct Model *model) {
	long k;
	model->C = (double*) realloc(model->C, sizeof(double) * (Q+1) );
	for(k=1;k<=Q;k++) {
		model->C[k] = C;
	}
	model->Q = Q;
}

/*
	Access typed data in model as a double 
	This function should not be used for intensive computations
*/
double model_get_Xij(struct Model *model, const long i, const long j) {
	int x_bit;
	unsigned int mask;
	
	switch(model->datatype) {
	case DATATYPE_DOUBLE:
		return model->X_double[i][j];
	case DATATYPE_FLOAT:
		return (double)model->X_float[i][j];
	case DATATYPE_INT:
		return (double)model->X_int[i][j];
	case DATATYPE_SHORT:
		return (double)model->X_short[i][j];
	case DATATYPE_BYTE:
		return (double)model->X_byte[i][j];
	case DATATYPE_BIT:
		x_bit = model->X_int[i][j/32];
		mask = 0x1;
		mask <<= j%32;		
		return (double)(x_bit & mask);
		
	default:
		printf("Cannot access model->X[i][j] with unknown data type in model.\n");
		exit(0);
	}
}


/*
	Ex.: i = check_argv(argc,argv,".model") 
	
	Check if any argument of the command line contains ".model"
	and returns the corresponding index of argv. 
	Returns 0 if ".model" cannot be found. 
*/
int check_argv(int argc, char *argv[], char *str) {
	int i;
	int arg_str = 0;
	for(i = 1; i < argc; i++)
		if(strstr(argv[i], str) != NULL ) {
			arg_str = i;
			break;
		}
		
	return arg_str;
}

/*
	Ex.: i = check_argv_eq(argc,argv,"-f") 
	
	Check if any argument of the command line is "-f"
	and returns the corresponding index of argv. 
	Returns 0 if "-f" cannot be found. 
*/
int check_argv_eq(int argc, char *argv[], char *str) {
	int i;
	int arg_str = 0;
	for(i = 1; i < argc; i++)
		if(strcmp(argv[i], str) == 0 ) {
			arg_str = i;
			break;
		}
		
	return arg_str;
}
/*
	Make a pause and wait for user to press RETURN 
*/
void Pause(char *message) {
	if(message!=NULL)
	  printf("%s\n",message);

	printf("\a""=> New Line to proceed... \n");
	while(getchar()!='\n');

	return;
}

/*
	Remove spaces from string
*/
void remove_spaces(char *string, char *str_nospaces, const int maxchar) {
	int i = 0;
	while(*string != '\0') {
	    if(*string != ' ' && i<maxchar-1) {
		*str_nospaces++ = *string;
		i++;	
	    }
      	    string++;
	}
	*str_nospaces = '\0';

	return;
		
}

/*
	Return the number of available processors
*/
int get_nprocessors(void) {
	int nprocs;
	
#ifdef _WIN32
	DWORD_PTR process_affinity_mask = 0;
	DWORD_PTR system_affinity_mask = 0;
	HANDLE current_process = GetCurrentProcess();

	GetProcessAffinityMask(current_process,
		&process_affinity_mask,
		&system_affinity_mask);
	/* On a dual-core Intel cpu both masks are 3 */
	int num_of_cores = 0;
	int i;
	for (i = 32; i > 0; i--)
	{
		if (process_affinity_mask & 1) num_of_cores++;
		process_affinity_mask >>= 1;
	}

	nprocs = num_of_cores;
	
#else
#ifdef _SC_NPROCESSORS_ONLN	// Linux
	nprocs = (int)sysconf(_SC_NPROCESSORS_ONLN);
#else				// BSD
	int mib[4];
	size_t len = sizeof(nprocs); 
	mib[0] = CTL_HW;
#ifdef HW_AVAILCPU
	mib[1] = HW_AVAILCPU;  
#else
	mib[1] = HW_NCPU;
#endif	
	sysctl(mib, 2, &nprocs, &len, NULL, 0);
#endif
#endif
	
	// Make sure nprocs >= 1
	if( nprocs < 1 ) 
		nprocs = 1;
	
	return nprocs;
}

/*
	Return the amount of physical memory 
	
	or -1 in case of failure 
		(information not available or overflow) 
*/
unsigned long long get_physical_memory(void) {
	
#ifdef _WIN32	
	unsigned long long mem = -1;
	MEMORYSTATUSEX status;
	status.dwLength = sizeof(status);
	GlobalMemoryStatusEx(&status);
	mem = (unsigned long long) status.ullTotalPhys;
	//mem = (unsigned long long) status.ullAvailPhys;
#else
#ifdef _SC_PHYS_PAGES	// Linux
	unsigned long mem = -1, pagesize = 0, nb_pages = 0;
	
	nb_pages = (unsigned long)sysconf(_SC_PHYS_PAGES);
	pagesize = (unsigned long)sysconf(_SC_PAGE_SIZE);
	
	mem = pagesize * nb_pages;
	
#else			
	unsigned long mem = -1;
	long mem_signed; 	
	int mib[2];
	size_t len = sizeof(mem_signed);
	mib[0] = CTL_HW;
	
	#ifdef HW_MEMSIZE	// OSX
	mib[1] = HW_MEMSIZE;
	sysctl(mib, 2, &mem_signed, &len, NULL, 0);
	mem = (unsigned long)mem_signed;	
	#else
	#ifdef HW_PHYSMEM64	// BSD
	mib[1] = HW_PHYSMEM64;
	sysctl(mib, 2, &mem_signed, &len, NULL, 0);
	mem = (unsigned long)mem_signed;
	#endif
	#endif
	
#endif
#endif
	if (mem < 0)
		mem = -1;	// failure due to overflow
		
	return mem;

}

/*
	Return the amount of available memory 
	(at the time of the call) 
	
	or -1 in case of failure 
		(information not available or overflow) 
*/
unsigned long long get_available_memory(void) {
	
#ifdef _WIN32	
	unsigned long long mem = -1;
	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof (statex);
	GlobalMemoryStatusEx(&statex);
	//nb_pages = (unsigned long)sysconf(_SC_AVPHYS_PAGES);
	//pagesize = (unsigned long)sysconf(_SC_PAGE_SIZE);
	
	mem = (unsigned long long) statex.ullAvailPageFile;// pagesize * nb_pages;
	
#else
#ifdef _SC_AVPHYS_PAGES	// Linux

	unsigned long mem = -1, pagesize = 0, nb_pages = 0;
	
	nb_pages = (unsigned long)sysconf(_SC_AVPHYS_PAGES);
	pagesize = (unsigned long)sysconf(_SC_PAGE_SIZE);
	
	mem = pagesize * nb_pages;
	
#else				// BSD
	// TO DO ... 
	unsigned long mem = -1; 
	/*
	int mib[2];
	size_t len;
	mib[0] = CTL_HW;
	mib[1] = HW_PAGESIZE;
	int pageSize_int;
	len = sizeof(pageSize_int);
	sysctl(mib, 2, &pageSize_int, &len, NULL, 0);
	pagesize = (unsigned long)pageSize_int;
	*/
#endif	
#endif
	
	if (mem < 0)
		mem = -1;	// failure due to overflow
		
	return mem;

}

#ifdef _WIN32	
DWORD GetThreadStackSize()
{
    SYSTEM_INFO systemInfo = {0};
    GetSystemInfo(&systemInfo);

    NT_TIB *tib = (NT_TIB*)NtCurrentTeb();
    DWORD_PTR stackBase = (DWORD_PTR)tib->StackBase;

    MEMORY_BASIC_INFORMATION mbi = {0};
    if (VirtualQuery((LPCVOID)(stackBase - systemInfo.dwPageSize), &mbi, sizeof(MEMORY_BASIC_INFORMATION)) != 0)
    {
        DWORD_PTR allocationStart = (DWORD_PTR)mbi.AllocationBase;
        return stackBase - allocationStart;
    }
    return 0;
}
#endif