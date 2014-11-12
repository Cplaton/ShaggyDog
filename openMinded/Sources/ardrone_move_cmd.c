/* ardrone_move_cmd.c */
#include <unistd.h>
#include <ardrone_api.h>
#include <ardrone_tool/UI/ardrone_input.h>
#include <ardrone_tool/ardrone_tool_configuration.h>
#include <utils.h>
#include "ardrone_move_cmd.h"
#include "UserApp/smartfox_api.h"

static commandToSnd_t current_command; // the command currently sent to the drone

static int isInSFM = 0; // indicates whether the drone is in safety mode or not
static vp_os_mutex_t sfm_state_mutex,
                     command_mutex,
                     stop_mutex;

static int stop_sending = 0;

/*
 * send the command specified to the drone
 * in case of landing or take off request, keeps the control while
 * the landing/take off is not done
 */
void send_command ();


/* ask for stopping sending commands : STOP the thread*/
void stop_sending_commands() {
  vp_os_mutex_lock(&stop_mutex);
  stop_sending = 1;
  vp_os_mutex_unlock(&stop_mutex);
}

int continue_sending() {
  int stop = 0;
  vp_os_mutex_lock(&stop_mutex);
  stop = stop_sending; 
  vp_os_mutex_unlock(&stop_mutex);

  return !stop;
}

DEFINE_THREAD_ROUTINE(th_move_cmd, data)
{
    // no command sent at the beginning
    current_command.commandType = NO_REQUEST;

    // mutex initialisation
    vp_os_mutex_init(&command_mutex);
    vp_os_mutex_init(&sfm_state_mutex);
    vp_os_mutex_init(&stop_mutex);
 
    ardrone_at_set_flat_trim(); 
    usleep(500000);

    while (continue_sending()) {
        send_command();
    }

    // request for stopping
    vp_os_mutex_destroy(&stop_mutex);
    vp_os_mutex_destroy(&command_mutex);
    vp_os_mutex_destroy(&sfm_state_mutex);

    return (0);
}

/*
 * send the command specified to the drone
 * in case of landing or take off request, keeps the control while
 * the landing/take off is not done
 */
#define HOVERING_MODE 0x0
#define CMD_MODE 0x1
#define YAW_AND_HOVERING_MODE 0x2
#define YAW_MODE 0x3
void send_command() {

  Inputs_t lastCommand;
  commandType_t lastType;
  float32_t stabVal=0.001;
  uint32_t flag = 0x0;
 
  get_command (&lastCommand ,&lastType);

  switch (lastType) {

  case TAKEOFF_REQUEST :
    do {
      ardrone_tool_set_ui_pad_start(TAKEOFF_OPTION);
    } while (get_drone_state() != FLYING);
    // drone took off
    lastType = NO_REQUEST;

    break;

  case FLYING_REQUEST : 
    if (lastCommand.roll == 0.0 &&
        lastCommand.pitch == 0.0 &&
        lastCommand.gas == 0.0 &&
        lastCommand.Vyaw == 0.0) { // hovering mode : the drone does not move
        
        stabVal*=-1; // sends alternatively a yaw = 0.01 and yaw = -0.01 to stabilize
        ardrone_at_set_progress_cmd(flag, lastCommand.roll,lastCommand.pitch,
                                lastCommand.gas,stabVal);
        flag = 0x2;
    }

    if(lastCommand.Vyaw != 0.0 || lastCommand.gas != 0.0){ // yaw or gas request
        flag=0x2;
        ardrone_at_set_progress_cmd(flag, lastCommand.roll,lastCommand.pitch,
                                lastCommand.gas,lastCommand.Vyaw);
    } else { // others
        flag=0x1;
        ardrone_at_set_progress_cmd(flag, lastCommand.roll,lastCommand.pitch,
                                lastCommand.gas,lastCommand.Vyaw);
    }

    break;

  case LANDING_REQUEST :
    do {
      ardrone_tool_set_ui_pad_start(LANDING_OPTION);
    } while (get_drone_state() != LANDED);
    // landed
    lastType = NO_REQUEST;

    break;
  
  default : ; // invalid command type or no request
  }

}


/*
 * To refresh the command to send to the drone in sfm
 * newCommand : the flight parameter of the command (yaw, pitch...)
 * type : the command type to land, take...
 * NOTE : this fonction does nothing if the drone is not in safety mode
 */
void set_sfm_command (Inputs_t * newCommand , commandType_t type) {

  vp_os_mutex_lock(&sfm_state_mutex);

  if (isInSFM) {

    vp_os_mutex_lock(&command_mutex);

   if (newCommand != NULL) {
      current_command.command.roll = newCommand->roll;  
      current_command.command.pitch = newCommand->pitch;  
      current_command.command.Vyaw = newCommand->Vyaw;  
      current_command.command.gas = newCommand->gas;
    } else {

      current_command.command.roll = 0.0;  
      current_command.command.pitch = 0.0;  
      current_command.command.Vyaw = 0.0;  
      current_command.command.gas = 0.0;

    }

    current_command.commandType = type;

    vp_os_mutex_unlock(&command_mutex);

  }

  vp_os_mutex_unlock(&sfm_state_mutex);
}

/*
 * To refresh the command to send to the drone
 * newCommand : the flight parameter of the command (yaw, pitch...)
 * type : the command type to land, take...
 * NOTE : this fontion does nothing if the drone is in safety mode
 */
void set_command (Inputs_t * newCommand, commandType_t type) {

  vp_os_mutex_lock(&sfm_state_mutex);

  if (!isInSFM) {

    vp_os_mutex_lock(&command_mutex);

    if (newCommand != NULL) {
      current_command.command.roll = newCommand->roll;  
      current_command.command.pitch = newCommand->pitch;  
      current_command.command.Vyaw = newCommand->Vyaw;  
      current_command.command.gas = newCommand->gas;
      satCmd(&(current_command.command));
    } else {

      current_command.command.roll = 0.0;  
      current_command.command.pitch = 0.0;  
      current_command.command.Vyaw = 0.0;  
      current_command.command.gas = 0.0;

    }
    current_command.commandType = type;

    vp_os_mutex_unlock(&command_mutex);
  }

  vp_os_mutex_unlock(&sfm_state_mutex);
}

/*
 * To get the last command sent to the drone
 * lastCommand : the flight parameter of the last command sent to the drone
 * type : the command type previously sent
 */
void get_command (Inputs_t * lastCommand , commandType_t * type) {

  vp_os_mutex_lock(&command_mutex);

  lastCommand->roll = current_command.command.roll;  
  lastCommand->pitch = current_command.command.pitch;  
  lastCommand->Vyaw = current_command.command.Vyaw;  
  lastCommand->gas = current_command.command.gas;
  *type = current_command.commandType;

  vp_os_mutex_unlock(&command_mutex);


}

/*
 * Enable to switch the drone into nominal mode i.e. the application
 * takes into account the user commands
 */
void switch_to_nominal_mode() {

  if(isInSFM!=0){
      ardrone_at_set_led_animation(STANDARD,1.0,1);
  }
  isInSFM = 0;

}

/*
 * Enable to switch the drone into the safety mode i.e. the application
 * blocks the user commands and takes control
 */
void switch_to_sfm() {
  if(isInSFM==0){
    ardrone_at_set_led_animation(SNAKE_GREEN_RED,1.0,1);
  }
  isInSFM = 1;


}


