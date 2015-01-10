#ifndef DISPLAY_MSG_H_
#define DISPLAY_MSG_H_

#include "gui.h"
#include "Model/residue.h"
#include <string.h>

void displayAlertMsg (fault_t msg);
void displayEmergencyMsg (emergency_state em);
void showRecognizedClass (int classid);

void showDroneState(int state);
void showDroneBatteryLevel(float level);
void showDroneWifiQualityLevel(float level);


#endif
