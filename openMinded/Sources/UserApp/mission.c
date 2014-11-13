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

#define MISSION_SFS_1 1
#define MISSION_SFS_2 2
#define MISSION_WALL_1 3

DEFINE_THREAD_ROUTINE(mission, data) {
	
	int mission_nb = MISSION_SFS_1;
	
	
		
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

	}	
		
	
			
return 0;

}

void mission_SFS_1 () {

	drone_state_t status = get_drone_state();
	float command;
	float fin;
	Inputs_t lastcommand;
	commandType_t type;
	int etat = 1;
	int ancien_etat = 1;
	static vp_os_mutex_t class_mutex;	
	vp_os_mutex_init(&class_mutex);
	
	while (1) {
		switch (etat) {
			printf("Etat %d\n", etat);
			case 1 :
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
				if ((type != TAKEOFF_REQUEST) && (status == FLYING)){
					etat = 2;
					printf("Passage à l'état 2\n");
				}		
				break;
			
			case 2 :			
				
				
				//void apply_command(roll, pitch, yaw, gas)
				vp_os_mutex_lock(&class_mutex);
                class_id=0;
                vp_os_mutex_unlock(&class_mutex);
				command = pitch(-0.2, 1000000);
				if (command != 0) {
					etat = 3;
					ancien_etat = 2;
					printf("Passage à l'état 3\n");
				}
				break;
			case 3 :
	

				vp_os_mutex_lock(&class_mutex);
                class_id=0;
                vp_os_mutex_unlock(&class_mutex);
				command = roll(-0.2,1000000);
				if (command != 0) {
					etat = 4;
					printf("Passage à l'état 4\n");
					ancien_etat = 3;
				}
				break;
			case 4 : 
				vp_os_mutex_lock(&class_mutex);
                class_id=0;
                vp_os_mutex_unlock(&class_mutex);
				command = pitch(0.2,1000000);
				if (command != 0) {
					etat = 5;
					printf("Passage à l'état 5\n");
					ancien_etat = 4;
				}
				break;
			case 5 :

				vp_os_mutex_lock(&class_mutex);
                class_id=0;
                vp_os_mutex_unlock(&class_mutex);
				command = roll(0.2,1000000);
				if (command != 0) {
					printf("Passage à l'état 6");
					etat = 6;
					ancien_etat = 5;
				}
				break;			
			case 6 : 

				vp_os_mutex_lock(&class_mutex);
                class_id=0;
                vp_os_mutex_unlock(&class_mutex);
				command = gas(0.4,2000000);	
				if (command != 0) {
					printf("Passage à l'état 7");
					etat = 7;
					ancien_etat = 6;
				}
				break;	
			case 7:

				vp_os_mutex_lock(&class_mutex);
                class_id=0;
                vp_os_mutex_unlock(&class_mutex);
				command = yaw(-1.0,2000000);	
				if (command != 0) {
					printf("Passage à l'état 8");
					etat = 8;
					ancien_etat = 7;
				}
				break;				
			
			case 8:

				vp_os_mutex_lock(&class_mutex);
                class_id=0;
                vp_os_mutex_unlock(&class_mutex);
				command = gas(-0.4,2000000);	
				if (command != 0) {
					printf("Passage à l'état 9");
					etat = 9;
					ancien_etat = 8;
				}
				break;	

			case 9:

				vp_os_mutex_lock(&class_mutex);
                class_id=0;
                vp_os_mutex_unlock(&class_mutex);
				command = yaw(1.0,2000000);	
				if (command != 0) {
					printf("Passage à l'état 10");
					etat = 10;
					ancien_etat = 9;
				}
				break;	

			case 10 :

				vp_os_mutex_lock(&class_mutex);
                class_id=0;
                vp_os_mutex_unlock(&class_mutex);
				landing();
				break;					
			case 11 : 

				vp_os_mutex_lock(&class_mutex);
                class_id=0;
                vp_os_mutex_unlock(&class_mutex);
				fin = hover(5000000);
				if (fin == 1)
					etat = ancien_etat + 1;
				break;				
		}
	}	

}

void mission_SFS_2() {


}

void mission_WALL_1() {


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

