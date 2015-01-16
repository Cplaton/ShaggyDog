/**
 * @file    controller.h
 * @author  shaggydogs
 * @date    10/01/15
 * @brief   Manage the events generated by the gui.
 **/


#include "gui.h"
#include "configurePage.h"

#ifndef __CONTROLLER_H
#define __CONTROLLER_H
/*------------------------------------------------------------Public variables*/
/**
 * @var		missionModeOn
 * @brief	inform whether the mission mode is activated or not
 */
extern int missionModeOn;

/**
 * @var     select_mission
 * @brief	The identifier of the selected mission
 **/
extern char * select_mission;

/*------------------------------------------------------------Public functions*/
/**
 * @brief   Manage the window destroy event.
 * @param   pWidget The object that generated the function call
 * @param   pData   The data generated by the event.
 **/
void OnDestroy(GtkWidget *pWidget, gpointer pData);

/**
 * @brief   Starts a new flight.
 **/
void start_flight();

/**
 * @fn      check_button_callback
 * @param   widget
 * @param   data        UI element that generated the call of this function
 * @brief   manage the click on the finish button of the main page
 */
void check_button_callback(GtkWidget *widget, gpointer data);

/**
 * @fn      check_selected_mission_id
 * @param   widget      The widget that
 * @param   data        UI element that generated the call of this function
 * @brief   manage the mission selection event
 */
void check_selected_mission_id(GtkWidget *widget, gpointer data);




#endif