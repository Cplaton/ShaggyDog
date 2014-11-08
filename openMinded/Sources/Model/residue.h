/*---------------------------------------------
 *Author: Kevin Delmas
 *Date: 10.01.2014
 *Version: 1.0
 *File: residue.h
 *---------------------------------------------
 */

#ifndef RESIDUE_H
#define RESIDUE_H

/*-------------------------------------------
 *  INCLUDES
 *-------------------------------------------
 */
#include  <math.h>
#include  <string.h>
#include <ardrone_api.h>
#include <VP_Os/vp_os_signal.h>

#include "model.h"
#include "utils.h"
#include "ardrone_move_cmd.h"

/*-----------------------------------------
 *   TYPEDEF
 *-----------------------------------------
 */
typedef enum _alert_t {ALERT_X, ALERT_Y, ALERT_Z, NO_ALERT, ALERT_ROT} alert_t;

typedef enum _alarm_t {ALARM_N, ALARM_Z, ALARM_P} alarm_t;

typedef enum _fault_t {OBSTACLE_DEVANT,OBSTACLE_ARRIERE,OBSTACLE_DROITE,
OBSTACLE_GAUCHE,OBSTACLE_HAUT, OBSTACLE_BAS, VENT_AVANT,
VENT_ARRIERE,OBSTACLE_DROITE_AVANT,OBSTACLE_DROITE_ARRIERE ,
OBSTACLE_GAUCHE_ARRIERE,OBSTACLE_GAUCHE_DEVANT,OBSTACLE_DEVANT_GAUCHE,
OBSTACLE_DEVANT_DROITE,OBSTACLE_ARRIERE_DROITE,OBSTACLE_ARRIERE_GAUCHE,
OBSTACLE_AVANT_DESSOUS,OBSTACLE_ARRIERE_DESSOUS,OBSTACLE_ARRIERE_DESSUS,
OBSTACLE_AVANT_DESSUS,OBSTACLE_DESSOUS_DROITE,OBSTACLE_DESSOUS_GAUCHE,
OBSTACLE_DESSUS_GAUCHE,OBSTACLE_DESSUS_DROIT,
NO_FAULT,UNKNOWN_FAULT} fault_t;

typedef enum _emergency_state 	{NO_EMERGENCY,GO_BACK,GO_FORWARD,GO_LEFT,GO_RIGHT,GO_UP,GO_DOWN,E_STABILISATION,LAND,E_VERIFICATION,VERIFICATION_FAILED} emergency_state;

typedef struct _Residue_t {
	float32_t r_roll;
	float32_t r_pitch;
	float32_t r_Vyaw;
	float32_t r_Vx;
	float32_t r_Vy;
	float32_t r_Vz;
}Residue_t ;

typedef struct _options_t {
	unsigned char saturation;
	unsigned char sysUrgenceExtreme;
	unsigned char sePoser;
	unsigned char SMLimited;
	unsigned char debug;
	unsigned char disableSSM;
}options_t;


/*------------------------------------
 *  PROTOTYPES
 *------------------------------------
 */


/******************
 function which initialize the drone filters in values
 
 parameters:
 in value   : drone data initialisation values
 ******************
 */
void initFilters(Navdata_t * value);


/******************
 function which filter the drone values
 
 parameters:
 in raw         : new drone data values
 out filtered   : filtered drone data values
 ******************
 */
void filters(Navdata_t * raw, Navdata_t * filtered);


/******************
 function which calculates the diference between the mathematical model outputs and the real values given by the drone
 
 parameters:
 in model       : mathematical model outputs
 in real        : new drone data values
 out residue    : difference between model and real
 ******************
 */
void calcResidue(Navdata_t * model, Navdata_t * real, Residue_t * residue );


/******************
 function which writes in the global variable alarm if the residue is too large, normal or too small
 
 parameters:
 return     : Obsolete
 in residue : difference between the mathematical model outputs and the new drone data values
 ******************
 */
alert_t residueAnalysis(Residue_t * residue);

/******************
 //function (state machine) which is going to make the drone react in case of an obstacle
 
 parameters:
 in fault       : The fault (or not fault) in which the drone is
 in droneState  : new drone data values
 return         : returns the stete in which the machine is
 ******************
 */
unsigned char smartSafetyMode(fault_t fault, Navdata_t * droneState);

/******************
 //function which starts signatureAnalysis with the detected fault
 
 parameters:
 return     : returns the fault (or not fault) in which the drone is
 ******************
 */

fault_t diagnosis (unsigned char reset) ;

/******************
//function which restricts the commands send to the drone to ensure its integrity
 parameters:
 out : commands send to the drone
 ******************
 */
void satCmd(Inputs_t* input);

/******************
 //function which lets the other threads write on the variable opt
 out opt : variable that is going to be writen by the other threads
 ******************
 */
void writeOpt(options_t * opt);

/******************
//function which lets the other threads read on the variable opt
 out opt : variable that is going to be read by the other threads
 ******************
 */
void readOpt(options_t * opt);

/******************
 //function which lets the other threads write on the variable signature
 out opt : variable that is going to be read by the other threads
 ******************
 */
void readSignature(alarm_t * signature);

/******************
 //function which lets the other threads read the variable data
 out opt : variable that is going to be read by the other threads
 ******************
 */
void readNavdata(Navdata_t* data);
void readFault(fault_t *fault);
void readEmergency(emergency_state *emergency);

#define getSignature 	readSignature
#define getFault 	readFault
#define getResidue 	calcResidue

#endif
