#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "libpq-fe.h"                                   /* Postgress library header */

#define DEBUG_MODE 0


struct augmented_navdata {
	int time;
	float alt; 
	float pitch; 
	float roll;
	float vyaw; 
	float vx;
	float vy; 
	float vz; 
	float ax; 
	float ay; 
	float az;	
};



/** 
@brief : Closes nicely the DB connection and exits
@param psql: Contains the database informations
 **/
static void exit_nicely();


/** 
@brief : Opens a connection with the database
@return : 0 if connection successes, 1 in case of error. Error messages are printed in standard error.
 **/
int connect_to_database();



/**
@brief: get values from the database
@param number: a limit for the number of values that should be getted, 0 if no limit
@param flight_id: the id of the flight that should be getted, -1 if get all 
@param result: would be filled with the number of line founded corresponding to the request
@return: an array filled with the data matching the request, null in case of error 
*/
struct augmented_navdata * get_values_from_db(int number, int flight_id, int * nb_res);


/** 
@brief : Create a new flight in the database
@return : 0 if connection successes, 1 in case of error. Error messages are printed in standard error.
 **/
int start_new_flight();


/** 
@brief : Insert a new data into the database
@param time : time in second spend since the begining of the flight
@param alt : altitude of the dronne
@param pitch : pitch of the drone
@param roll : roll of the drone
@param vyaw : yaw speed of the drone
@param vx : x speed of the drone
@param vy : y speed of the drone
@param vz : z speed of the drone
@param ax : x acceleration of the drone
@param ay : y acceleration of the drone
@param az : z acceleration of the drone
@return : 0 if connection successes, 1 in case of error. Error messages are printed in standard error.
 **/
int insert_new_data( int time, float alt, float pitch, float roll, float vyaw, float vx, float vy, float vz, float ax, float ay, float az);


/** 
@brief : Closes a connection with the database
@return : 0 if connection successes, 1 in case of error. Error messages are printed in standard error.
 **/
int disconnect_to_database();
