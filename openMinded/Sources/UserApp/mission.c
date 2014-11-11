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
#include "Navdata/navdata_analyse.h"


DEFINE_THREAD_ROUTINE(mission, data) {
	
	drone_state_t status = get_drone_state();
	float command;
	float fin;
	Inputs_t lastcommand;
	commandType_t type;
	int etat = 1;
	int ancien_etat = 1;
	int class_id;
	static vp_os_mutex_t class_mutex;	
	vp_os_mutex_init(&class_mutex);
	while(1) {
		
		printf("Etat %d\n", etat);
		switch (etat) {

			case 1 :
				
				takeoff();							
				usleep(5000000);
				get_command(&lastcommand, &type);// Type : TAKEOFF_REQUEST, 	FLYING_REQUEST, LANDING_REQUEST
				status = get_drone_state();				
				if ((type != TAKEOFF_REQUEST) && (status == FLYING)){
					etat = 2;
					vp_os_mutex_lock(&class_mutex);
   					class_id=1;
  					vp_os_mutex_unlock(&class_mutex);	
					printf("Passage à l'état 2\n");
				}		
				break;
			
			case 2 :			
				
				//void apply_command(roll, pitch, yaw, gas)
				command = pitch(-0.2, 1000000);
				vp_os_mutex_lock(&class_mutex);
                                class_id=0;
                                vp_os_mutex_unlock(&class_mutex);
				if (command != 0) {
					etat = 3;
					ancien_etat = 2;
					printf("Passage à l'état 11\n");
				}
				break;
			case 3 :
	


				command = roll(-0.2,1000000);
                                vp_os_mutex_lock(&class_mutex);
                                class_id=0;
                                vp_os_mutex_unlock(&class_mutex);	
				if (command != 0) {
					etat = 4;
					printf("Passage à l'état 11\n");
					ancien_etat = 3;
				}
				break;
			case 4 : 

				command = pitch(0.2,1000000);
                                vp_os_mutex_lock(&class_mutex);
                                class_id=0;
                                vp_os_mutex_unlock(&class_mutex);
				if (command != 0) {
					etat = 5;
					printf("Passage à l'état 11\n");
					ancien_etat = 4;
				}
				break;
			case 5 :

				command = roll(0.2,1000000);
                                vp_os_mutex_lock(&class_mutex);
                                class_id=0;
                                vp_os_mutex_unlock(&class_mutex);
				if (command != 0) {
					printf("Passage à l'état 11");
					etat = 6;
					ancien_etat = 5;
				}
				break;			
			case 6 : 

				command = gas(0.4,2000000);	
                                vp_os_mutex_lock(&class_mutex);
                                class_id=0;
                                vp_os_mutex_unlock(&class_mutex);		
				if (command != 0) {
					printf("Passage à l'état 11");
					etat = 7;
					ancien_etat = 6;
				}
				break;	
			case 7:

				command = yaw(-1.0,2000000);	
                                vp_os_mutex_lock(&class_mutex);
                                class_id=0;
                                vp_os_mutex_unlock(&class_mutex);		
				if (command != 0) {
					printf("Passage à l'état 11");
					etat = 8;
					ancien_etat = 7;
				}
				break;				
			
			case 8:
				command = gas(-0.4,2000000);	
                                vp_os_mutex_lock(&class_mutex);
                                class_id=0;
                                vp_os_mutex_unlock(&class_mutex);
				if (command != 0) {
					printf("Passage à l'état 11");
					etat = 9;
					ancien_etat = 8;
				}
				break;	

			case 9:
				command = yaw(1.0,2000000);	
                                vp_os_mutex_lock(&class_mutex);
                                class_id=0;
                                vp_os_mutex_unlock(&class_mutex);		
				if (command != 0) {
					printf("Passage à l'état 11");
					etat = 10;
					ancien_etat = 9;
				}
				break;	

			case 10 :

				landing();
                                vp_os_mutex_lock(&class_mutex);
                                class_id=0;
                                vp_os_mutex_unlock(&class_mutex);
				break;					
			case 11 : 

				fin = hover(5000000);
                                vp_os_mutex_lock(&class_mutex);
                                class_id=0;
                                vp_os_mutex_unlock(&class_mutex);
				if (fin == 1)
					etat = ancien_etat + 1;
				break;				
		}
	}
			
return 0;

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

