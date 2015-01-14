/**
 * @file    classifier.h
 * @author  shaggydogs
 * @date    10/01/15
 * @brief   Contain the structure used by each classifiers.
 **/

#ifndef _CLASSIFIER_H_
#define _CLASSIFIER_H_

/**
 * @struct predict_results 
 * @abstract   Structure that represents a classification's result.
 **/
struct s_predict_results{
	double confidence;         /**< Confidense we can have in the result (in %). */
	int predict_class;        /**< Predicted class. */
	int class_count;          /**< Number of class recognized by the classifier. */
}typedef predict_results;


#endif

