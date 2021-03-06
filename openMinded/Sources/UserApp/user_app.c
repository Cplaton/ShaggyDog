/**
 * @file    user_app.c
 * @author  gayraudbenoit@gmail.com (Smartfox), modified by ShaggyDogs
 * @brief   Contains all the functions used to send commands to the drone
 * @version 2.0
 * @date    January 2014 (last modified: Nov 2014)
 **/
#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "smartfox_api.h"
#include "user_app.h"


static vp_os_mutex_t class_mutex;
static vp_os_mutex_t reaction_mutex;

/*to stop diagnosis app*/
void kill_drone();

DEFINE_THREAD_ROUTINE(th_user_app, data)
{

	/* ############################################################ */
	/* ################### BEGINNING CUSTOM CODE ################## */
	/* ###################### user thread ######################### */
	/* ############################################################ */

	char keyboard_file[50];

	struct input_event ev;
	int fd;

	vp_os_mutex_init(&class_mutex);
	vp_os_mutex_init(&reaction_mutex);

	printf("STARTING USER APP................. \n");

	init_userapp(keyboard_file,50);

	fd = open(keyboard_file, O_RDONLY);

	if (fd < 0)
	{
		fprintf(stderr, "failed to open input device %s: %s\n", keyboard_file, strerror(errno));
		exit(1);
	}

	while (1)
	{
		if (read(fd, &ev, sizeof(struct input_event)) < 0)
		{
			fprintf(stderr, "failed to read input event from input device %s: %s\n", keyboard_file, strerror(errno));
			if (errno == EINTR)
				continue;
			break;
		}

		if (ev.type == EV_KEY && modeReaction == 0) {
			extract_key_event(&ev);
		}


	}

	close(fd);

	/* ############################################################ */
	/* ###################### END CUSTOM CODE ##################### */
	/* ############################################################ */

	return 0;
}

/* ############################################################ */
/* ################### BEGINNING CUSTOM CODE ################## */
/* ####################### functions  ######################### */
/* ############################################################ */

/* initialization function : reads the conf file */
void init_userapp(char * keyboard_file, size_t len) {
	char line[150];
	char conf_param[50];
	FILE * fd;

	if ((fd = fopen(CONF_FILE,"r")) == NULL) {
		perror("configuration file openning : ");
		exit(1);
	}

	vp_os_memset(line,0,150);
	vp_os_memset(conf_param,0,50);
	vp_os_memset(keyboard_file,0,len);

	if (fgets(line,150,fd) == NULL) {
		fprintf(stderr,"invalid configuration file : impossible to get the keyboard file\n");
	}

	sscanf(line,"%s %s",conf_param,keyboard_file);


	if (fclose(fd) == EOF) {
		perror("configuration file closing : ");
	}

}

/* extract the key that triggered the event and send the matching command to the drone */
void extract_key_event(struct input_event * ev) {

	Inputs_t lastcommand;
	commandType_t type;

	switch (ev->code) {

	case GO_FORWARD:
		if (ev->value == 1) {
			apply_command(0.0,-0.2,0.0,0.0);

		} else if (ev->value == 0) {
			hovering();

		}
		break;

	case GO_BACK:
		if (ev->value == 1) {
			apply_command(0.0,0.2,0.0,0.0);

		} else if (ev->value == 0) {
			hovering();
		}
		break;

	case GO_LEFT:
		if (ev->value == 1) {
			apply_command(-0.2,0.0,0.0,0.0);

		} else if (ev->value == 0) {
			hovering();
		}
		break;

	case GO_RIGHT:
		if (ev->value == 1) {
			apply_command(0.2,0.0,0.0,0.0);

		} else if (ev->value == 0) {
			hovering();
		}
		break;

	case GO_UP:
		if (ev->value == 1) {
			apply_command(0.0,0.0,0.0,0.4);

		} else if (ev->value == 0) {
			hovering();
		}
		break;

	case GO_DOWN:
		if (ev->value == 1) {
			apply_command(0.0,0.0,0.0,-0.4);

		} else if (ev->value == 0) {
			hovering();

		}
		break;

	case ROTATE_RIGHT:
		if (ev->value == 1) {
			apply_command(0.0,0.0,0.5,0.0);

		} else if (ev->value == 0) {
			hovering();
		}
		break;

	case ROTATE_LEFT:
		if (ev->value == 1) {
			apply_command(0.0,0.0,-0.4,0.0);

		} else if (ev->value == 0) {

			hovering();
		}
		break;
	case LANDING:
		landing();
		break;
	case TAKEOFF:
		takeoff();

		break;
	case KILL:
		kill_drone();
		break;
	case CLASS_WALL:
		get_command(&lastcommand, &type);
		printf("Pitch = %f\n", lastcommand.pitch);
		if(ev->value==1) {
			if (lastcommand.pitch > 0.0) {
				vp_os_mutex_lock(&class_mutex);
				class_id=3;
				vp_os_mutex_unlock(&class_mutex);
			}
			else if (lastcommand.pitch < 0.0) {
				vp_os_mutex_lock(&class_mutex);
				class_id=2;
				vp_os_mutex_unlock(&class_mutex);
			}
			else if (lastcommand.roll > 0.0) {
				vp_os_mutex_lock(&class_mutex);
				class_id=4;
				vp_os_mutex_unlock(&class_mutex);
			}
			else if (lastcommand.roll < 0.0) {
				vp_os_mutex_lock(&class_mutex);
				class_id=5;
				vp_os_mutex_unlock(&class_mutex);
			}

		}
		/*
		   else{
		   vp_os_mutex_lock(&class_mutex);
		   class_id=0;
		   vp_os_mutex_unlock(&class_mutex);}
		 */
		break;

	default:;
	}
}

/* to make the drone hovers over the floor*/
void hovering() {
	Inputs_t cmd;
	cmd.roll = 0.0;
	cmd.pitch = 0.0;
	cmd.Vyaw = 0.0;
	cmd.gas = 0.0;

	set_command (&cmd,FLYING_REQUEST);

}

/* to send a specifing command to the drone while it is flying */
void apply_command(float roll, float pitch, float Vyaw, float gas) {

	Inputs_t cmd;
	cmd.roll = roll;
	cmd.pitch = pitch;
	cmd.Vyaw = Vyaw;
	cmd.gas = gas;

	set_command (&cmd,FLYING_REQUEST);

}

/* to make the drone lands */
void landing() {

	set_command (NULL,LANDING_REQUEST);

}

/* to make the drone takes off */
void takeoff() {

	set_command (NULL,TAKEOFF_REQUEST);

}

void kill_drone(){
	set_command(NULL,KILL_REQUEST);
}


/* ############################################################ */
/* ###################### END CUSTOM CODE ##################### */
/* ############################################################ */

