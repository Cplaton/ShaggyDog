/**
 * @file    svm-predict.h
 * @author  ShaggyDogs
 * @brief   prediction with svm
 * @version 1.0
 * @date    December 2014
 **/
#ifndef _PREDICT_H
#define _PREDICT_H
#include "classifier.h"
/**
 * @struct  specimen
 * @abstract   Structure that represents a sample for svm.
 **/
struct s_specimen{
    float pitch;    /**< Pitch angle of the sample. */
    float roll;     /**< Roll angle of the sample. */
    float vyaw;     /**< Yaw velocity of the sample. */
    float vx;     /**< Velocity in x of the sample. */
    float vy;     /**< Velocity in y of the sample. */
    float vz;     /**< Velocity in z of the sample. */
    float ax;     /**< Acceleration in x of the sample. */
    float ay;     /**< Acceleration in y of the sample. */
    float az;     /**< Acceleration in z of the sample. */
}typedef specimen;




/**
 * @brief   Externed array containing 10 samples to classify.
 **/
extern specimen specimen_buffer[10];


/**
 * @fn      recognition_process
 * @brief   prediction function with SVM.
 * @param   buffer  sample's buffer.
 * @param   training_model  path of the SVM model to use for classification. 
 * @return	Structure predict_results
 **/
predict_results recognition_process(specimen* buffer, char* training_model);

#endif /* _PREDICT_H */
