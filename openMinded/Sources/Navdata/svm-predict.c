#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "svm.h"
#include "svm-predict.h"

int print_null(const char *s,...) {return 0;}

static int (*info)(const char *fmt,...) = &printf;

//specimen specimen_buffer[10];

//struct svm_node *x;
int max_nr_attr = 64;

struct svm_model* modell;
int predict_probability=0;

static int max_line_len;

void exit_input_error(int line_num)
{
    fprintf(stderr,"Wrong input format at line %d\n", line_num);
    exit(1);
}

predict_results predict(specimen * buffer)
{
    predict_results res;
    int nr_class=svm_get_nr_class(modell);
    int j,i,l;
    int *labels;
    double recog_values[50];
    double predict_label;
    
    struct svm_node x[10];
    
    
    //	printf("dans predict\n");
    // pour avoir les labels
    labels=(int *) malloc(nr_class*sizeof(int));
    svm_get_labels(modell,labels);
    //printf("apres get labels\n");
    
    max_line_len = 1024;
    //	printf("apres realloc\n");
    for(l=0;l<10;l++) // pour parcourir le tableau de structure
    {
        
        //x = (struct svm_node *) malloc(10*sizeof(struct svm_node));
        for(i=0;i<10;i++) // pour parcourir la structure
        {
            x[i].index = i+1;
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
            //printf("%d : %lf\n",x[i].index,x[i].value);
        }
        //		printf("apres tableau\n");
        predict_label = svm_predict(modell,x);
        //		printf("apres call svm predict(modell,x)\n");
        recog_values[l] = predict_label;
        //free(x);
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
        for (i=0;i<l;i++)
        {
            if (recog_values[i]==labels[j])
            {
                counters[j]++;
            }
        }
    }
    
    int max = 0;
    int recog_class = 0;
    for (j=0;j<nr_class;j++)
    {
        if (counters[j]>max)
        {
            max = counters[j];
            recog_class = labels[j];
        }
    }
    
    printf("nombre de classes : %d\n",nr_class);
    printf("Classe reconnue : %d\n",recog_class);
    printf("Confiance : %lf\n ", 100*((double)max)/((double)l));
    
    free(labels);
    free(counters);
    res.predict_class = recog_class;
    res.confidence = (int)100*((double)max)/((double)l);
    
    return res;
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

predict_results recognition_process(specimen* buffer, char* training_model)
{
    predict_results res;
    if((modell=svm_load_model(training_model))==0)
    {
        fprintf(stderr,"can't open model file %s\n",training_model);
        exit(1);
    }
    
    if(svm_check_probability_model(modell)!=0)
        info("Model supports probability estimates, but disabled in prediction.\n");
    
    res =predict(buffer);
    
    svm_free_and_destroy_model(&modell);
    return res;
}