/**
 * @file    user_app.h
 * @author  gayraudbenoit@gmail.com (Smartfox), modified by ShaggyDogs
 * @brief   Contains all the functions used to send commands to the drone
 * @version 2.0
 * @date    January 2014 (last modified: Nov 2014)
 **/
#ifndef USER_APP_H__
#define USER_APP_H__
#include <linux/input.h>
#include <VP_Api/vp_api_thread_helper.h> // api pour les threads

 extern int modeReaction;

PROTO_THREAD_ROUTINE(th_user_app, data);

/**
 * @fn      hovering
 * @brief   make the drone hovers over the floor
 **/
void hovering();

/**
 * @fn      apply_command
 * @brief   sends a specific command to the drone while it is flying
 * @param   roll    the roll angle to set to the drone
 * @param   pitch   the pitch angle to set to the drone
 * @param   Vyaw    the yaw speed to set to the drone
 * @param   gas     the gas to set to th drone motors
 **/
void apply_command(float roll, float pitch, float Vyaw, float gas);

/**
 * @fn      landing
 * @brief   makes the drone lands 
 **/
void landing();

/**
 * @fn      takeoff
 * @brief   makes the drone takes off 
 **/
void takeoff();

/**
 * @fn      init_userapp
 * @brief   initialize the user app by reading the configuration file 
 * @param   keyboard_file       the conf file indicating wich key is used for each possible command
 * @param   len                 size of the file
 **/
void init_userapp(char * keyboard_file, size_t len);

/**
 * @fn      extract_key_event
 * @brief   extract the key that triggered the event and send the matching command to the drone
 * @param   ev      event that indicate wich key has been pressed
 **/
void extract_key_event(struct input_event * ev);

/* ############################################################################################################## */

/* configuration file containing the keyboard file */
#define CONF_FILE "./Sources/UserApp/user_app.conf"

/* drone motions and keyboard keys association*/
#define GO_FORWARD 	KEY_W
#define GO_BACK 	KEY_S
#define GO_LEFT		KEY_A
#define GO_RIGHT	KEY_D
#define GO_UP		KEY_UP
#define GO_DOWN		KEY_DOWN
#define ROTATE_LEFT	KEY_LEFT
#define ROTATE_RIGHT	KEY_RIGHT 
#define TAKEOFF		KEY_T
#define LANDING		KEY_L
#define KILL		KEY_K
#define CLASS_WALL  KEY_N
#define MODE_REACTION KEY_R


#define LOW_VELOCITY	0.3


/* ############################################################################################################## */


#endif
