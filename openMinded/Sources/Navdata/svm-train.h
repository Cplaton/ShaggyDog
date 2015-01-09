/**
 * @file    svm-train.h
 * @author  ShaggyDogs
 * @brief   prediction with svm
 * @version 1.0
 * @date    December 2014
 **/
#ifndef _TRAIN_H
#define _TRAIN_H

/**
 * @fn      training_model_generation
 * @brief   function generating a SVM model from training samples.
 * @param   training_set    path to the file containing the learning base. 
 * @param   training_model  path to the
 * @param 
 * @param
 * @return
 **/
void training_model_generation(char* training_set, char* training_model, int folds, int nb_specimen);

#endif /* _TRAIN_H */
