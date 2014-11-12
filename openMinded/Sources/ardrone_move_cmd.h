#ifndef ARDRONE_MOVE_CMD_H
#define ARDRONE_MOVE_CMD_H

/* ardrone_move_cmd.h */
#include <VP_Api/vp_api_thread_helper.h> // api pour les threads

#include "Model/model.h"
#include "Model/residue.h"
#include "Navdata/navdata_analyse.h"

#define LANDING_OPTION 0
#define TAKEOFF_OPTION 1
#define COMMAND_SENDING_PERIOD 250


PROTO_THREAD_ROUTINE(th_move_cmd, data);

typedef enum commandType {
  NO_REQUEST,
  TAKEOFF_REQUEST,
  LANDING_REQUEST,
  FLYING_REQUEST,
  KILL_REQUEST
} commandType_t;


typedef struct commandToSnd {
  Inputs_t command;
  commandType_t commandType;
} commandToSnd_t;


/*
 * To refresh the command to send to the drone in sfm
 * newCommand : the flight parameter of the command (yaw, pitch...)
 * type : the command type to land, take...
 * NOTE : this fonction does nothing if the drone is not in safety mode
 */
void set_sfm_command (Inputs_t * newCommand , commandType_t type);

/*
 * To get the last command sent to the drone
 * lastCommand : the flight parameter of the last command sent to the drone
 * type : the command type previously sent
 */
void get_command (Inputs_t * lastCommand , commandType_t * type);

/*
 * Enable to switch the drone into nominal mode i.e. the application
 * takes into account the user commands
 */
void switch_to_nominal_mode();

/*
 * Enable to switch the drone into the safety mode i.e. the application
 * blocks the user commands and takes control
 */
void switch_to_sfm();


/* ask for stopping sending commands : STOP the thread*/
void stop_sending_commands(); 
#endif
