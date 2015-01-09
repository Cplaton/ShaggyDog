/*---------------------------------------------
 * Author: Kevin Delmas
 * Date: 10.01.2014
 * Version: 1.0
 * File: model.h
 *---------------------------------------------
 */
#ifndef MODEL_H
#define MODEL_H

/*-------------------------------------------
 *  INCLUDES
 *-------------------------------------------
 */
#include <string.h>
#include <ardrone_api.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

/*-----------------------------------------
 *   TYPEDEF
 *-----------------------------------------
 */

// structure de donnees du drone
typedef struct _Navdata_t {
	float32_t roll;
	float32_t pitch;
	float32_t Vyaw;
	float32_t Vx;
	float32_t Vy;
	float32_t Vz;
}Navdata_t;

// structure de commande du drone
typedef struct _Inputs_t {
	float32_t roll;
	float32_t pitch;
	float32_t Vyaw;
	float32_t gas;
}Inputs_t;

/*------------------------------------
 *  PROTOTYPES
 *------------------------------------
 */


/******************
   function which initialize the drone model with in values

   parameters:
    in initSaveValue : drone data initialisation values
    in altitude      : altitude initialisation value
    in yaw           : yaw initialisation value
 ******************
 */
void initModel(Navdata_t * initSaveValue,float32_t altitude, float32_t yaw );


/******************
   function which calculates the drone's mathematical model outputs

   parameters:
   in inputs      : commands send to the drone
   out outputs    : drone's mathematical model outputs
 ******************
 */
void model(Inputs_t * inputs, Navdata_t * outputs);


/******************
   function which updates the navigation data with the new data from the drone

   parameters:
   out selectedNavdata    : convertion of the data send by the drone in the international system of units
   in nd                  : new navigation data send by the drone
 ******************
 */
void updateNavdata(Navdata_t *selectedNavdata,const navdata_demo_t *nd);


#endif

