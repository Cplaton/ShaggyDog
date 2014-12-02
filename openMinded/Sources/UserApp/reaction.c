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
#include "reaction.h"

static vp_os_mutex_t class_mutex;


DEFINE_THREAD_ROUTINE(reaction, data) {

	while (1) {

		if (modeReaction == 1) {

			printf("Module de reaciton active\n");
			avoid_front_wall();

		}	
	}

	return 0;
}

void avoid_front_wall () {

	float command;
	float fin;
	int etat = BACKWARD_PITCH;
	vp_os_mutex_init(&class_mutex);
	while (1) {
		switch (etat) {

			case BACKWARD_PITCH :
				vp_os_mutex_lock(&class_mutex);
   				class_id=0;
  				vp_os_mutex_unlock(&class_mutex);
				
				command = pitch(0.2,1000000);
	
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
					etat = END_REACTION;
				break;
			
			case END_REACTION :
				
				modeReaction = 0;
				break;	
		}
	}

}

void avoid_left_wall () {

	float command;
	float fin;
	int etat = RIGHT_ROLL;
	vp_os_mutex_init(&class_mutex);
	while (1) {
		switch (etat) {

			case RIGHT_ROLL :

				//void apply_command(roll, pitch, yaw, gas)
				vp_os_mutex_lock(&class_mutex);
                class_id=0;
                vp_os_mutex_unlock(&class_mutex);
				command = roll(0.2, 10000000);
				if (command != 0) {
					etat = LAND_DRONE;
				}
				break;

			case HOVER_DRONE :

				vp_os_mutex_lock(&class_mutex);
                class_id=1;
                vp_os_mutex_unlock(&class_mutex);
				fin = hover(2000000);
				if (fin == 1)
					etat = END_REACTION;
				break;
			
			case END_REACTION :
				
				modeReaction = 0;
				break;	
		}
	}

}

void avoid_back_wall () {

	float command;
	float fin;
	int etat = FORWARD_PITCH;
	vp_os_mutex_init(&class_mutex);
	while (1) {
		switch (etat) {

			case FORWARD_PITCH :

				//void apply_command(roll, pitch, yaw, gas)
				vp_os_mutex_lock(&class_mutex);
                class_id=0;
                vp_os_mutex_unlock(&class_mutex);
				command = pitch(-0.2, 10000000);
				if (command != 0) {
					etat = LAND_DRONE;
				}
				break;

			case HOVER_DRONE :

				vp_os_mutex_lock(&class_mutex);
                class_id=1;
                vp_os_mutex_unlock(&class_mutex);
				fin = hover(2000000);
				if (fin == 1)
					etat = END_REACTION;
				break;
			
			case END_REACTION :
				
				modeReaction = 0;
				break;	
		}
	}


}

void avoid_right_wall () {

	float command;
	float fin;
	int etat = LEFT_ROLL;
	vp_os_mutex_init(&class_mutex);
	while (1) {
		switch (etat) {

			case LEFT_ROLL :

				//void apply_command(roll, pitch, yaw, gas)
				vp_os_mutex_lock(&class_mutex);
                class_id=0;
                vp_os_mutex_unlock(&class_mutex);
				command = roll(-0.2, 10000000);
				if (command != 0) {
					etat = LAND_DRONE;
				}
				break;

			case HOVER_DRONE :

				vp_os_mutex_lock(&class_mutex);
                class_id=1;
                vp_os_mutex_unlock(&class_mutex);
				fin = hover(2000000);
				if (fin == 1)
					etat = END_REACTION;
				break;
			
			case END_REACTION :
				
				modeReaction = 0;
				break;	
		}
	}

}