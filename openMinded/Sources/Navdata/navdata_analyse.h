#ifndef _NAVDATA_ANALYSE_H_
#define _NAVDATA_ANALYSE_H_

#include <ardrone_tool/Navdata/ardrone_navdata_client.h>

#define NAME_MODEL_DATA "dataModel"
#define NAME_REAL_DATA "dataReal" 
#define NAME_FILTERED_DATA "dataFiltered"
#define NAME_SELECTED_NAVDATA "selectedNav"
#define NAME_RESIDUE "residue"
#define NAME_LOG "logSFM"

typedef enum drone_state {
  FLYING, LANDING, LANDED, TAKING_OFF, UNKNOWN_STATE
} drone_state_t;

/* Returns the last drone state */
drone_state_t get_drone_state();

/* returns the battery level in percentage */
float get_battery_level(); 

/* returns the wifi link quality */
float get_wifi_quality();

extern int class_id;
#endif // _NAVDATA_H_
