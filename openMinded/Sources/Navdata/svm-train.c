#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "svm.h"
#include "svm-train.h"
#define Malloc(type,n) (type *)malloc((n)*sizeof(type))

void read_problem(const char *filename);

void exit_input_errors(int line_num)
{
	fprintf(stderr,"Wrong input format at line %d\n", line_num);
	exit(1);
}

struct svm_parameter param;		// set by parse_command_line
struct svm_problem prob;		// set by _problem
struct svm_model *modele;
struct svm_node *x_space;
int cross_validation;
int nr_fold,nb_indiv;
FILE * base;
FILE * matlab;
static char *line = NULL;
static int max_line_len;

static char* readline(FILE *input)
{
	int len;

	if(fgets(line,max_line_len,input) == NULL)
		return NULL;

	while(strrchr(line,'\n') == NULL)
	{
		max_line_len *= 2;
		line = (char *) realloc(line,max_line_len);
		len = (int) strlen(line);
		if(fgets(line+len,max_line_len-len,input) == NULL)
			break;
	}
	return line;
}

double do_cross_validation()
{
	int i;
	int total_correct = 0;
	double total_error = 0;
	double sumv = 0, sumy = 0, sumvv = 0, sumyy = 0, sumvy = 0;
	double *target = Malloc(double,prob.l);
    double accuracy = 0;
    svm_cross_validation(&prob,&param,nr_fold,target);
	if(param.svm_type == EPSILON_SVR ||
	   param.svm_type == NU_SVR)
	{
		for(i=0;i<prob.l;i++)
		{
			double y = prob.y[i];
			double v = target[i];
			total_error += (v-y)*(v-y);
			sumv += v;
			sumy += y;
			sumvv += v*v;
			sumyy += y*y;
			sumvy += v*y;
		}
		printf("Cross Validation Mean squared error = %g\n",total_error/prob.l);
		printf("Cross Validation Squared correlation coefficient = %g\n",
			((prob.l*sumvy-sumv*sumy)*(prob.l*sumvy-sumv*sumy))/
			((prob.l*sumvv-sumv*sumv)*(prob.l*sumyy-sumy*sumy))
			);
	}
	else
	{
		for(i=0;i<prob.l;i++)
			if(target[i] == prob.y[i])
				++total_correct;
        if(prob.l!=0){
            accuracy = 100.0*total_correct/prob.l ;
        }else{
            accuracy = -1.0;
        }
		printf("Cross Validation Accuracy = %g%%\n",100.0*total_correct/prob.l);
	}
	free(target);
    return accuracy;
}

void create_model(int folds, float gamma, float C)
{
	// default values
	param.svm_type = C_SVC;
	param.kernel_type = RBF;
	param.degree = 3;
	param.gamma = gamma;	// Si =0, calculé automatiquement = 1/num_features
	param.coef0 = 0;
	param.nu = 0.5;
	param.cache_size = 40;
	param.C = C;
	param.eps = 0.001;
	param.p = 0.1;
	param.shrinking = 1;
	param.probability = 0;
	param.nr_weight = 0;
	param.weight_label = NULL;
	param.weight = NULL;
    if(folds>1){
	    cross_validation = 1;
        nr_fold = folds;
    }else{
        cross_validation = 0;
        nr_fold = 0 ;
    }
}

float * compute_parameters(char* training_set, int folds){
 
    char* input_file_name=training_set;
    const char *error_msg;
    float gamma,C,C_aux;
	float gamma_aux = 0.0;
    double aux, accuracy;
    float * param_aux=malloc(16);
    printf("dans compute, nb_indiv=%d",nb_indiv);
    aux = 0.0;
    C=1;
        gamma=nb_indiv/6;
        while(gamma<nb_indiv/3){
            create_model(folds,gamma,C);
            read_problem(input_file_name);
            error_msg = svm_check_parameter(&prob,&param);
            accuracy = do_cross_validation();
            if(accuracy > aux){
                aux=accuracy;
                gamma_aux=gamma;
                C_aux=C;
            }
            gamma++;
        }
    printf("\n");
    printf("final accuracy = %lf%%, C=%f, gamma=%f\n",aux,C_aux,gamma_aux);
    param_aux[0]=gamma_aux;
    param_aux[1]=C_aux;
    return param_aux;
}


void read_problem(const char *filename)
{
	int max_index, inst_max_index, i;
	size_t elements, j;
	FILE *fp = fopen(filename,"r");
	char *endptr;
	char *idx, *val, *label;

	if(fp == NULL)
	{
		fprintf(stderr,"can't open input file %s\n",filename);
		exit(1);
	}

	prob.l = 0;
	elements = 0;

	max_line_len = 1024;
	line = Malloc(char,max_line_len);
	while(readline(fp)!=NULL)
	{
		char *p = strtok(line," \t"); // label

		// features
		while(1)
		{
			p = strtok(NULL," \t");
			if(p == NULL || *p == '\n') // check '\n' as ' ' may be after the last feature
				break;
			++elements;
		}
		++elements;
		++prob.l;
	}
	rewind(fp);

	prob.y = Malloc(double,prob.l);
	prob.x = Malloc(struct svm_node *,prob.l);
	x_space = Malloc(struct svm_node,elements);

	max_index = 0;
	j=0;
	for(i=0;i<prob.l;i++)
	{
		inst_max_index = -1; // strtol gives 0 if wrong format, and precomputed kernel has <index> start from 0
		readline(fp);
		prob.x[i] = &x_space[j];
		label = strtok(line," \t\n");
		if(label == NULL) // empty line
			exit_input_errors(i+1);

		prob.y[i] = strtod(label,&endptr);
		if(endptr == label || *endptr != '\0')
			exit_input_errors(i+1);

		while(1)
		{
			idx = strtok(NULL,":");
			val = strtok(NULL," \t");

			if(val == NULL)
				break;

			errno = 0;
			x_space[j].index = (int) strtol(idx,&endptr,10);
			if(endptr == idx || errno != 0 || *endptr != '\0' || x_space[j].index <= inst_max_index)
				exit_input_errors(i+1);
			else
				inst_max_index = x_space[j].index;

			errno = 0;
			x_space[j].value = strtod(val,&endptr);
			if(endptr == val || errno != 0 || (*endptr != '\0' && !isspace(*endptr)))
				exit_input_errors(i+1);

			++j;
		}

		if(inst_max_index > max_index)
			max_index = inst_max_index;
		x_space[j++].index = -1;
	}

	if(param.gamma == 0 && max_index > 0)
		param.gamma = 1.0/max_index;

	if(param.kernel_type == PRECOMPUTED)
		for(i=0;i<prob.l;i++)
		{
			if (prob.x[i][0].index != 0)
			{
				fprintf(stderr,"Wrong input format: first column must be 0:sample_serial_number\n");
				exit(1);
			}
			if ((int)prob.x[i][0].value <= 0 || (int)prob.x[i][0].value > max_index)
			{
				fprintf(stderr,"Wrong input format: sample_serial_number out of range\n");
				exit(1);
			}
		}

	fclose(fp);
}

void training_model_generation(char* training_set, char* training_model, int folds, int nb_specimen)
{
	char* input_file_name=training_set;
	char* model_file_name=training_model;
	const char *error_msg;
    float * parameters;
    double accuracy;
    nb_indiv=nb_specimen;

    create_model(0,1,1);
	read_problem(input_file_name);
	error_msg = svm_check_parameter(&prob,&param);
    parameters = compute_parameters(training_set, folds);
    printf("gamma=%f C=%f\n",parameters[0],parameters[1]); 
    create_model(0,parameters[0],parameters[1]);
    printf("model créé\n");
    free(parameters);

	if(error_msg)
	{
		fprintf(stderr,"ERROR: %s\n",error_msg);
		exit(1);
	}

	if(cross_validation)
	{
		 accuracy = do_cross_validation();
	}
	else
	{
		modele = svm_train(&prob,&param);
		if(svm_save_model(model_file_name,modele))
		{
			fprintf(stderr, "can't save model to file %s\n", model_file_name);
			exit(1);
		}
		svm_free_and_destroy_model(&modele);
	}
	svm_destroy_param(&param);
	free(prob.y);
	free(prob.x);
	free(x_space);
	free(line);
}
