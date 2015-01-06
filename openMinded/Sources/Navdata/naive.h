/**
 * @file    naive.h
 * @author  ShaggyPlaton
 * @brief   Naive Bayes library
 * @version 2.0
 * @date    December 2014
 **/

#ifndef _NAIVE_H_
#define _NAIVE_H_

/**
* @struct  sample
* @abstract    Struct representing a specimen.
**/
struct s_sample{
    int classe;         /**< Classe of the specimen. */
    float feature[9];   /**< Array of the features' values. */
}typedef sample;

/**
* @struct  naive_model
* @abstract    Struct representing the model a the naive bayes classifier.
**/
struct s_naive_model{
    int nb_class;       /**< Number of different classes. */
    int nb_feature;     /**< Number of different features. */
    int * classe;       /**< Array composed by the ID of each class. */
    float ** mean;      /**< Matrix of means. */
    float ** variance;  /**< Matrix of variances. */
}typedef naive_model;


/**
* @brief   Trains the naive bayes classifier, creates a file naive_model in the current directory. 
* @param   tab_indiv   Array of sample, i.e. array composed of all the specimens of the learning base. 
* @param   nb_indiv   Integer representing the number of specimens, i.e. the size of the previous array.
**/

void naive_training(sample ** tab_indiv, int nb_indiv);

/**
* @brief   Reads the naive bayes model, creates a structure. 
* @param   file_name   String representing the address of the model file.
* @return  naive_model Structure naive_model.
**/
naive_model * read_Model(char * file_name);
    
/**
* @brief   classify a given specimen with a given model. 
* @param   indiv   Sample structure's pointer, specimen to classify. 
* @param   model   naive_model structure's pointer, model to use for classification.
* @return  Integer representing the predicted class.
**/
int naive_predict(sample * indiv,naive_model * model);

/**
* @brief   classify a given specimens' buffer with a given model. 
* @param   indiv   Sample structure's pointer, specimen to classify. 
* @param   model   naive_model structure's pointer, model to use for classification.
* @return  Integer representing the predicted class.
**/
int naive_predict_mean(sample * indiv,naive_model * model);

/**
* @brief   free the memspace used for a specimen. 
* @param   indiv   Sample structure's pointer, specimen to destroy. 
**/
void destroy_indiv(sample * indiv);

/**
* @brief   free the memspace used for a model. 
* @param   model   naive_model structure's pointer, model to destroy.
**/
void destroy_model(naive_model * model);
#endif // _NAIVE_H_
