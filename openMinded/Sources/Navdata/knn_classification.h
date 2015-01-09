/**
 * @file    knn_classification.h
 * @author  OussamaBenShaggy
 * @brief   KNN library
 * @version 1.0
 * @date    December 2014
 **/

#ifndef _KNN_CLASSIFICATION_H
#define _KNN_CLASSIFICATION_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define K 3
#define Buffer_Size 3
#define Trained_Class_Nb 3

typedef struct {

	float pitch;
	float roll;
	float vyaw;
	float vx;
	float vy;
	float vz;
	float ax;
	float ay;
	float az;
	int  class_id;
	int nb_indiv;
}indiv_knn;


indiv_knn * load_data(char * nomFichier);

float euclideanDistance(indiv_knn instance1, indiv_knn instance2);

indiv_knn * getNeighbors(indiv_knn * trainingSet, indiv_knn testInstance);

int getResponse (indiv_knn neighbors[K]);

int getResponse_mean (int buffer[Buffer_Size]);

int occurence_number (int * tab, int val, int size);

#endif


