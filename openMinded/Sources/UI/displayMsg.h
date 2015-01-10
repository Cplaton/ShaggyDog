/**
 * @file    displayMsg.h
 * @author  shaggydogs
 * @date    10/01/15
 * @brief   Manage the display of informations into the GUI.
 **/

#ifndef DISPLAY_MSG_H_
#define DISPLAY_MSG_H_

#include "gui.h"
#include "Model/residue.h"
#include <string.h>

/**
 * @brief       Displays an alert message corresponding to the given fault into the screen
 * @param       msg     The fault to display into the screen
 **/
void displayAlertMsg (fault_t msg);

/**
 * @brief       Displays an emergency message corresponding to the state given in param into the screen. An emergency message represent the reactio choosen by the reaction module when a dengerous situation is detected.
 * @param       em      Emergency state code representing the reaction chosen by the reaction module.
 **/
void displayEmergencyMsg (emergency_state em);

/**
 * @brief       Displays the recognized class into the class field into the GUI. 
 * @param       classid     Identifier of the recognized class
 **/
void showRecognizedClass (int classid);

/**
 * @brief       Displays the drone state into the gui. NB: the drone state is a value between: flyin, landing, hovering ... It's different of it's class.
 * @param       state   Identifier of the drone state
 **/
void showDroneState(int state);

/**
 * @brief       Displays the battery level into the screen
 * @param       level   Battery level of the drone to display
 **/
void showDroneBatteryLevel(float level);

/**
 * @brief       Displays the wifi link quality into the screen
 * @param       level   Wifi quality level to display
 **/
void showDroneWifiQualityLevel(float level);


#endif
