/**
 * @file    mission.c
 * @author  ShaggyDogs
 * @brief   Contains all the functions used to send commands to the drone
 * @version 2.0
 * @date    November 2014
 */

#ifndef MISSION_H
#define MISSION_H

#include <linux/input.h>
#include <VP_Api/vp_api_thread_helper.h> // api pour les threads
#include "Navdata/navdata_analyse.h"
#include "Model/model.h"
#include "Model/residue.h"
#include "UI/configurePage.h"


/**
 * @fn      mission_SFS_1
 * @brief   Sends commands to the drone in order to do a mission. This mission is the first one used to learn the SFS situation
 **/
void mission_SFS_1();

/**
 * @fn      mission_SFS_2
 * @brief   Sends commands to the drone in order to do a mission. This mission is the one used to test the SFS situation recognition.
 **/
void mission_SFS_2();

/**
 * @fn      mission_WALL_1
 * @brief   Sends commands to the drone in order to do a mission. This mission is the first one used to learn the front wall impact situation
 **/
void mission_WALL_1();

/**
 * @fn      mission_WALL_2
 * @brief   Sends commands to the drone in order to do a mission. This mission is the first one used to learn the left wall impact situation
 **/
void mission_WALL_2();

/**
 * @fn      mission_WALL_3
 * @brief   Sends commands to the drone in order to do a mission. This mission is the first one used to learn the right wall impact situation
 **/
void mission_WALL_3();

/**
 * @fn      mission_WALL_4
 * @brief   Sends commands to the drone in order to do a mission. This mission is the first one used to learn the back wall impact situation
 **/
void mission_WALL_4();

/**
 * @fn      roll
 * @brief   Applies the given roll angle to the drone during the given time.
 * @param   value       the roll angle to set (in ?? )
 * @param   us          the duration in us to wait for the answer.
 **/
float roll(float value, int us);

float pitch(float value, int us);

float yaw(float value, int us);

float gas(float value, int us);

float hover(int us);




PROTO_THREAD_ROUTINE(mission, data);





#endif
