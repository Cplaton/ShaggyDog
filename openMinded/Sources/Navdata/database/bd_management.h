/**
 * @file    bd_management.h
 * @author  Arnaud LECHANTRE  - ShaggyDogs
 * @brief   Librairy that manage a specific database that save each sensor values available on AR - Drone
 * @version 1.0
 * @date    1 november 2014
 **/
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>	
//#include "utils.h"
#include "libpq-fe.h"                                   /* Postgress library header */
#ifndef OPENMINDED_BD_LIB
#define OPENMINDE_BD_LIB 
#define DEBUG_MODE 0

#define DATA_ALT 0
#define DATA_PITCH 1
#define DATA_ROLL 2
#define DATA_VYAW 3
#define DATA_VX 4
#define DATA_VY 5
#define DATA_VZ 6
#define DATA_AX 7
#define DATA_AY 8
#define DATA_AZ 9

/**
 * @struct	augmented_navdata
 * @abstract	Struct representing the data of one sensor mesure as it's stored in the database.
 **/
struct augmented_navdata {
	int time;       /**< Time spent since the begining of the flight. */
	float alt;      /**< Altitude of the drone in m. */
	float pitch;    /**< pitch of the drone. */
	float roll;     /**< roll of the drone. */
	float vyaw;     /**< yaw speed of the drone. */
	float vx;       /**< x speed of the drone m/s. */
	float vy;       /**< y speed of the drone m/s. */
	float vz;       /**< z speed of the drone m/s. */
	float ax;       /**< x acceleration of the drone (m.s^-2). */
	float ay;       /**< y acceleration of the drone (m.s^-2). */
	float az;       /**< z acceleration of the drone (m.s^-2). */
	int class_id;	/**< identifier of the class representing the data. 0 represent the no classified items. */
};



/**
 * @brief       Disconnect properly the program from the database
 */
void exit_nicely();


/**
 * @brief 	Opens a connection with the database
 * @return 	0 if connection successes, 1 in case of error. Error messages are printed in standard error.
 **/
int connect_to_database();


/**
 * @brief	Get the maximum value from the selected column from the selected table 
 * @param	columName	String representing the name of the column in witch the maximun should be searched
 * @param	tableName	String representign the name of the table in witch the value should be searched
 * @return	The maximum value founded in the selected column from the selected table (into float format).
 **/
float get_max(char * columnName, char * tableName);

/**
 * @brief	Get the minimum value from the selected column from the selected table 
 * @param	columName	String representing the name of the column in witch the minimun should be searched
 * @param	tableName	String representign the name of the table in witch the value should be searched
 * @return	The minimum value founded in the selected column from the selected table (into float format).
 **/
float get_min(char * columnName, char * tableName);

/**
 * @brief	Norm a value (between -1 and 1) considering the given limits
 * @param	init	Initial value to be normed
 * @param	min		Current minimum value
 * @param	max		Current maximum value 
 * @return 	The normed value
 **/
 float norm_value(float init, float min, float max);

double norm_indiv(double init, int type);
 
/**
 * @brief	get values from the database
 * @param 	number	 	Limit for the number of values that should be getted, 0 if no limit
 * @param 	flight_id	The id of the flight that should be getted, -1 if get all 
 * @param 	nb_res 		Would be filled with the number of line founded corresponding to the request
 * @return	An array filled with the data matching the request, null in case of error 
 **/
struct augmented_navdata * get_values_from_db(int number, int flight_id, int * nb_res);

/**
 * @brief	get values from the database and return them into a normed format
 * @param 	number	 	Limit for the number of values that should be getted, 0 if no limit
 * @param 	flight_id	The id of the flight that should be getted, -1 if get all 
 * @param 	nb_res 		Would be filled with the number of line founded corresponding to the request
 * @return	An array filled with the normed data matching the request, null in case of error 
 **/
struct augmented_navdata * get_normed_values_from_db(int number, int flight_id, int * nb_res);

/**
 * @brief	get values from the database and write them into a CSV file
 * @param 	number	 	Limit for the number of values that should be getted, 0 if no limit
 * @param 	flight_id	The id of the flight that should be getted, -1 if get all 
 * @param 	should_norm	Indicate whether the values should be normed or not before being writed in the CSV file.
 * @return	0 in case of success, 1 in case of error 
 **/
 int write_data_to_csv(char * csvFileName, int number, int flight_id, int should_norm);

/**
 * @brief 	Create a new flight in the database
 * @return 	0 if connection successes, 1 in case of error. Error messages are printed in standard error.
 **/
int start_new_flight();


/**
 * @brief 	Insert a new data into the database
 * @param 	time 		Time in second spend since the begining of the flight
 * @param 	alt 		Altitude of the dronne
 * @param 	pitch 		Pitch of the drone
 * @param 	roll 		Roll of the drone
 * @param 	vyaw 		Yaw speed of the drone
 * @param 	vx 		x speed of the drone
 * @param 	vy 		y speed of the drone
 * @param 	vz 		z speed of the drone
 * @param 	ax 		x acceleration of the drone
 * @param 	ay 		y acceleration of the drone
 * @param 	az 		z acceleration of the drone
 * @param 	class_id	identifier of the class in wich the item has been classified. 0 if it's unclassified.
 * @return : 0 if connection successes, 1 in case of error. Error messages are printed in standard error.
 **/
int insert_new_data( int time, float alt, float pitch, float roll, float vyaw, float vx, float vy, float vz, float ax, float ay, float az, int class_id);


/** 
 * @brief 	Closes a connection with the database
 * @return 	0 if connection successes, 1 in case of error. Error messages are printed in standard error.
 **/
int disconnect_to_database();

/**
 * @brief       Returns the name of th class with the given classid
 * @param       classId         (in)  identifier of the class
 * @param       className       (out) the char array representing the class name 
 **/
//void getClassName (int classId, char* className);
#endif

