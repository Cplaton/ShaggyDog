/**
 * @file    naive.c
 * @author  ShaggyPlaton
 * @brief   Naive Bayes library
 * @version 2.0
 * @date    December 2014
 **/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "naive.h"

float Calculate_Mean(float * value, int nb_value){
    int i; 
    float mean=0;
    for(i=0;i<nb_value;i++){
        mean+=value[i]/nb_value;
    }
    return mean ;
}

float Calculate_Variance(float * value, int nb_value, float mean){
    int i; 
    float var = 0;
    for(i=0;i<nb_value;i++){
        var+=(value[i]-mean)*(value[i]-mean)/nb_value;
    }
    return var;
}

double Gaussian(float x,float var, float mean){
    double res=0;
    if(var!=0){
        res = (1/(sqrt(((double)var*2*M_PI))))*exp((double)((-1)*(x-mean)*(x-mean))/(2*var));
    }else{
        res=-1000;
    }
    return res;
}

double proba_x_given_class(float x , float mean_class , float var_class, int nb_class){
    double res=0;
    res = Gaussian(x,var_class,mean_class);
    if(res==-1000){
//        printf("error in naive.c : variance is null\n");
        res=1;
    }else{
        res=res/(1/(double)nb_class);
    }
    return res;
}

sample * create_indiv(int class, float roll, float pitch, float vyaw, float vx, float vy, float vz, float ax, float ay, float az){
    sample * indiv = malloc(sizeof(sample));
    indiv->classe=class;
    indiv->feature[0]=roll;
    indiv->feature[1]=pitch;
    indiv->feature[2]=vyaw;
    indiv->feature[3]=vx;
    indiv->feature[4]=vy;
    indiv->feature[5]=vz;
    indiv->feature[6]=ax;
    indiv->feature[7]=ay;
    indiv->feature[8]=az;
    return indiv;
}

void destroy_indiv(sample * indiv){
    free(indiv);
}

void destroy_model(naive_model * model){
    free(model);
}

int find_class(int * tab, int val, int tab_size){
    int found = 0;
    int i;

    for(i=0;i<tab_size;i++){
        if(tab[i]==val){
            found=1;
        }
    }
    return found;
}

void naive_training(sample ** tab_indiv, int nb_indiv){
    FILE * fmodel;
    int * tab_class;
    int * tab_aux;
    int * nb_indiv_per_class;
    int * nb_indiv_aux ;
    int index_tab_class = 0;    
    int i,j,k;
    float m,v;
    struct s_values_tmp{
        int nr_class;
        int index;
        float * pitch;
        float * roll;
        float * vyaw;
        float * vx;
        float * vy;
        float * vz;
        float * ax;
        float * ay;
        float * az;
    }typedef values_tmp;

    tab_class=NULL;
    for(i=0;i<nb_indiv;i++){
        if(tab_class==NULL){
            tab_class=(int *)malloc(sizeof(int));
            nb_indiv_per_class=(int *)malloc(sizeof(int));
            tab_class[0]=tab_indiv[i]->classe;
            nb_indiv_per_class[0]=1;
            index_tab_class=1;
        }else{   
            if(find_class(tab_class,tab_indiv[i]->classe,index_tab_class)==0){
                index_tab_class++;
                tab_aux=(int *)malloc(sizeof(tab_class));
                nb_indiv_aux=(int *)malloc(sizeof(nb_indiv_per_class));
                for(j=0;j<index_tab_class-1;j++){
                    tab_aux[j]=tab_class[j];
                    nb_indiv_aux[j]=nb_indiv_per_class[j];
                }

                tab_class = (int *)malloc(sizeof(tab_class)+sizeof(int));
                nb_indiv_per_class = (int *)malloc(sizeof(nb_indiv_per_class)+sizeof(int));

                for(j=0;j<index_tab_class-1;j++){
                    tab_class[j]=tab_aux[j];
                    nb_indiv_per_class[j]=nb_indiv_aux[j];
                }
                tab_class[index_tab_class-1]=tab_indiv[i]->classe;
                nb_indiv_per_class[index_tab_class-1]=1;
                //free(tab_aux);
            }else{
                k=0;
                while(tab_class[k]!=tab_indiv[i]->classe){
                    k++;
                }
                nb_indiv_per_class[k]++;
            }
        }
    
    }
    printf("index=%d\n",index_tab_class);
    values_tmp tab_values[index_tab_class];

    for(i=0;i<index_tab_class;i++){
        tab_values[i].index=0;
        tab_values[i].nr_class=tab_class[i];
        printf("nb_indiv pour classe %d = %d\n",tab_class[i],nb_indiv_per_class[i]);
        tab_values[i].pitch = (float *)malloc(sizeof(float)*nb_indiv_per_class[i]);
        tab_values[i].roll = (float *)malloc(sizeof(float)*nb_indiv_per_class[i]);
        tab_values[i].vyaw = (float *)malloc(sizeof(float)*nb_indiv_per_class[i]);
        tab_values[i].vx = (float *)malloc(sizeof(float)*nb_indiv_per_class[i]);
        tab_values[i].vy = (float *)malloc(sizeof(float)*nb_indiv_per_class[i]);
        tab_values[i].vz = (float *)malloc(sizeof(float)*nb_indiv_per_class[i]);
        tab_values[i].ax = (float *)malloc(sizeof(float)*nb_indiv_per_class[i]);
        tab_values[i].ay = (float *)malloc(sizeof(float)*nb_indiv_per_class[i]);
        tab_values[i].az = (float *)malloc(sizeof(float)*nb_indiv_per_class[i]);
    }

    for(i=0;i<nb_indiv;i++){
       for(j=0;j<index_tab_class;j++){
            if(tab_indiv[i]->classe==tab_values[j].nr_class){
                tab_values[j].pitch[tab_values[j].index]=tab_indiv[i]->feature[0];
                tab_values[j].roll[tab_values[j].index]=tab_indiv[i]->feature[1];
                tab_values[j].vyaw[tab_values[j].index]=tab_indiv[i]->feature[2];
                tab_values[j].vx[tab_values[j].index]=tab_indiv[i]->feature[3];
                tab_values[j].vy[tab_values[j].index]=tab_indiv[i]->feature[4];
                tab_values[j].vz[tab_values[j].index]=tab_indiv[i]->feature[5];
                tab_values[j].ax[tab_values[j].index]=tab_indiv[i]->feature[6];
                tab_values[j].ay[tab_values[j].index]=tab_indiv[i]->feature[7];
                tab_values[j].az[tab_values[j].index]=tab_indiv[i]->feature[8];
                tab_values[j].index++;
                j=index_tab_class;
            }
       }
    }
    fmodel=fopen("naive_model","w+");
    fprintf(fmodel,"nb_cl %d\n",index_tab_class);
    fprintf(fmodel,"nb_feat 9\n");
    for(i=0;i<index_tab_class;i++){
        fprintf(fmodel,"%d ",tab_class[i]);
        m=Calculate_Mean(tab_values[i].pitch,nb_indiv_per_class[i]);
        v=Calculate_Variance(tab_values[i].pitch,nb_indiv_per_class[i],m);
        fprintf(fmodel,"%f %f ",m,v);
        m=Calculate_Mean(tab_values[i].roll,nb_indiv_per_class[i]);
        v=Calculate_Variance(tab_values[i].roll,nb_indiv_per_class[i],m);
        fprintf(fmodel,"%f %f ",m,v);
        m=Calculate_Mean(tab_values[i].vyaw,nb_indiv_per_class[i]);
        v=Calculate_Variance(tab_values[i].vyaw,nb_indiv_per_class[i],m);
        fprintf(fmodel,"%f %f ",m,v);
        m=Calculate_Mean(tab_values[i].vx,nb_indiv_per_class[i]);
        v=Calculate_Variance(tab_values[i].vx,nb_indiv_per_class[i],m);
        fprintf(fmodel,"%f %f ",m,v);
        m=Calculate_Mean(tab_values[i].vy,nb_indiv_per_class[i]);
        v=Calculate_Variance(tab_values[i].vy,nb_indiv_per_class[i],m);
        fprintf(fmodel,"%f %f ",m,v);
        m=Calculate_Mean(tab_values[i].vz,nb_indiv_per_class[i]);
        v=Calculate_Variance(tab_values[i].vz,nb_indiv_per_class[i],m);
        fprintf(fmodel,"%f %f ",m,v);
        m=Calculate_Mean(tab_values[i].ax,nb_indiv_per_class[i]);
        v=Calculate_Variance(tab_values[i].ax,nb_indiv_per_class[i],m);
        fprintf(fmodel,"%f %f ",m,v);
        m=Calculate_Mean(tab_values[i].ay,nb_indiv_per_class[i]);
        v=Calculate_Variance(tab_values[i].ay,nb_indiv_per_class[i],m);
        fprintf(fmodel,"%f %f ",m,v);
        m=Calculate_Mean(tab_values[i].az,nb_indiv_per_class[i]);
        v=Calculate_Variance(tab_values[i].az,nb_indiv_per_class[i],m);
        fprintf(fmodel,"%f %f\n",m,v);
    }
//    free(tab_values);
    printf("naive model created\n");
    fclose(fmodel);    
}

naive_model * read_Model(char * file_name){

    FILE * fmodel;
    naive_model * model;
    char * check;
    int nb_class, nb_feat, no_err,j;
    model=(naive_model *)malloc(sizeof(model));        
    
    fmodel = fopen(file_name,"r+");
    if(fmodel!=NULL){
        check=(char*)malloc(5);
        fscanf(fmodel,"%s %d",check,&nb_class);
        if(strcmp(check,"nb_cl")==0){
            model->nb_class=nb_class;
            no_err=1;
        }else{
            no_err=0;
        }
        if(no_err){
            check=realloc(check,7);
            fscanf(fmodel,"%s %d",check,&nb_feat);
            if(strcmp(check,"nb_feat")==0){
                model->nb_feature=nb_feat;
                no_err=1;
            }else{
                no_err=0;
            }
            if(no_err){
                //free(check);
                model->classe=(int *)malloc(sizeof(int)*model->nb_class);
                model->mean=(float **)malloc(sizeof(float)*model->nb_class);
                model->variance=(float **)malloc(sizeof(float)*model->nb_class);
                for(j=0;j<model->nb_class;j++){
                    model->mean[j]=(float *)malloc(sizeof(float)*model->nb_feature);
                    model->variance[j]=(float *)malloc(sizeof(float)*model->nb_feature);
                    fscanf(fmodel,"%d %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f",&model->classe[j], &model->mean[j][0], &model->variance[j][0], &model->mean[j][1], &model->variance[j][1], &model->mean[j][2], &model->variance[j][2], &model->mean[j][3], &model->variance[j][3], &model->mean[j][4], &model->variance[j][4], &model->mean[j][5], &model->variance[j][5], &model->mean[j][6], &model->variance[j][6], &model->mean[j][7], &model->variance[j][7], &model->mean[j][8], &model->variance[j][8]);
                }
            }else{
                model=NULL;
            }
        }
    }
    else{
        model=NULL;
    }
    return model;
}


int naive_predict(sample * indiv,naive_model * model){
   double posterior;
   double max = 0;
   int index = 0 ;
   int i,j;

   for(j=0;j<model->nb_class;j++){
       posterior = (double)1/(model->nb_class);
       for(i=0;i<model->nb_feature;i++){
           if(proba_x_given_class(indiv->feature[i],model->mean[j][i], model->variance[j][i], model->nb_class)>0.0000001){
               posterior=posterior*proba_x_given_class(indiv->feature[i],model->mean[j][i], model->variance[j][i], model->nb_class);
           }
       }
       //printf("ressemblance à la classe %d : %lf\n", model->classe[j],posterior);
       if(max<=posterior){
           max=posterior;
           index=j;
       }
   }
return model->classe[index];
}

int naive_predict_mean(sample * buffer, naive_model * model){
    int i,j,l;
    int * counters;
    int recog_values[10];
    counters = (int*) malloc(sizeof(int)*model->nb_class);
    for (l=0;l<0;l++){
        recog_values[l]=naive_predict(&buffer[l],model);
    }
    if (counters == NULL){
        printf("malloc rate");
        exit(1);
    }
    for(i=0;i<model->nb_class;i++){
        counters[i]=0;
    }
    for(j=0;j<model->nb_class;j++){
        for(i=0;i<10;i++){
            if (recog_values[i]==model->classe[j]){
                counters[j]++;
            }
        }
    }
    int max=0;
    int recog_class=0;
    for(j=0;j<model->nb_class;j++){
        if(counters[j]>max){
            max=counters[j];
            recog_class=model->classe[j];
        }
    }
    printf("nombre de classes naive : %d\n",model->nb_class);
    printf("Classe naivement reconnue : %d\n",recog_class);
    printf("Confiance naive: %lf\n ", 100*((double)max)/((double)l));
    free(counters);
    return recog_class;
}

/*

int main(){
    sample * indiv1;
    sample * indiv2;
    sample * indiv3;
    sample * indiv4;
    sample * indiv5;
    sample * indiv6;
    sample * indiv_unknown;
    naive_model * model;
    sample ** tab_indiv;


    indiv1 = create_indiv(0, 0.1 , 0.1 , 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1);
    indiv5 = create_indiv(0, 0.04 , 0.03 , 0 , 0.01, 0.014, 0.01, 0.014, 0.011, 0.09);
    indiv2 = create_indiv(2, 0.2 , 0.2 , 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2);
    indiv3 = create_indiv(1, 0.3 , 0.3 , 0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3);
    indiv6 = create_indiv(1, 0.35 , 0.33 , 0.34, 0.31, 0.29, 0.43, 0.38, 0.34, 0.33);
    indiv4 = create_indiv(2, 0.1 , 0.1 , 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1);
    indiv_unknown = create_indiv(-1, 0.15 , 0.15 , 0.15, 0.15, 0.15, 0.15, 0.15, 0.15, 0.15);

    tab_indiv = (sample **)malloc(sizeof(sample)*6);
    tab_indiv[0]=indiv1;
    tab_indiv[1]=indiv2;
    tab_indiv[2]=indiv3;
    tab_indiv[3]=indiv4;
    tab_indiv[4]=indiv5;
    tab_indiv[5]=indiv6;
    naive_training(tab_indiv, 6);
    model=read_Model("naive_model");
    int res;
    res=naive_predict(indiv_unknown,model);
    printf("classe reconnue : %d\n",res);
    return 0;
}

*/
