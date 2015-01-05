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

#define NB_LIGNE 464
#define K 3


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
}navdata;


navdata * load_data(char * nomFichier);

float euclideanDistance(navdata instance1, navdata instance2);

navdata * getNeighbors(navdata trainingSet[NB_LIGNE], navdata testInstance);

int getResponse (navdata neighbors[K]);

float getAccuracy(navdata * testSet, int * tab, int size);

int exists (int * tab, int val, int size);

int occurence_number (int * tab, int val, int size);

#endif


