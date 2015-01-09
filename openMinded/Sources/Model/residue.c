/*---------------------------------------------
 * Author: Kevin Delmas
 * Date: 10.01.2014
 * Version: 1.0
 * File: residue.c
 *---------------------------------------------
 */

#include "residue.h"

/*-------------------------------------------
 *  DEFINES
 *-------------------------------------------
 */

//filter function for the pitch => (a1p + a2p*Z + a3p*Z^2)/(b1p + b2p*Z + b3p*Z^2)
#define a1p 0.000206
#define a2p 0.000412
#define a3p 0.000206
#define b1p 0.9598
#define b2p -1.959
#define b3p 1

//filter function for the roll => (a1r + a2r*Z + a3r*Z^2)/(b1r + b2r*Z + b3r*Z^2)
#define a1r 0.0001381
#define a2r 0.0002762
#define a3r 0.0001381
#define b1r 0.967
#define b2r -1.9665
#define b3r 1

//filter function for the yaw => (a1y + a2y*Z + a3y*Z^2)/(b1y + b2y*Z + b3y*Z^2)
#define a1y 0.00021814
#define a2y 0.00043627
#define a3y 0.00021814
#define b1y 0.9587
#define b2y -1.9578
#define b3y 1

//filter function for Vx => (a1Vx + a2Vx*Z + a3Vx*Z^2)/(b1Vx + b2Vx*Z + b3Vx*Z^2)
#define a1Vx 0.00002057
#define a2Vx 0.00004114
#define a3Vx 0.00002057
#define b1Vx 0.98721311
#define b2Vx -1.9871308
#define b3Vx 1

//filter function for Vy => (a1Vy + a2Vy*Z + a3Vy*Z^2)/(b1Vy + b2Vy*Z + b3Vy*Z^2)
#define a1Vy 0.000013618677
#define a2Vy 0.000027237354
#define a3Vy 0.000013618677
#define b1Vy 0.9895894
#define b2Vy -1.9895349
#define b3Vy 1

//filter function for Vz => (a1Vz + a2Vz*Z + a3Vz*Z^2)/(b1Vz + b2Vz*Z + b3Vz*Z^2)
#define a1Vz 0.000070558
#define a2Vz 0.00014112
#define a3Vz 0.000070558
#define b1Vz 0.9764
#define b2Vz -1.9761
#define b3Vz 1

#define roll_dcgain 30 //roll transfer function gain

#define pitch_dcgain 18.2
#define Vyaw_dcgain 91.7
#define Vx_dcgain 7.5
#define Vy_dcgain 7.5
#define Vz_dcgain 0.8067
#define roll_to 0.625           // roll rise time
#define pitch_to 0.315          // pitch rise time
#define Vyaw_to 2       // Vyaw rise time
#define Vx_to 2.89              // Vx rise time
#define Vy_to 4.195             // Vy rise time
#define Vz_to 1.5               // Vz rise time
#define INIT_PERCENT_P 0.5      // pitch residue difference tolerance percentage (INIT_PERCENT_P * pitch_dcgain = pitch tolerance value)
#define END_PERCENT_P 0.3       // steady state residue difference tolerence percentage (END_PERCENT_P * pitch_dcgain = steady state pitch tolerance value)
#define INIT_PERCENT_R 0.5 //roll residue difference tolerance percentage (INIT_PERCENT_R * roll_dcgain = roll tolerance value)
#define END_PERCENT_R 0.15 //steady state residue difference tolerance percentage (END_PERCENT_R * roll_dcgain = steady state roll tolerance value)
#define INIT_PERCENT_Y 1 //VYaw residue difference tolerance percentage (INIT_PERCENT_Y * Vyaw_dcgain = VYaw tolerance value)
#define END_PERCENT_Y 0.3 //steady state residue difference tolerance percentage (END_PERCENT_Y * Vyaw_dcgain = steady state VYaw tolerance value)
#define INIT_PERCENT_VX 0.5 //Vx residue difference tolerance percentage (INIT_PERCENT_VX * Vx_dcgain = Vx tolerance value)
#define END_PERCENT_VX 0.3 //steady state residue difference tolerance percentage (END_PERCENT_VX * Vx_dcgain = steady state Vx tolerance value)
#define INIT_PERCENT_VY 0.5 //Vy residue difference tolerance percentage (INIT_PERCENT_VY * Vy_dcgain = Vy tolerance value)
#define END_PERCENT_VY 0.3 //steady state residue difference tolerance percentage (END_PERCENT_VY * Vy_dcgain = steady state Vy tolerance value)
#define INIT_PERCENT_VZ 1 //Vz residue difference tolerance percentage (INIT_PERCENT_VZ * Vz_dcgain = Vz tolerance value)
#define END_PERCENT_VZ 0.2 //steady state residue difference tolerance percentage (END_PERCENT_P * Vz_dcgain = steady state Vz tolerance value)

#define SIGN_TAB_NB_FAULT 24    //amount of diagnosticable faults
#define SIGN_TAB_NB_RESIDU 6    //amount of outputs recived from the drone

/*-------------------------------------------
 *  GLOBAL VARIABLES
 *-------------------------------------------
 */

//extern Inputs_t saveInputs;
extern Inputs_t sfm_cmd;
extern Inputs_t local_cmd;
extern FILE* logSFM;
extern float32_t saveAltitude;
options_t options={0,0,0,0,0,0,0};

Navdata_t saveRaw1;
Navdata_t saveRaw2;
Navdata_t saveFiltered1;
Navdata_t saveFiltered2;
alarm_t alarm[SIGN_TAB_NB_RESIDU]; // current signature
fault_t currentFault=NO_FAULT;
static emergency_state emergency = NO_EMERGENCY;

vp_os_mutex_t sign_mutex,opt_mutex,data_mutex,fault_mutex,emergency_mutex;

alarm_t SIGN_MAT[SIGN_TAB_NB_FAULT][SIGN_TAB_NB_RESIDU]={
	{ALARM_Z,ALARM_N,ALARM_Z,ALARM_Z,ALARM_Z,ALARM_Z},
	{ALARM_Z,ALARM_P,ALARM_Z,ALARM_Z,ALARM_Z,ALARM_Z},
	{ALARM_Z,ALARM_Z,ALARM_N,ALARM_Z,ALARM_Z,ALARM_Z},
	{ALARM_Z,ALARM_Z,ALARM_P,ALARM_Z,ALARM_Z,ALARM_Z},
	{ALARM_Z,ALARM_Z,ALARM_Z,ALARM_Z,ALARM_Z,ALARM_N},
	{ALARM_Z,ALARM_Z,ALARM_Z,ALARM_Z,ALARM_Z,ALARM_P},
	{ALARM_Z,ALARM_Z,ALARM_Z,ALARM_N,ALARM_Z,ALARM_Z},
	{ALARM_Z,ALARM_Z,ALARM_Z,ALARM_P,ALARM_Z,ALARM_Z},
	{ALARM_N,ALARM_Z,ALARM_Z,ALARM_Z,ALARM_Z,ALARM_Z},
	{ALARM_P,ALARM_Z,ALARM_Z,ALARM_Z,ALARM_Z,ALARM_Z},
	{ALARM_Z,ALARM_N,ALARM_N,ALARM_Z,ALARM_Z,ALARM_Z},
	{ALARM_Z,ALARM_N,ALARM_P,ALARM_Z,ALARM_P,ALARM_Z},
	{ALARM_Z,ALARM_P,ALARM_N,ALARM_N,ALARM_Z,ALARM_Z},
	{ALARM_Z,ALARM_P,ALARM_P,ALARM_N,ALARM_Z,ALARM_Z},
	{ALARM_Z,ALARM_Z,ALARM_N,ALARM_P,ALARM_Z,ALARM_Z},
	{ALARM_Z,ALARM_Z,ALARM_P,ALARM_P,ALARM_Z,ALARM_Z},
	{ALARM_Z,ALARM_P,ALARM_Z,ALARM_Z,ALARM_Z,ALARM_N},
	{ALARM_Z,ALARM_N,ALARM_Z,ALARM_Z,ALARM_Z,ALARM_N},
	{ALARM_Z,ALARM_P,ALARM_Z,ALARM_Z,ALARM_Z,ALARM_P},
	{ALARM_Z,ALARM_N,ALARM_Z,ALARM_Z,ALARM_Z,ALARM_P},
	{ALARM_N,ALARM_Z,ALARM_Z,ALARM_Z,ALARM_Z,ALARM_N},
	{ALARM_P,ALARM_Z,ALARM_Z,ALARM_Z,ALARM_Z,ALARM_N},
	{ALARM_N,ALARM_Z,ALARM_Z,ALARM_Z,ALARM_Z,ALARM_P},
	{ALARM_P,ALARM_Z,ALARM_Z,ALARM_Z,ALARM_Z,ALARM_P}
};

//Array which shows the different faults that can be diagnostic-able
fault_t FAULT_TAB [SIGN_TAB_NB_FAULT]={
	OBSTACLE_DEVANT,
	OBSTACLE_ARRIERE,
	UNKNOWN_FAULT,
	UNKNOWN_FAULT,
	OBSTACLE_HAUT,
	OBSTACLE_BAS,
	VENT_AVANT,
	VENT_ARRIERE,
	OBSTACLE_GAUCHE,
	OBSTACLE_DROITE,
	OBSTACLE_DEVANT,
	OBSTACLE_DEVANT,
	OBSTACLE_ARRIERE,
	OBSTACLE_ARRIERE,
	OBSTACLE_ARRIERE,
	OBSTACLE_ARRIERE,
	OBSTACLE_DEVANT,
	OBSTACLE_ARRIERE,
	OBSTACLE_ARRIERE,
	OBSTACLE_DEVANT,
	OBSTACLE_DESSOUS_DROITE, //never tested
	OBSTACLE_DESSOUS_GAUCHE, //never tested
	OBSTACLE_DESSUS_GAUCHE, //never tested
	OBSTACLE_DESSUS_DROIT //never tested
};

/*-------------------------------------------
 *  FUNCTIONS
 *-------------------------------------------
 */

//function which initialize the drone filters in values
void initFilters(Navdata_t *value){
	Navdata_t init ={value->roll,value->pitch,0.0,0.0,0.0,0.0};

	memcpy(&saveRaw1,&init,sizeof(Navdata_t)); //initialize saveRaw1
	memcpy(&saveRaw2,&init,sizeof(Navdata_t)); //initialize saveRaw2
	memcpy(&saveFiltered1,&init,sizeof(Navdata_t)); //initialize saveFiltered1
	memcpy(&saveFiltered2,&init,sizeof(Navdata_t)); //initialize saveFiltered2

	vp_os_mutex_init(&sign_mutex);
	vp_os_mutex_init(&opt_mutex);
	vp_os_mutex_init(&data_mutex);
	vp_os_mutex_init(&fault_mutex);
	vp_os_mutex_init(&emergency_mutex);
}

//function which filter the drone values
void filters(Navdata_t * raw, Navdata_t * filtered){

	filtered->roll=1/(b3r)*(raw->roll*a3r+saveRaw1.roll*a2r +saveRaw2.roll*a1r-saveFiltered1.roll*b2r-saveFiltered2.roll*b1r); //filter the value of roll using its transfer function *

	filtered->pitch=1/(b3p)*(raw->pitch*a3p+saveRaw1.pitch*a2p      +saveRaw2.pitch*a1p-saveFiltered1.pitch*b2p-saveFiltered2.pitch*b1p); //filter the value of pitch using its transfer function *

	filtered->Vyaw=1/(b3y)*(raw->Vyaw*a3y+saveRaw1.Vyaw*a2y +saveRaw2.Vyaw*a1y-saveFiltered1.Vyaw*b2y-saveFiltered2.Vyaw*b1y); //filter the value of Vyaw using its transfer function *

	filtered->Vz=1/(b3Vz)*(raw->Vz*a3Vz+saveRaw1.Vz*a2Vz    +saveRaw2.Vz*a1Vz-saveFiltered1.Vz*b2Vz-saveFiltered2.Vz*b1Vz); //filter the value of Vz using its transfer function *

	filtered->Vy=1/(b3Vy)*(raw->Vy*a3Vy+saveRaw1.Vy*a2Vy    +saveRaw2.Vy*a1Vy-saveFiltered1.Vy*b2Vy-saveFiltered2.Vy*b1Vy); //filter the value of Vy using its transfer function *

	filtered->Vx=1/(b3Vx)*(raw->Vx*a3Vx+saveRaw1.Vx*a2Vx  +saveRaw2.Vx*a1Vx-saveFiltered1.Vx*b2Vx-saveFiltered2.Vx*b1Vx); //filter the value of Vx using its transfer function *

	// * see the transfer functions in DEFINES

	memcpy(&saveRaw2,&saveRaw1,sizeof(Navdata_t)); //save the second input last value in saveRaw2 (in the next loop saveRaw2 will have all the z^(-2) input data)

	memcpy(&saveRaw1,raw,sizeof(Navdata_t)); //save the input last value in saveRaw (in the next loop saveRaw will have all the z^(-1) input data)

	memcpy(&saveFiltered2,&saveFiltered1,sizeof(Navdata_t)); //save the second filtered output value in saveFiltered2 (in the next loop saveFiltered2 will have all the z^(-2) filter output data)

	memcpy(&saveFiltered1,filtered,sizeof(Navdata_t)); //save the filtered output last value in saveFiltered (in the next loop saveFiltered will have all the filter output z^(-1) data)




}

// function which calculates the difference between the mathematical model outputs and the real values given by the drone
void calcResidue(Navdata_t * model, Navdata_t * real, Residue_t * residue ){
	residue->r_roll=real->roll-model->roll;
	residue->r_pitch=real->pitch-model->pitch;
	residue->r_Vyaw=real->Vyaw-model->Vyaw;
	residue->r_Vx=real->Vx-model->Vx;
	residue->r_Vy=real->Vy-model->Vy;
	residue->r_Vz=real->Vz-model->Vz;
}

// function which writes in the global variable alarm if the residue is too large, normal or too small
alert_t residueAnalysis(Residue_t * residue){
	static float t=0;
	Navdata_t th_h,th_l;
	alert_t result=NO_ALERT;
	static Inputs_t savedInputs;

	if(savedInputs.roll!=local_cmd.roll || savedInputs.pitch!=local_cmd.pitch || savedInputs.Vyaw!=local_cmd.Vyaw || savedInputs.gas!=local_cmd.gas) {
		t=0;
	}
	memcpy(&savedInputs,&local_cmd,sizeof(Inputs_t));
	th_h.roll=INIT_PERCENT_R*roll_dcgain*((1-END_PERCENT_R/INIT_PERCENT_R)*expf(-t/roll_to)+END_PERCENT_R/INIT_PERCENT_R); //defines the upper limit of the roll residue, above this value the residue will be consider positiv
	th_l.roll=-th_h.roll; //defines the lower limit of the roll residue, under this value the residue will be consider negativ

	th_h.pitch=INIT_PERCENT_P*pitch_dcgain*((1-END_PERCENT_P/INIT_PERCENT_P)*expf(-t/pitch_to)+END_PERCENT_P/INIT_PERCENT_P); //defines the upper limit of the pitch residue, above this value the residue will be consider positive
	th_l.pitch=-th_h.pitch; //defines the lower limit of the pitch residue, under this value the residue will be consider negative

	th_h.Vyaw=INIT_PERCENT_Y*Vyaw_dcgain*((1-END_PERCENT_Y/INIT_PERCENT_Y)*expf(-t/Vyaw_to)+END_PERCENT_Y/INIT_PERCENT_Y); //defines the upper limit of the Vyaw residue, above this value the residue will be consider positive
	th_l.Vyaw=-th_h.Vyaw; //defines the lower limit of the Vyaw residue, under this value the residue will be consider negative

	th_h.Vx=INIT_PERCENT_VX*Vx_dcgain*((1-END_PERCENT_VX/INIT_PERCENT_VX)*expf(-t/Vx_to)+END_PERCENT_VX/INIT_PERCENT_VX); //defines the upper limit of the Vx residue, above this value the residue will be consider positive
	th_l.Vx=-th_h.Vx; //defines the lower limit of the Vx residue, under this value the residue will be consider negative

	th_h.Vy=INIT_PERCENT_VY*Vy_dcgain*((1-END_PERCENT_VY/INIT_PERCENT_VY)*expf(-t/Vy_to)+END_PERCENT_VY/INIT_PERCENT_VY); //defines the upper limit of the Vy residue, above this value the residue will be consider positive
	th_l.Vy=-th_h.Vy; //defines the lower limit of the Vy residue, under this value the residue will be consider negative

	th_h.Vz=INIT_PERCENT_VZ*Vz_dcgain*((1-(END_PERCENT_VZ*2)/INIT_PERCENT_VZ)*expf(-t/Vz_to)+END_PERCENT_VZ/INIT_PERCENT_VZ); //defines the upper limit of the Vz residue, above this value the residue will be consider positive
	th_l.Vz=-INIT_PERCENT_VZ*Vz_dcgain*((1-(END_PERCENT_VZ)/INIT_PERCENT_VZ)*expf(-t/Vz_to)+END_PERCENT_VZ/INIT_PERCENT_VZ); //defines the lower limit of the Vz residue, under this value the residue will be consider negative

	t+=0.005;

	if(residue->r_roll>th_h.roll) {
		alarm[0]= ALARM_P; //the residue is consider as positive
	}else if (residue->r_roll<th_l.roll) {
		alarm[0]=ALARM_N; //the residue is consider as negative
	}else{
		alarm[0]=ALARM_Z; //the residue is consider as zero
	}

	if(residue->r_pitch>th_h.pitch) {
		alarm[1]= ALARM_P; //the residue is consider as positive
		result=ALERT_Y;
	}else if (residue->r_pitch<th_l.pitch) {
		alarm[1]=ALARM_N; //the residue is consider as negative
	}else{
		alarm[1]=ALARM_Z; //the residue is consider as zero
	}

	if(residue->r_Vyaw>th_h.Vyaw) {
		alarm[2]= ALARM_P; //the residue is consider as positive
		result=ALERT_Y;
	}else if (residue->r_Vyaw<th_l.Vyaw) {
		alarm[2]=ALARM_N; //the residue is consider as negative
	}else{
		alarm[2]=ALARM_Z; //the residue is consider as zero
	}

	if(residue->r_Vx>th_h.Vx) {
		alarm[3]= ALARM_P; //the residue is consider as positive
		result=ALERT_Y;
	}else if (residue->r_Vx<th_l.Vx) {
		alarm[3]=ALARM_N; //the residue is consider as negative
	}else{
		alarm[3]=ALARM_Z; //the residue is consider as zero
	}

	if(residue->r_Vy>th_h.Vy) {
		alarm[4]= ALARM_P; //the residue is consider as positive
		result=ALERT_Y;
	}else if (residue->r_Vy<th_l.Vy) {
		alarm[4]=ALARM_N; //the residue is consider as negative
	}else{
		alarm[4]=ALARM_Z; //the residue is consider as zero
	}

	if(residue->r_Vz>th_h.Vz) {
		alarm[5]= ALARM_P; //the residue is consider as positive
		result=ALERT_Y;
	}else if (residue->r_Vz<th_l.Vz) {
		alarm[5]=ALARM_N; //the residue is consider as negative
	}else{
		alarm[5]=ALARM_Z; //the residue is consider as zero
	}

	if(options.debug!=0) {
		//fprintf(logSFM,"[residue Analysis] R:%d, P:%d, Y:%d, Vx:%d, Vy:%d, Vz:%d\n",alarm[0],alarm[1],alarm[2],alarm[3],alarm[4],alarm[5]);//writes the value of the residue (P,N,Z) in the log file
	}
	return result;
}

//function which decide if the fault found is possible
unsigned char isCompatible(fault_t toTest){

	if( (toTest==OBSTACLE_DEVANT) && (local_cmd.pitch >0.0)) {
		return 1; // if the fault is an obstacle behind the drone, the drone should be going backwards)
	} else if ((toTest==OBSTACLE_ARRIERE) && (local_cmd.pitch <0.0)) {
		return 1; // if the fault is an obstacle in front of the drone, the drone should be advancing)
	}
	else if( (toTest==OBSTACLE_DROITE) && (local_cmd.roll >0.0)) {
		return 1; // if the fault is an obstacle on the drones right side, the drone should be going to the right)
	} else if ((toTest==OBSTACLE_GAUCHE) && (local_cmd.roll <0.0)) {
		return 1; // if the fault is an obstacle on the drones left side, the drone should be going to the left)
	}
	else if( (toTest==OBSTACLE_HAUT) && (local_cmd.gas >0.0)) {
		return 1; // if the fault is an obstacle on the drones top, the drone should be going up)
	} else if ((toTest==OBSTACLE_BAS) && (local_cmd.gas <0.0)) {
		return 1; // if the fault is an obstacle on the drones top, the drone should be going up)
	}
	else if( (toTest==OBSTACLE_DROITE) && (local_cmd.Vyaw >0.0)) {
		return 1;
	} else if ((toTest==OBSTACLE_GAUCHE) && (local_cmd.Vyaw <0.0)) {
		return 1;
	}

	return 0;
}

#define INACTIVE 0
#define CONFIRM 1
#define WAITING_COMPATIBLE_SIG 2
#define FAULT_CONFIRMED 3
#define CONFIRM_TIME 59
#define WAITING_TIME 59

//function which describes a state machine that is going to decide if there is a fault or not after a validation

fault_t signatureAnalysis(fault_t incoming){
	static unsigned char state=INACTIVE;
	static unsigned char compteur=0;
	static fault_t analysedFault=NO_FAULT;

	switch(state) {
	case INACTIVE:         //state when there are no faults
		compteur=0;
		if(incoming!=NO_FAULT) {
			state=CONFIRM;         //if there is a fault go to the state confirm
			analysedFault=incoming;
		}
		break;

	case CONFIRM:         //to confirm the fault a validation period have to pass
		compteur++;
		if(incoming==NO_FAULT || incoming!=analysedFault) {
			state=INACTIVE;         //if there are no faults go to inactive
			break;
		}
		if(compteur>=CONFIRM_TIME) {
			compteur=0;
			state=WAITING_COMPATIBLE_SIG;
		}
		break;


	case WAITING_COMPATIBLE_SIG:        //if the fault is compatible (see isCompatible function) and the fault is verificated go to state confirmed
		compteur++;
		if(incoming==NO_FAULT || incoming!=analysedFault) {
			state=INACTIVE;        //if there are no faults go to inactive
			break;
		}
		if(isCompatible(incoming) || compteur>=WAITING_TIME) {
			state=FAULT_CONFIRMED;        //if the fault is compatible (see isCompatible function) and the fault is checked go to state confirmed
		}
		break;
	case FAULT_CONFIRMED:        //is going to send a confirmed fault
		compteur=0;
		if(incoming==NO_FAULT || incoming!=analysedFault) {
			state=INACTIVE;        //if there are no faults go to inactive
		}
		return incoming;         //returns the fault
		break;
	}
	return NO_FAULT;
}



//function which search the residues found in the signature matrix to find the fault
fault_t matrixAnalysis (void)
{
	unsigned char i,j, signFound;
	for(i=0; i<SIGN_TAB_NB_FAULT; i++) {
		signFound=1;
		for(j=0; j<SIGN_TAB_NB_RESIDU && signFound!=0; j++) {
			if(SIGN_MAT[i][j]!=alarm[j]) {
				signFound=0;
			}
		}
		if(signFound!=0) {
			return FAULT_TAB[i]; //return the fault found
		}
	}
	for(i=0; i<SIGN_TAB_NB_RESIDU; i++) {
		if(alarm[i]!=ALARM_Z) {
			return UNKNOWN_FAULT; //if there is an alarm but there is no signature for it the retourn is UNKNOWN_FAULT
		}
	}
	return NO_FAULT;
}



fault_t diagnosis (unsigned char reset){
	fault_t detected;
	detected=matrixAnalysis();
	if(reset!=0) {
		detected=NO_FAULT;
	}
	currentFault=signatureAnalysis(detected);
	if(options.debug != 0) {
		//fprintf(logSFM,"CurrentFault : %d\n",currentFault);
	}
	return currentFault;
}

#define NO_SAFETY_MODE 0
#define SAFETY_PROC 1
#define STABILISATION 2
#define VERIFICATION 3
#define TIME_SAFETY_PROC 600
#define TIME_SAFETY_PROC_LONG 2000
#define TIME_STABILISATION_PROC 400
#define TIME_VERIFICATION_PROC 200
#define END_OF_PROC 4

#define END_VY 0.1 //velocity that the drone is going to go right or left in case of a specific fault
#define END_VX 0.1 //velocity that the drone is going to advance or go backwards in case of a specific fault
#define END_ALTITUDE 1.0 //meters that the drone is going to descend or climb in case of a specific fault
#define GAIN_VZ 1.0 //Vz proportional close loop gain
#define GAIN_R 6.0 //Roll proportional close loop gain
#define GAIN_P 2.0 //Pitch proportional close loop gain
#define MAX_P_RESCUE 0.3
#define MIN_P_RESCUE 0
#define MAX_R_RESCUE 0.5
#define MIN_R_RESCUE 0

#define EXTREM_P 15.0
#define EXTREM_R 15.0

#define MAX_P_CMD 0.1 //maximum pitch that can be send to the drone if satCmd is on
#define MAX_R_CMD 0.1 //maximum roll that can be send to the drone if satCmd is on
#define MAX_VYAW_CMD 0.5 //maximum Y velocity that can be send to the drone if satCmd is on
#define MAX_GAS_CMD 0.6 //maximum gas that can be send to the drone if satCmd is on

//function which send a command to the drone to avoid the obstacle found
void safetyProc(fault_t fault, Navdata_t * droneState){
	float temp=0.0;
	if(options.sePoser==0) {
		switch(fault) {
		case NO_FAULT:
			break;
		case UNKNOWN_FAULT:
			emergency = LAND;
			set_sfm_command (NULL, LANDING_REQUEST);
			break;
		case OBSTACLE_HAUT:
			temp=(saveAltitude > 2*END_ALTITUDE) ? 2*END_ALTITUDE : saveAltitude;

			sfm_cmd.gas=(END_ALTITUDE-temp)*GAIN_VZ;        //The drone is going to descend until END_ALTITUDE to be sure that the drone do that a proportional close loop is done
			emergency = GO_DOWN;

			set_sfm_command (&sfm_cmd, FLYING_REQUEST);
			break;
		case OBSTACLE_BAS:
			temp=(saveAltitude > 2*END_ALTITUDE) ? 2*END_ALTITUDE : saveAltitude;

			sfm_cmd.gas=(END_ALTITUDE-temp)*GAIN_VZ;        //The drone is going to climb until END_ALTITUDE to be sure that the drone do that a proportional close loop is done
			emergency = GO_UP;
			set_sfm_command (&sfm_cmd, FLYING_REQUEST);
			break;
		case OBSTACLE_DROITE:
			emergency = LAND;
			set_sfm_command (NULL, LANDING_REQUEST);
			break;
		case OBSTACLE_GAUCHE:
			emergency = LAND;
			set_sfm_command (NULL, LANDING_REQUEST);
			break;
		case OBSTACLE_DEVANT:
			temp=(droneState->Vx+END_VX)*GAIN_P;
			temp= (temp>MAX_P_RESCUE) ? MAX_P_RESCUE : temp;
			temp= (temp<MIN_P_RESCUE) ? MIN_P_RESCUE : temp;
			sfm_cmd.pitch=temp;        //The drone is going to go backwards until a velocity of END_VX is achieved to be sure that the drone do that a proportional close loop is done
			emergency = GO_BACK;
			set_sfm_command (&sfm_cmd, FLYING_REQUEST);
			break;
		case OBSTACLE_ARRIERE:
			temp=-(END_VX-droneState->Vx)*GAIN_P;
			temp= (temp<-MAX_P_RESCUE) ? -MAX_P_RESCUE : temp;
			temp= (temp>-MIN_P_RESCUE) ? -MIN_P_RESCUE : temp;
			sfm_cmd.pitch=temp;        //The drone is going to advance until a velocity of END_VX is achieved to be sure that the drone do that a proportional close loop is done
			emergency = GO_FORWARD;
			break;
		default:
			break;
		}
	}else{
		emergency = LAND;
		set_sfm_command (NULL, LANDING_REQUEST);
	}
}
//function which restricts the commands send to the drone to ensure its integrity
void satCmd(Inputs_t *input){
	if(options.saturation!=0) {
		input->roll= (input->roll>MAX_R_CMD) ? MAX_R_CMD : input->roll;
		input->pitch=(input->pitch>MAX_P_CMD) ? MAX_P_CMD : input->pitch;
		input->Vyaw=(input->Vyaw>MAX_VYAW_CMD) ? MAX_VYAW_CMD : input->Vyaw;
		input->gas=(input->gas>MAX_GAS_CMD) ? MAX_GAS_CMD : input->gas;
		input->roll= (input->roll<-MAX_R_CMD) ? -MAX_R_CMD : input->roll;
		input->pitch=(input->pitch<-MAX_P_CMD) ? -MAX_P_CMD : input->pitch;
		input->Vyaw=(input->Vyaw<-MAX_VYAW_CMD) ? -MAX_VYAW_CMD : input->Vyaw;
		input->gas=(input->gas<-MAX_GAS_CMD) ? -MAX_GAS_CMD : input->gas;
	}
}

//function (state machine) which is going to make the drone react in case of an obstacle
unsigned char smartSafetyMode(fault_t fault, Navdata_t * droneState){
	static unsigned int timer=0;
	static unsigned char state=NO_SAFETY_MODE;
	unsigned char result=0;
	static fault_t savedFault=NO_FAULT;

	if(options.sysUrgenceExtreme!=0) {
		if( (fabsf(droneState->roll))>EXTREM_R || (fabsf(droneState->pitch))>EXTREM_P) {
			emergency = LAND;
			switch_to_sfm();
			set_sfm_command (NULL, LANDING_REQUEST);
			switch_to_nominal_mode();
		}
	}

	timer++;
	switch(state) {
	case NO_SAFETY_MODE:         //Waiting for a fault
		timer=0;
		savedFault=fault;
		result= NO_SAFETY_MODE;
		emergency = NO_EMERGENCY;
		if(fault!=NO_FAULT) {
			state=SAFETY_PROC;
		}
		break;
	case SAFETY_PROC:         //make the drone react according to the obstacle found
		switch_to_sfm();
		safetyProc(savedFault, droneState);
		if(savedFault==OBSTACLE_HAUT) {
			if(timer>TIME_SAFETY_PROC_LONG) {
				state=STABILISATION;
				timer=0;
			}
		}else{
			if(timer>TIME_SAFETY_PROC) {
				state=STABILISATION;
				timer=0;
			}
		}
		result= SAFETY_PROC;
		break;
	case STABILISATION:         //makes the drone stay in the same place where it is
		sfm_cmd.roll=0.0;
		sfm_cmd.pitch=0.0;
		sfm_cmd.Vyaw=0.0;
		sfm_cmd.gas=0.0;
		emergency = E_STABILISATION;
		set_sfm_command (&sfm_cmd, FLYING_REQUEST);
		if(timer>TIME_STABILISATION_PROC) {
			state=VERIFICATION;
			result= END_OF_PROC;
			timer=0;
		}else{
			result= STABILISATION;
		}
		break;

	case VERIFICATION:         //if there are no faults the drone is going to wait a period of time to receive more commands if there is still an error the drone is going to land
		emergency = E_VERIFICATION;
		if(fault!=NO_FAULT) {
			emergency = VERIFICATION_FAILED;
			set_sfm_command (NULL, LANDING_REQUEST);
			switch_to_nominal_mode();
		}
		if(timer>TIME_VERIFICATION_PROC) {
			state=NO_SAFETY_MODE;
			timer=0;
			switch_to_nominal_mode();
		}
		result=VERIFICATION;
		break;
	}
	if(options.debug!=0) {

		fprintf(logSFM,"[sfm]: state:%u R:%f,P:%f,Y:%f,G:%f\n Emergency : %d \n",result,sfm_cmd.roll,sfm_cmd.pitch,sfm_cmd.Vyaw,sfm_cmd.gas,emergency); //writes the value of the commands send to the drone in the log file
	}
	return result;


}


//function which lets the other threads write on the variable opt
void writeOpt(options_t * opt){
	if (vp_os_mutex_trylock(&opt_mutex) == C_OK) {
		memcpy(&options,opt, sizeof(options_t));
		vp_os_mutex_unlock(&opt_mutex);
	}
}

//function which lets the other threads read on the variable opt
void readOpt(options_t* opt){
	if (vp_os_mutex_trylock(&opt_mutex) == C_OK) {
		memcpy(opt,&options,sizeof(options_t));
		vp_os_mutex_unlock(&opt_mutex);
	}
}

//function which lets the other threads write on the variable signature
void readSignature(alarm_t * signature){
	if (vp_os_mutex_trylock(&sign_mutex) == C_OK) {
		memcpy(signature,&alarm,SIGN_TAB_NB_RESIDU*sizeof(alarm_t));
		vp_os_mutex_unlock(&sign_mutex);
	}
}

//function which lets the other threads read the variable data
void readNavdata(Navdata_t* data){
	if (vp_os_mutex_trylock(&data_mutex) == C_OK) {
		memcpy(data,&saveFiltered1,sizeof(Navdata_t));
		vp_os_mutex_unlock(&data_mutex);
	}
}

//function which lets the other threads read the variable out
void readFault(fault_t* out){
	if (vp_os_mutex_trylock(&fault_mutex) == C_OK) {
		*out=currentFault;
		vp_os_mutex_unlock(&fault_mutex);
	}
}

void readEmergency(emergency_state* em){
	if (vp_os_mutex_trylock(&emergency_mutex) == C_OK) {
		*em=emergency;
		vp_os_mutex_unlock(&emergency_mutex);
	}
}
