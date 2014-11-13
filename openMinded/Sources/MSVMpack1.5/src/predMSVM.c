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
/*  Name           : predMSVM.c                                             */
/*  Version        : 2.0                                                    */
/*  Creation       : 04/27/10                                               */
/*  Last update    : 01/20/11                                               */
/*  Subject        : MSVMpack: a multi-class SVM package		    */
/*  Algo.          : Prediction command-line tool		            */
/*  Author         : Fabien Lauer fabien.lauer@loria.fr                     */
/*--------------------------------------------------------------------------*/
/*
	predMSVM.c -- Command line tool to classify a dataset with an M-SVM model
			 previously trained by trainmsvm from MSVMpack

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "libMSVM.h"
#include "libevalMSVM.h"

int main(int argc, char *argv[]) {

	char data_file[taille], outputs_file[taille], model_file[taille];
	
	int i,n_kernel_par,multipleC;
	long k;
	
	char *outputs = NULL;
	strcpy(model_file,"msvm.model");
	int nprocs = get_nprocessors();
	
	// Parse command line
	if(argc<2) {
		printf("MSVMpack prediction tool\n");
		printf("-- Version: %1.1f \n\n", VERSION);
		printf("Usage: \n");
		printf("\tpredmsvm data_file [MSVM.model] [pred.outputs] [-t N]\n\n");
		printf("computes the output of an M-SVM on the given data_file\n");
		printf("using N processors (default N = %d on this computer).\n\n",nprocs);
		printf(" Or:\n\tpredmsvm -i [MSVM.model] \n\n");		
		printf("prints model information.\n\n");		

		exit(0);
	}
	
	strcpy(data_file, argv[1]); 	// Data file must be the first argument
					// except for the -i option
	i = check_argv(argc,argv, ".model");
	if(i) {
		strcpy(model_file,argv[i]);
	}
	i = check_argv(argc,argv, ".outputs");
	if(i) {
		strcpy(outputs_file,argv[i]);
		outputs = outputs_file;
	}
	
	// Load model
	struct Model *model;
	model = MSVM_load_model(model_file);
	if (model == NULL)
		exit(1);
		
	// Check -i option and print model information if needed
	i = check_argv(argc,argv, "-i");
	if(i) {
		printf("\n%s\t : ", model_file);
		switch(model->type) {
			case MSVM2 :
				printf("M-SVM^2 (MSVM2)\n");
				break;
			case WW :
				printf("Weston and Watkins M-SVM (WW) \n");
				break;
			case CS :
				printf("Crammer and Singer M-SVM (CS) \n");
				break;
			case LLW :
				printf("Lee, Lin and Wahba M-SVM (LLW) \n");
				break;
				
			default :
				printf("Unknown\n");
				break;
		}
		printf(" version\t : MSVMpack %1.1f\n",model->version);
		printf(" training set\t : %s",model->training_set_name);
		if(model->normalization != NULL)
			printf("\t (data normalized)\n");
		else
			printf("\n");
		
		printf(" training error\t : %3.2lf %%\n",100 * model->training_error);
		
		enum Kernel_type kerneltype = model->nature_kernel;
		switch(model->datatype) {
		case DATATYPE_DOUBLE:
				printf(" data format\t : double precision floating-point (64-bit)\n");
				break;
		case DATATYPE_FLOAT:
				printf(" data format\t : single precision floating-point (32-bit)\n");
				kerneltype -= 10;
				break;
		case DATATYPE_INT:
				printf(" data format\t : integer (32-bit)\n");
				kerneltype -= 20;
				break;
		case DATATYPE_SHORT:
				printf(" data format\t : short integer (16-bit)\n");
				kerneltype -= 30;
				break;
		case DATATYPE_BYTE:
				printf(" data format\t : byte (8-bit unsigned integer)\n");
				kerneltype -= 40;
				break;
		case DATATYPE_BIT:
				printf(" data format\t : 0-1 bit \n");
				kerneltype -= 50;
				break;

			default:
				printf(" data format\t : unknown (%d)\n",model->datatype);
		}
		
		printf("\t Q\t : %ld\n",model->Q);

		n_kernel_par = 0;
		if(model->kernel_par != NULL )
			n_kernel_par = (int)model->kernel_par[0];
			
		switch(kerneltype) {
			case LINEAR:
				printf("\t kernel\t : linear\n");
				break;
			case RBF:
				printf("\t kernel\t : Gaussian RBF (sigma = %lf)\n", model->kernel_par[1]);
				break;
			case POLY_H:
				printf("\t kernel\t : Homogeneous polynomial (degree = %d)\n", (int)model->kernel_par[1]);
				break;
			case POLY:
				printf("\t kernel\t : Non-homogeneous polynomial (degree = %d)\n", (int)model->kernel_par[1]);
				break;
			case CUSTOM1:
				if(n_kernel_par==0)
					printf("\t kernel\t : %s (no parameter)",CUSTOM_KERNEL_1_NAME);
				else if (n_kernel_par==1)
					printf("\t kernel\t : %s (parameter = %lf)\n", CUSTOM_KERNEL_1_NAME,model->kernel_par[1]);
				else if (model->kernel_par[0]<5) {
					printf("\t kernel\t : %s (parameters = %lf", CUSTOM_KERNEL_1_NAME,model->kernel_par[1]);
					for(i=2;i<=n_kernel_par;i++)
						printf(" %lf", model->kernel_par[i]);
					printf(")\n");
				}				
				else
					printf("\t kernel\t : %s (%d parameters)\n", CUSTOM_KERNEL_1_NAME,n_kernel_par);
				
				break;				
			case CUSTOM2:
				if(n_kernel_par==0)
					printf("\t kernel\t : %s (no parameter)",CUSTOM_KERNEL_2_NAME);		
				else if (n_kernel_par==1)
					printf("\t kernel\t : %s (parameter = %lf)\n", CUSTOM_KERNEL_2_NAME, model->kernel_par[1]);
				else if (model->kernel_par[0]<5) {
					printf("\t kernel\t : %s (parameters = %lf", CUSTOM_KERNEL_2_NAME, model->kernel_par[1]);
					for(i=2;i<=n_kernel_par;i++)
						printf(" %lf", model->kernel_par[i]);
					printf(")\n");
				}				
				else
					printf("\t kernel\t : %s (%d parameters)\n", CUSTOM_KERNEL_2_NAME,n_kernel_par);
				break;
			case CUSTOM3:
				if(n_kernel_par==0)
					printf("\t kernel\t : %s (no parameter)",CUSTOM_KERNEL_3_NAME);
				else if (n_kernel_par==1)
					printf("\t kernel\t : %s (parameter = %lf)\n", CUSTOM_KERNEL_3_NAME,model->kernel_par[1]);
				else if (model->kernel_par[0]<5) {
					printf("\t kernel\t : %s (parameters = %lf", CUSTOM_KERNEL_3_NAME,model->kernel_par[1]);
					for(i=2;i<=n_kernel_par;i++)
						printf(" %lf", model->kernel_par[i]);
					printf(")\n");
				}				
				else
					printf("\t kernel\t : %s (%d parameters)\n", CUSTOM_KERNEL_3_NAME,n_kernel_par);
				break;
			default:
				printf("\t kernel\t : unknown\n");
				break;
		}
		
		multipleC = 0;
		for(k=2;k<=model->Q;k++) 
			if(model->C[k] != model->C[1])
				multipleC = 1;
		if(multipleC) {
			for(k=1;k<=model->Q;k++) {
				if(model->C[k] > 0.001)
					printf("\t C_%ld\t : %1.3lf\n",k,model->C[k]);
				else
					printf("\t C_%ld\t : %1.3e\n",k,model->C[k]);
			}			
		}
		else {
			if(model->C[1] > 0.001)
				printf("\t C\t : %1.3lf\n",model->C[1]);
			else
				printf("\t C\t : %1.3e\n",model->C[1]);
		}
		
		printf("\t nSV\t : %ld\n",model->nb_data);
		exit(0);
	}
	
	// Load data
	struct Data *data_set;
	data_set = MSVM_make_dataset(data_file,model->datatype);
	
	if(data_set->dim != model->dim_input) {
		printf("Error: wrong data dimension (%ld) for model %s\n", data_set->dim, model_file);
		printf("\t (expected input dimension is %ld).\n",model->dim_input);
		exit(1);
	} 	
	
	// Set number of threads / CPUs
	i = check_argv(argc,argv, "-t");
	if(i) {
		if(atoi(argv[i+1]) > nprocs) {			
			printf("Unattended request of a number of working threads above the number of available processors (%d)\n",nprocs);
		}
		else
			nprocs = atoi(argv[i+1]);
	}
	
	// Predict	
	MSVM_classify_set(NULL, data_set->X, data_set->y, data_set->nb_data, outputs, model, nprocs);
	
	// Free memory
	MSVM_delete_dataset(data_set);
	MSVM_delete_model_with_data(model);

	return 0;	
}

