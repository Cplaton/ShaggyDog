/**
 * @file    configurePage.h
 * @author  shaggydogs & smartfox
 * @date    10/01/15
 * @brief   Manage the configuratio page creation. The configuration page is the page in wich the user selects for exemple if he want to activate the reaction module. It's the page that is displayed when the users open the application.
 **/
#ifndef CONFIGURE_PAGE_H_
#define CONFIGURE_PAGE_H_

#include "gui.h"
#include "displayMsg.h"
#include "debugPage.h"
#include "Navdata/navdata_analyse.h"
#include "UserApp/mission.h"
#include "UserApp/reaction.h"
#include "controller.h"




/*------------------------------------------------------------Public structure*/

typedef struct
{
    gint index;
    gchar *  p_text;
}
combo_data_st;


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
 * @brief       Constructor of the configure page
 **/
void configPage();


/**
 * @brief   Hides the mission selection dialog
 **/
void hideMissionSelectionDialog();

/**
 * 	@brief   Gets the selected item information from the mission selector;
 **/
combo_data_st getSelectedMissionFromCombo();

#endif
