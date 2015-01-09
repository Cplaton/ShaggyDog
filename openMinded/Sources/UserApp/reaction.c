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

#include "../UI/displayMsg.h"
#include "../Model/residue.h" // pour l'acces a la structure utilisee dans les messages

#define FRONT_WALL 2
#define BACK_WALL  3
#define RIGHT_WALL 4
#define LEFT_WALL  5

int enable_openMinded_safety_mode=0;
int modeReaction = 0;


static vp_os_mutex_t class_mutex;
static vp_os_mutex_t reaction_mutex;

DEFINE_THREAD_ROUTINE(reaction, data) {

	while (1) {

		usleep(100);
		if (modeReaction == 0 && enable_openMinded_safety_mode==1)
			check_situation();	
	}

	return 0;
}

void avoid_front_wall () {

	float command;
	float fin;
	int etat = BACKWARD_PITCH;
	vp_os_mutex_init(&class_mutex);
	while (modeReaction == 1) {
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
				vp_os_mutex_lock(&reaction_mutex);
				modeReaction = 0;
				vp_os_mutex_unlock(&reaction_mutex);
				displayEmergencyMsg(NO_EMERGENCY);
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
				command = roll(0.2, 1000000);
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
				vp_os_mutex_lock(&reaction_mutex);
				modeReaction = 0;
				vp_os_mutex_unlock(&reaction_mutex);
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
				command = pitch(-0.2, 1000000);
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
				command = roll(-0.2, 1000000);
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
					etat = HOVER_DRONE;
				break;
			
			case END_REACTION :
				
				modeReaction = 0;
				break;	
		}
	}

}

void check_situation () {

	switch(class_id) {

		case FRONT_WALL:
			printf("Reaction.c class_id = %d\n", class_id);
			vp_os_mutex_lock(&reaction_mutex);
			modeReaction = 1;
			vp_os_mutex_unlock(&reaction_mutex);
			
			displayAlertMsg(OBSTACLE_DEVANT);
			displayEmergencyMsg(GO_BACK);
			
			avoid_front_wall();
			
			break;

		case BACK_WALL:
			printf("Reaction.c class_id = %d\n", class_id);
			vp_os_mutex_lock(&reaction_mutex);
			modeReaction = 1;
			vp_os_mutex_unlock(&reaction_mutex);
			
			displayAlertMsg(OBSTACLE_ARRIERE);
			displayEmergencyMsg(GO_FORWARD);
			
			avoid_back_wall();
			break;

		case RIGHT_WALL:
			printf("Reaction.c class_id = %d\n", class_id);
			vp_os_mutex_lock(&reaction_mutex);
			modeReaction = 1;
			vp_os_mutex_unlock(&reaction_mutex);
			
			displayAlertMsg(OBSTACLE_DROITE);
			displayEmergencyMsg(GO_LEFT);
			
			avoid_right_wall();
			break;

		case LEFT_WALL:
			printf("Reaction.c class_id = %d\n", class_id);
			vp_os_mutex_lock(&reaction_mutex);
			modeReaction = 1;
			vp_os_mutex_unlock(&reaction_mutex);

			displayAlertMsg(OBSTACLE_GAUCHE);
			displayEmergencyMsg(GO_RIGHT);
			
			avoid_left_wall();
			break;		

	}

}
