/**
 * @file    mission.c
 * @author  ShaggyDogs
 * @brief   Contains all the functions used to send commands to the drone
 * @version 2.0
 * @date    November 2014
 */

#include <stdio.h>
#include <stdlib.h>

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

#include "mission.h"
#include "smartfox_api.h"
#include "user_app.h"
#include "ardrone_move_cmd.h"
#include "Model/model.h"
#include "Model/residue.h"

#define MISSION_SFS_1 1
#define MISSION_SFS_2 2
#define MISSION_WALL_1 3 //Forward
#define MISSION_WALL_2 4 //Right
#define MISSION_WALL_3 5 //Backward
#define MISSION_WALL_4 6 //Left

static vp_os_mutex_t class_mutex;
extern options_t options;

DEFINE_THREAD_ROUTINE(mission, data) {

	int mission_nb = MISSION_SFS_2;

    printf("missionModeOn=%d\n",options.mission);
	while(options.mission==0){
		
        //printf("c'est la déprime\n");
        usleep(1000000);
    }

	switch (mission_nb) {

		case MISSION_SFS_1 :

			mission_SFS_1();
			break;

		case MISSION_SFS_2 :

			mission_SFS_2();
			break;

		case MISSION_WALL_1 :

			mission_WALL_1();
			break;

		case MISSION_WALL_2:

			mission_WALL_2();
			break;

		case MISSION_WALL_3:

			mission_WALL_3();
			break;

		case MISSION_WALL_4:

			mission_WALL_4();
			break;
	}

return 0;

}

void mission_SFS_1 () {

	drone_state_t status = get_drone_state();
	float command;
	Inputs_t lastcommand;
	commandType_t type;
	int etat = TAKEOFF_DRONE;
	vp_os_mutex_init(&class_mutex);

    while (1) {
		switch (etat) {

			case TAKEOFF_DRONE :
				vp_os_mutex_lock(&class_mutex);
   				class_id=0;
  				vp_os_mutex_unlock(&class_mutex);
				takeoff();
				usleep(5000000);
				vp_os_mutex_lock(&class_mutex);
   				class_id=1;
  				vp_os_mutex_unlock(&class_mutex);
				usleep(5000000);
				get_command(&lastcommand, &type);// Type : TAKEOFF_REQUEST, 	FLYING_REQUEST, LANDING_REQUEST
				status = get_drone_state();
				if (status == FLYING){
					etat = FORWARD_PITCH;
				}
				break;

			case FORWARD_PITCH :

				vp_os_mutex_lock(&class_mutex);
                class_id=0;
                vp_os_mutex_unlock(&class_mutex);
				command = pitch(-0.2, 1000000);
				if (command != 0) {
					etat = LEFT_ROLL;
				}
				break;

			case LEFT_ROLL :

				vp_os_mutex_lock(&class_mutex);
                class_id=0;
                vp_os_mutex_unlock(&class_mutex);
				command = roll(-0.2,1000000);
				if (command != 0) {
					etat = BACKWARD_PITCH;
				}
				break;

			case BACKWARD_PITCH :

				vp_os_mutex_lock(&class_mutex);
                class_id=0;
                vp_os_mutex_unlock(&class_mutex);
				command = pitch(0.2,1000000);
				if (command != 0) {
					etat = RIGHT_ROLL;
				}
				break;

			case RIGHT_ROLL :

				vp_os_mutex_lock(&class_mutex);
                class_id=0;
                vp_os_mutex_unlock(&class_mutex);
				command = roll(0.2,1000000);
				if (command != 0) {
					etat = GAS_UP;
				}
				break;

			case GAS_UP :

				vp_os_mutex_lock(&class_mutex);
                class_id=0;
                vp_os_mutex_unlock(&class_mutex);
				command = gas(0.4,2000000);
				if (command != 0) {
					etat = LEFT_YAW;
				}
				break;

			case LEFT_YAW:

				vp_os_mutex_lock(&class_mutex);
                class_id=0;
                vp_os_mutex_unlock(&class_mutex);
				command = yaw(-1.0,2000000);
				if (command != 0) {
					etat = GAS_DOWN;
				}
				break;

			case GAS_DOWN:

				vp_os_mutex_lock(&class_mutex);
                class_id=0;
                vp_os_mutex_unlock(&class_mutex);
				command = gas(-0.4,2000000);
				if (command != 0) {
					etat = RIGHT_YAW;
				}
				break;

			case RIGHT_YAW:

				vp_os_mutex_lock(&class_mutex);
                class_id=0;
                vp_os_mutex_unlock(&class_mutex);
				command = yaw(1.0,2000000);
				if (command != 0) {
					etat = LAND_DRONE;
				}
				break;

			case LAND_DRONE :

				vp_os_mutex_lock(&class_mutex);
                class_id=0;
                vp_os_mutex_unlock(&class_mutex);
				landing();
				break;
		}
	}

}

void mission_SFS_2() {

	drone_state_t status = get_drone_state();
	float command;
	float fin;
	Inputs_t lastcommand;
	commandType_t type;
	int etat = TAKEOFF_DRONE;
	vp_os_mutex_init(&class_mutex);

	while (1) {
		switch (etat) {

			case TAKEOFF_DRONE :
				vp_os_mutex_lock(&class_mutex);
   				class_id=0;
  				vp_os_mutex_unlock(&class_mutex);
				takeoff();
				usleep(5000000);
				vp_os_mutex_lock(&class_mutex);
   				class_id=1;
  				vp_os_mutex_unlock(&class_mutex);
				usleep(5000000);
				get_command(&lastcommand, &type);// Type : TAKEOFF_REQUEST, 	FLYING_REQUEST, LANDING_REQUEST
				status = get_drone_state();
				if (status == FLYING){
					etat = FORWARD_PITCH;
				}
				break;

			case FORWARD_PITCH :

				//void apply_command(roll, pitch, yaw, gas)
				vp_os_mutex_lock(&class_mutex);
                class_id=0;
                vp_os_mutex_unlock(&class_mutex);
				command = pitch(-0.2, 1000000);
				if (command != 0) {
					etat = GAS_UP;
				}
				break;

			case GAS_UP :

				vp_os_mutex_lock(&class_mutex);
                class_id=0;
                vp_os_mutex_unlock(&class_mutex);
				command = gas(0.4,2000000);
				if (command != 0) {
					etat = HOVER_DRONE;
				}
				break;

			case HOVER_DRONE :

				vp_os_mutex_lock(&class_mutex);
                class_id=1;
                vp_os_mutex_unlock(&class_mutex);
				fin = hover(5000000);
				if (fin == 1)
					etat = LEFT_YAW;
				break;

			case LEFT_YAW:

				vp_os_mutex_lock(&class_mutex);
                class_id=0;
                vp_os_mutex_unlock(&class_mutex);
				command = yaw(-1.0,2000000);
				if (command != 0) {
					etat = GAS_DOWN;
				}
				break;

			case GAS_DOWN:

				vp_os_mutex_lock(&class_mutex);
                class_id=0;
                vp_os_mutex_unlock(&class_mutex);
				command = gas(-0.4,2000000);
				if (command != 0) {
					etat = RIGHT_YAW;
				}
				break;

			case RIGHT_YAW:

				vp_os_mutex_lock(&class_mutex);
                class_id=0;
                vp_os_mutex_unlock(&class_mutex);
				command = yaw(1.0,2000000);
				if (command != 0) {
					etat = LAND_DRONE;
				}
				break;

			case LAND_DRONE :

				vp_os_mutex_lock(&class_mutex);
                class_id=0;
                vp_os_mutex_unlock(&class_mutex);
				landing();
				break;

		}
	}

}

void mission_WALL_1() {

	drone_state_t status = get_drone_state();
	float command;
	float fin;
	Inputs_t lastcommand;
	commandType_t type;
	int etat = TAKEOFF_DRONE;
	vp_os_mutex_init(&class_mutex);
	while (1) {
		switch (etat) {

			case TAKEOFF_DRONE :
				vp_os_mutex_lock(&class_mutex);
   				class_id=0;
  				vp_os_mutex_unlock(&class_mutex);
				takeoff();
				usleep(5000000);
				vp_os_mutex_lock(&class_mutex);
   				class_id=1;
  				vp_os_mutex_unlock(&class_mutex);
				usleep(5000000);
				get_command(&lastcommand, &type);// Type : TAKEOFF_REQUEST, 	FLYING_REQUEST, LANDING_REQUEST
				status = get_drone_state();
				if (status == FLYING){
					etat = GAS_DOWN;
				}
				break;

			case GAS_DOWN:

				vp_os_mutex_lock(&class_mutex);
                class_id=0;
                vp_os_mutex_unlock(&class_mutex);
				command = gas(-0.4,1000000);
				if (command != 0) {
					etat = HOVER_DRONE;
				}
				break;

			case HOVER_DRONE :

				vp_os_mutex_lock(&class_mutex);
                class_id=1;
                vp_os_mutex_unlock(&class_mutex);
				fin = hover(2000000);
				if (fin == 1)
					etat = FORWARD_PITCH;
				break;

			case FORWARD_PITCH :

				//void apply_command(roll, pitch, yaw, gas)
				vp_os_mutex_lock(&class_mutex);
                class_id=0;
                vp_os_mutex_unlock(&class_mutex);
				command = pitch(-0.1, 10000000);
				if (command != 0) {
					etat = LAND_DRONE;
				}
				break;

			case LAND_DRONE :

				vp_os_mutex_lock(&class_mutex);
                class_id=0;
                vp_os_mutex_unlock(&class_mutex);
				landing();
				break;
		}
	}
}

void mission_WALL_2() {

	drone_state_t status = get_drone_state();
	float command;
	float fin;
	Inputs_t lastcommand;
	commandType_t type;
	int etat = TAKEOFF_DRONE;
	vp_os_mutex_init(&class_mutex);
	while (1) {
		switch (etat) {

			case TAKEOFF_DRONE :
				vp_os_mutex_lock(&class_mutex);
   				class_id=0;
  				vp_os_mutex_unlock(&class_mutex);
				takeoff();
				usleep(5000000);
				vp_os_mutex_lock(&class_mutex);
   				class_id=1;
  				vp_os_mutex_unlock(&class_mutex);
				usleep(5000000);
				get_command(&lastcommand, &type);// Type : TAKEOFF_REQUEST, 	FLYING_REQUEST, LANDING_REQUEST
				status = get_drone_state();
				if (status == FLYING){
					etat = GAS_DOWN;
				}
				break;

			case GAS_DOWN:

				vp_os_mutex_lock(&class_mutex);
                class_id=0;
                vp_os_mutex_unlock(&class_mutex);
				command = gas(-0.4,1000000);
				if (command != 0) {
					etat = HOVER_DRONE;
				}
				break;

			case HOVER_DRONE :

				vp_os_mutex_lock(&class_mutex);
                class_id=1;
                vp_os_mutex_unlock(&class_mutex);
				fin = hover(2000000);
				if (fin == 1)
					etat = RIGHT_YAW;
				break;

			case RIGHT_YAW :

				//void apply_command(roll, pitch, yaw, gas)
				vp_os_mutex_lock(&class_mutex);
                class_id=0;
                vp_os_mutex_unlock(&class_mutex);
				command = roll(0.1, 10000000);
				if (command != 0) {
					etat = LAND_DRONE;
				}
				break;

			case LAND_DRONE :

				vp_os_mutex_lock(&class_mutex);
                class_id=0;
                vp_os_mutex_unlock(&class_mutex);
				landing();
				break;
		}
	}
}

void mission_WALL_3() {

	drone_state_t status = get_drone_state();
	float command;
	float fin;
	Inputs_t lastcommand;
	commandType_t type;
	int etat = TAKEOFF_DRONE;
	vp_os_mutex_init(&class_mutex);
	while (1) {
		switch (etat) {

			case TAKEOFF_DRONE :
				vp_os_mutex_lock(&class_mutex);
   				class_id=0;
  				vp_os_mutex_unlock(&class_mutex);
				takeoff();
				usleep(5000000);
				vp_os_mutex_lock(&class_mutex);
   				class_id=1;
  				vp_os_mutex_unlock(&class_mutex);
				usleep(5000000);
				get_command(&lastcommand, &type);// Type : TAKEOFF_REQUEST, 	FLYING_REQUEST, LANDING_REQUEST
				status = get_drone_state();
				if (status == FLYING){
					etat = GAS_DOWN;
				}
				break;

			case GAS_DOWN:

				vp_os_mutex_lock(&class_mutex);
                class_id=0;
                vp_os_mutex_unlock(&class_mutex);
				command = gas(-0.4,1000000);
				if (command != 0) {
					etat = HOVER_DRONE;
				}
				break;

			case HOVER_DRONE :

				vp_os_mutex_lock(&class_mutex);
                class_id=1;
                vp_os_mutex_unlock(&class_mutex);
				fin = hover(2000000);
				if (fin == 1)
					etat = BACKWARD_PITCH;
				break;

			case BACKWARD_PITCH :

				//void apply_command(roll, pitch, yaw, gas)
				vp_os_mutex_lock(&class_mutex);
                class_id=0;
                vp_os_mutex_unlock(&class_mutex);
				command = pitch(0.1, 10000000);
				if (command != 0) {
					etat = LAND_DRONE;
				}
				break;

			case LAND_DRONE :

				vp_os_mutex_lock(&class_mutex);
                class_id=0;
                vp_os_mutex_unlock(&class_mutex);
				landing();
				break;
		}
	}
}

void mission_WALL_4() {

	drone_state_t status = get_drone_state();
	float command;
	float fin;
	Inputs_t lastcommand;
	commandType_t type;
	int etat = TAKEOFF_DRONE;
	vp_os_mutex_init(&class_mutex);
	while (1) {
		switch (etat) {

			case TAKEOFF_DRONE :
				vp_os_mutex_lock(&class_mutex);
   				class_id=0;
  				vp_os_mutex_unlock(&class_mutex);
				takeoff();
				usleep(5000000);
				vp_os_mutex_lock(&class_mutex);
   				class_id=1;
  				vp_os_mutex_unlock(&class_mutex);
				usleep(5000000);
				get_command(&lastcommand, &type);// Type : TAKEOFF_REQUEST, 	FLYING_REQUEST, LANDING_REQUEST
				status = get_drone_state();
				if (status == FLYING){
					etat = GAS_DOWN;
				}
				break;

			case GAS_DOWN:

				vp_os_mutex_lock(&class_mutex);
                class_id=0;
                vp_os_mutex_unlock(&class_mutex);
				command = gas(-0.4,1000000);
				if (command != 0) {
					etat = HOVER_DRONE;
				}
				break;

			case HOVER_DRONE :

				vp_os_mutex_lock(&class_mutex);
                class_id=1;
                vp_os_mutex_unlock(&class_mutex);
				fin = hover(2000000);
				if (fin == 1)
					etat = LEFT_YAW;
				break;

			case LEFT_YAW :

				//void apply_command(roll, pitch, yaw, gas)
				vp_os_mutex_lock(&class_mutex);
                class_id=0;
                vp_os_mutex_unlock(&class_mutex);
				command = roll(-0.1, 10000000);
				if (command != 0) {
					etat = LAND_DRONE;
				}
				break;

			case LAND_DRONE :

				vp_os_mutex_lock(&class_mutex);
                class_id=0;
                vp_os_mutex_unlock(&class_mutex);
				landing();
				break;
		}
	}
}

float roll(float value, int us) {

	Inputs_t lastcommand;
	commandType_t type;

	apply_command(value, 0.0, 0.0, 0.0);
	usleep(us);
	get_command(&lastcommand, &type);

	return lastcommand.roll;
}

float pitch(float value, int us) {

	Inputs_t lastcommand;
	commandType_t type;

	apply_command(0.0, value, 0.0, 0.0);
	usleep(us);
	get_command(&lastcommand, &type);

	return lastcommand.pitch;
}

float yaw(float value, int us) {

	Inputs_t lastcommand;
	commandType_t type;

	apply_command(0.0, 0.0, value, 0.0);
	usleep(us);
	get_command(&lastcommand, &type);

	return lastcommand.Vyaw;
}

float gas(float value, int us) {

	Inputs_t lastcommand;
	commandType_t type;

	apply_command(0.0, 0.0, 0.0, value);
	usleep(us);
	get_command(&lastcommand, &type);

	return lastcommand.gas;
}

float hover(int us) {

	Inputs_t lastcommand;
	commandType_t type;
	int end = 0;

	hovering();
	usleep(us);
	get_command(&lastcommand, &type);

	if ((lastcommand.roll == 0.0) && (lastcommand.pitch == 0.0) && (lastcommand.Vyaw == 0.0) && (lastcommand.gas == 0.0)) {
		end = 1;
	}
	return end;
}

