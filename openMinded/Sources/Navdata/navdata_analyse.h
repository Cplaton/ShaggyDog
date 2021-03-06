/**
 * @file    navdata_analyse.h
 * @author  ShaggyDogs and Smartfox
 * @brief   Navdata management based on Smartfox's file
 * @version 1.0
 * @date    November 2014
 **/

#ifndef _NAVDATA_ANALYSE_H_
#define _NAVDATA_ANALYSE_H_

#include <ardrone_tool/Navdata/ardrone_navdata_client.h>

#define NAME_MODEL_DATA "dataModel"
#define NAME_REAL_DATA "dataReal"
#define NAME_FILTERED_DATA "dataFiltered"
#define NAME_SELECTED_NAVDATA "selectedNav"
#define NAME_RESIDUE "residue"
#define NAME_LOG "logSFM"

#define NAME_TRAINING_SET "Sources/Navdata/classification/BaseApp"
#define NAME_TRAINING_MODEL "Sources/Navdata/classification/BaseApp.model"

#define KNN_DATA_SET "Sources/Navdata/classification/KNN_BaseApp"

/**
 * @enum        drone_state
 * @abstract	enum representing the possibles state of the drone.
 **/
typedef enum drone_state {
	FLYING,     /**< State that take the drone when he is flying normaly. */
	LANDING,    /**< State that take the drone when he is landing. */
	LANDED,     /**< State that take the drone when he is landed. */
	TAKING_OFF, /**< State that take the drone when he is taking off. */
	UNKNOWN_STATE /**< State that take the drone when he is not in anyone of the other states. */
} drone_state_t;


/* Returns the last drone state */
/**
 * @brief   Gets the last drone state into drone_state_t format
 * @return  the last drone state
 **/
drone_state_t get_drone_state();

/**
 * @brief   Gets the current battery level in percentage
 * @return  the current battery level in percentage
 **/
float get_battery_level();

/**
 * @brief   Gets the current wifi quality, the value is between ... and ...
 * @return  the wifi link quality
 **/
float get_wifi_quality();

/**
 * @brief   Shared variable that contain the identifier of the class that is representing the current state.
 *          This value is setted by an expert function (in mission file) and is then written in the database.
 **/
extern int class_id;
//extern int file_number;
//extern char * shared_file_name;
//

/**
 * @brief	Display the new detected class to the user (gui and console display)
 * @param	classId		Identifier of the detected class **/
void alertDroneState (int classId);

/**
 * @brief	Display the new detected class to the user (gui and console display)
 * @param	classCount	Number of class detected by the algorithm
 * @param	classId		Identifier of the detected class
 * @param	confidence	Confidence rate (in percentage) of the detected class, -1 if none
 * @param	algoName		Name of the algorithm used for the classification
 **/
void alertFullDroneState (int classCount, int classId, double confidence, char * algoName);
#endif // _NAVDATA_H
