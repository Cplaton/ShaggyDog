#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "libpq-fe.h"                                   /* Postgress library header */
#ifndef OPENMINDED_BD_LIB
#define OPENMINDE_BD_LIB 
#define DEBUG_MODE 0

/**
 * @struct	augmented_navdata
 * @abstract	Struct representing the data of one sensor mesure as it's stored in the database.
 * @field	time
 * @field	alt
 * @field	pitch
 * @field	roll
 * @field	vyaw
 * @field	vx
 * @field	vy
 * @field 	vz
 * @field
 * @field
 * @field
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
};



/**
 * @brief       Disconnect properly the program from the database
 */
static void exit_nicely();


/**
 * @brief 	Opens a connection with the database
 * @return 	0 if connection successes, 1 in case of error. Error messages are printed in standard error.
 **/
int connect_to_database();



/**
 * @brief	get values from the database
 * @param 	number	 	Limit for the number of values that should be getted, 0 if no limit
 * @param 	flight_id	The id of the flight that should be getted, -1 if get all 
 * @param 	result 		Would be filled with the number of line founded corresponding to the request
 * @return	An array filled with the data matching the request, null in case of error 
 **/
struct augmented_navdata * get_values_from_db(int number, int flight_id, int * nb_res);


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
 * @return : 0 if connection successes, 1 in case of error. Error messages are printed in standard error.
 **/
int insert_new_data( int time, float alt, float pitch, float roll, float vyaw, float vx, float vy, float vz, float ax, float ay, float az);


/** 
 * @brief 	Closes a connection with the database
 * @return 	0 if connection successes, 1 in case of error. Error messages are printed in standard error.
 **/
int disconnect_to_database();

#endif

