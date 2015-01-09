/*---------------------------------------------
 * Author: Kevin Delmas
 * Date: 10.01.2014
 * Version: 1.0
 * File: model.c
 *---------------------------------------------
 */

#include "model.h"

/*-------------------------------------------
 *  DEFINES
 *-------------------------------------------
 */
#define PATH "./DataModel/"

//pitch transfer function => p0/(z-p1)
#define p0 1.109
#define p1 0.9390625

//Vx transfer function  => p2/(z-p3)
#define p2 -0.002866
#define p3 0.9930429

//roll transfer function  => r0/(z-r1)
#define r0 0.927977
#define r1 0.96906742

//Vy transfer function  => r2/(z-r3)
#define r2 0.00195582
#define r3 0.9921767

//Vz (Vertical velocity) transfer function  => g0/(z-g1)
#define g0 0.024952
#define g1 0.96906742

//Vyaw transfer function => y0/(z-y1)
#define y0 3.7628616
#define y1 0.95896552

#define Te 0.005

/*-------------------------------------------
 *  GLOBAL VARIABLES
 *-------------------------------------------
 */

Navdata_t saveValue;
Inputs_t saveInputs={0,0,0,0};
float32_t saveAltitude, saveYaw;

/*-------------------------------------------
 *  FUNCTIONS
 *-------------------------------------------
 */

//function which initialize the drone model with in values
void initModel(Navdata_t * initSaveValue, float32_t altitude, float32_t yaw){
	Navdata_t init={0.0, 0.0, 0.0,0.0,0.0,0.0};
	memcpy(&saveValue,&init,sizeof(Navdata_t)); //initialize saveValue
	saveValue.Vyaw=0;
	saveAltitude=altitude;
	saveYaw=yaw;
}

//function which calculates the outputs of the drone's mathematical model
void model(Inputs_t * inputs, Navdata_t * outputs){

	outputs->roll=saveValue.roll*r1+saveInputs.roll*r0; //calculate the roll transfer function output *

	outputs->pitch=saveValue.pitch*p1+saveInputs.pitch*p0; //calculate the pitch transfer function output *

	outputs->Vyaw=saveValue.Vyaw*y1+saveInputs.Vyaw*y0; //calculate the Vyaw transfer function output *

	outputs->Vz=saveValue.Vz*g1+saveInputs.gas*g0; //calculate the Vz transfer function output *

	outputs->Vy=saveValue.Vy*r3+outputs->roll*r2; //calculate the Vy transfer function output *

	outputs->Vx=saveValue.Vx*p3+outputs->pitch*p2; //calculate the Vx transfer function output *

	// * see the transfer functions in DEFINES


	memcpy(&saveValue,outputs,sizeof(Navdata_t)); //save the last drone's mathematical model outputs in saveValue
	memcpy(&saveInputs,inputs,sizeof(Inputs_t)); //save the last commands send to the drone values in saveInputs

}

//function which updates the navigation data with the new data from the drone
void updateNavdata(Navdata_t *selectedNavdata,const navdata_demo_t *nd){
	float32_t altitude= (float32_t)(nd->altitude);
	altitude/=1000;
	static float32_t lastVyaw=0;

	if(nd!=NULL && selectedNavdata!=NULL) {
		if(nd->psi*saveYaw<0) {
			selectedNavdata->Vyaw= lastVyaw;
		}else{
			selectedNavdata->Vyaw= (nd->psi/1000-saveYaw)/Te; //Vyaw calculation by finding the yaw derivate
			lastVyaw=selectedNavdata->Vyaw;
		}
		selectedNavdata->Vz= (altitude-saveAltitude)/Te; //Vz calculation by finding the altitude derivate
		saveAltitude=altitude;
		saveYaw=nd->psi/1000;
		selectedNavdata->roll=nd->phi/1000;
		selectedNavdata->pitch=nd->theta/1000;
		selectedNavdata->Vx=nd->vx/1000;
		selectedNavdata->Vy=nd->vy/1000;
	}

}

