/**
 * @file    mission.h
 * @author  ShaggyDogs
 * @brief   Contains all the functions used to send commands to the drone
 * @version 2.0
 * @date    November 2014
 */

#ifndef MISSION_H
#define MISSION_H

#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <VP_Api/vp_api_thread_helper.h> // api pour les threads
#include "Navdata/navdata_analyse.h"
#include "Model/model.h"
#include "Model/residue.h"
#include "UI/configurePage.h"
#include "smartfox_api.h"
#include "user_app.h"
#include "ardrone_move_cmd.h"

#define TAKEOFF_DRONE 1
#define FORWARD_PITCH 2
#define LEFT_ROLL 3
#define BACKWARD_PITCH 4
#define RIGHT_ROLL 5
#define GAS_UP 6
#define LEFT_YAW 7
#define GAS_DOWN 8
#define RIGHT_YAW 9
#define LAND_DRONE 10
#define HOVER_DRONE 11
#define END_REACTION 12 

extern char * mission_select_list[6];
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

/**
 * @fn      pitch
 * @brief   Applies the given pitch angle to the drone during the given time.
 * @param   value       the pitch angle to set (in ?? )
 * @param   us          the duration in us to wait for the answer.
 **/
float pitch(float value, int us);

/**
 * @fn      yaw
 * @brief   Applies the given yaw velocity to the drone during the given time.
 * @param   value       the yaw velocity to set (in ?? )
 * @param   us          the duration in us to wait for the answer.
 **/
float yaw(float value, int us);

/**
 * @fn      gas
 * @brief   Applies the given gas velocity to the drone during the given time.
 * @param   value       the gas velocity to set (in ?? )
 * @param   us          the duration in us to wait for the answer.
 **/
float gas(float value, int us);

/**
 * @fn      hover
 * @brief   Applies the hovering command to the drone during the given time.
 * @param   us          the duration in us to wait for the answer.
 **/
float hover(int us);




PROTO_THREAD_ROUTINE(mission, data);





#endif
