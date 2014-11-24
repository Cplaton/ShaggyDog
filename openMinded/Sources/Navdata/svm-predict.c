#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "svm.h"
#include "svm-predict.h"

int print_null(const char *s,...) {return 0;}

static int (*info)(const char *fmt,...) = &printf;

specimen specimen_buffer[10];

struct svm_node *x;
int max_nr_attr = 64;

struct svm_model* modell;
int predict_probability=0;

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

void exit_input_error(int line_num)
{
	fprintf(stderr,"Wrong input format at line %d\n", line_num);
	exit(1);
}

<<<<<<< HEAD
int predict(specimen* buffer, FILE *output)
=======
int predict(FILE *input, FILE *output)
>>>>>>> b4503d94bff23e3c5e7e222523f93df51230d134
{
	int correct = 0;
	double error = 0;
	double sump = 0, sumt = 0, sumpp = 0, sumtt = 0, sumpt = 0;

	int svm_type=svm_get_svm_type(modell);
	int nr_class=svm_get_nr_class(modell);
	double *prob_estimates=NULL;
	int j,i,l;
	int *labels;
	double recog_values[50];
<<<<<<< HEAD
=======
	
	if(predict_probability)
	{
		if (svm_type==NU_SVR || svm_type==EPSILON_SVR)
			info("Prob. model for test data: target value = predicted value + z,\nz: Laplace distribution e^(-|z|/sigma)/(2sigma),sigma=%g\n",svm_get_svr_probability(modell));
		else
		{
			labels=(int *) malloc(nr_class*sizeof(int));
			svm_get_labels(modell,labels);
			prob_estimates = (double *) malloc(nr_class*sizeof(double));
			fprintf(output,"labels");
			for(j=0;j<nr_class;j++)
				fprintf(output," %d",labels[j]);
			fprintf(output,"\n");
			//free(labels);
		}
	}
	
	// pour avoir les labels
	labels=(int *) malloc(nr_class*sizeof(int));
	svm_get_labels(modell,labels);
>>>>>>> b4503d94bff23e3c5e7e222523f93df51230d134

	max_line_len = 1024;
	line = (char *)malloc(max_line_len*sizeof(char));

	for(l=0;l<10;l++)
	{
		int i = 0;
		double predict_label;
		char *idx, *val, *label, *endptr;
		int inst_max_index = -1; // strtol gives 0 if wrong format, and precomputed kernel has <index> start from 0

		labels=(int *) malloc(nr_class*sizeof(int));
        svm_get_labels(modell,labels);

		for(i=0;i<9;i++)
		{
            x = (struct svm_node *) malloc(10*sizeof(struct svm_node));
			x[i].index = i;

			switch(i)
			{
			    case 0:x[i].value = buffer[l].pitch;
			    break;
                case 1:x[i].value = buffer[l].roll;
			    break;
                case 2:x[i].value = buffer[l].vyaw;
			    break;
                case 3:x[i].value = buffer[l].vx;
			    break;
                case 4:x[i].value = buffer[l].vy;
			    break;
                case 5:x[i].value = buffer[l].vz;
			    break;
                case 6:x[i].value = buffer[l].ax;
			    break;
                case 7:x[i].value = buffer[l].ay;
			    break;
                case 8:x[i].value = buffer[l].az;
			    break;
                case 9:x[i].index = -1;
			    break;
			}
		}
        predict_label = svm_predict(modell,x);
        fprintf(output,"%g\n",predict_label);

		// affiche le label
		printf("%lf\n",predict_label);
		// enregistrement des labels reconnus dans un tableau
		recog_values[l] = predict_label;
	}

	// traitement des labels reconnus
	int * counters;

	counters = (int*) malloc(sizeof(int)*nr_class);
	if (counters == NULL)
	{
		printf("malloc rate");
		exit(1);
	}

	// init counter
	for (i=0;i<nr_class;i++)
	{
		counters[i]=0;
	}

	// compte le nombre d'apparitions pour chaque classe reconnue
	for (j=0;j<nr_class;j++)
	{
<<<<<<< HEAD
		for (i=0;i<l;i++)
=======
		for (i=0;i<total;i++)
>>>>>>> b4503d94bff23e3c5e7e222523f93df51230d134
		{
			if (recog_values[i]==labels[j])
			{
				counters[j]++;
			}
		}
	}

	int max = 0;
	int recog_class;
	for (j=0;j<nr_class;j++)
	{
		if (counters[j]>max)
		{
			max = counters[j];
			recog_class = labels[j];
		}
	}
<<<<<<< HEAD

	//free(labels);
	//free(counters);
	printf("Classe reconnue : %d\n",recog_class);
	printf("Accuracy : %lf\n ", 100*((double)max)/((double)l));

    return(recog_class);
=======
	
	/* --------- DEBUG ---------
	printf("labels existants\n");
	for (i=0;i<nr_class;i++){
		printf("label %d : %d\n", i, labels[i]);
	}
	
	printf("-------------------------\n");
	
	printf("tableau des valeurs reconnues\n");
	for (i=0;i<total;i++){
		printf("recog value %d : %lf\n", i, recog_values[i]);
	}
	
	printf("-------------------------\n");
	printf("Valeurs des compteurs\n");
	for (i=0;i<nr_class;i++)
	{
		printf("counter %d : %d\n",i,counters[i]);
	}
	printf("nr_class : %d\n",nr_class);
	printf("max : %d\n",max);
	printf("total : %d\n",total);
	
	//free(labels);
	//free(counters);
	printf("Classe reconnue : %d\n",recog_class);
	printf("Accuracy : %lf ", 100*((double)max)/((double)total));
	*/
	return recog_class;
	
	/*
	if (svm_type==NU_SVR || svm_type==EPSILON_SVR)
	{
		info("Mean squared error = %g (regression)\n",error/total);
		info("Squared correlation coefficient = %g (regression)\n",
			((total*sumpt-sump*sumt)*(total*sumpt-sump*sumt))/
			((total*sumpp-sump*sump)*(total*sumtt-sumt*sumt))
			);
	}
	else
		// info("Accuracy = %g%% (%d/%d) (classification)\n",
		// 		(double)correct/total*100,correct,total);
	if(predict_probability)
		free(prob_estimates);*/
>>>>>>> b4503d94bff23e3c5e7e222523f93df51230d134
}

void exit_with_help()
{
	printf(
	"Usage: svm-predict [options] test_file model_file output_file\n"
	"options:\n"
	"-b probability_estimates: whether to predict probability estimates, 0 or 1 (default 0); for one-class SVM only 0 is supported\n"
	"-q : quiet mode (no outputs)\n"
	);
	exit(1);
}

int recognition_process(specimen* buffer, char* training_model, char* class_out)
{
	FILE *output;
	int recog_class;

	output = fopen(class_out,"w");
	if(output == NULL)
	{
		fprintf(stderr,"can't open output file %s\n",class_out);
		exit(1);
	}

	if((modell=svm_load_model(training_model))==0)
	{
		fprintf(stderr,"can't open model file %s\n",training_model);
		exit(1);
	}

	x = (struct svm_node *) malloc(max_nr_attr*sizeof(struct svm_node));
<<<<<<< HEAD

    if(svm_check_probability_model(modell)!=0)
        info("Model supports probability estimates, but disabled in prediction.\n");

	recog_class =predict(&buffer[10],output);
=======
	if(predict_probability)
	{
		if(svm_check_probability_model(modell)==0)
		{
			fprintf(stderr,"Model does not support probabiliy estimates\n");
			exit(1);
		}
	}
	else
	{
		if(svm_check_probability_model(modell)!=0)
			info("Model supports probability estimates, but disabled in prediction.\n");
	}
	
	int recog_class;
	recog_class = predict(input,output);
>>>>>>> b4503d94bff23e3c5e7e222523f93df51230d134
	svm_free_and_destroy_model(&modell);
	free(x);
	free(line);
	fclose(output);
	return recog_class;
}

/*int main(int argc, char **argv)
{
	FILE *input, *output;
	int i;
	// parse options
	for(i=1;i<argc;i++)
	{
		if(argv[i][0] != '-') break;
		++i;
		switch(argv[i-1][1])
		{
			case 'b':
				predict_probability = atoi(argv[i]);
				break;
			case 'q':
				info = &print_null;
				i--;
				break;
			default:
				fprintf(stderr,"Unknown option: -%c\n", argv[i-1][1]);
				exit_with_help();
		}
	}

	if(i>=argc-2)
		exit_with_help();

	input = fopen(argv[i],"r");
	if(input == NULL)
	{
		fprintf(stderr,"can't open input file %s\n",argv[i]);
		exit(1);
	}

	output = fopen(argv[i+2],"w");
	if(output == NULL)
	{
		fprintf(stderr,"can't open output file %s\n",argv[i+2]);
		exit(1);
	}

	if((model=svm_load_model(argv[i+1]))==0)
	{
		fprintf(stderr,"can't open model file %s\n",argv[i+1]);
		exit(1);
	}

	x = (struct svm_node *) malloc(max_nr_attr*sizeof(struct svm_node));
	if(predict_probability)
	{
		if(svm_check_probability_model(model)==0)
		{
			fprintf(stderr,"Model does not support probabiliy estimates\n");
			exit(1);
		}
	}
	else
	{
		if(svm_check_probability_model(model)!=0)
			info("Model supports probability estimates, but disabled in prediction.\n");
	}

	predict(input,output);
	svm_free_and_destroy_model(&model);
	free(x);
	free(line);
	fclose(input);
	fclose(output);
	return 0;
}*/
