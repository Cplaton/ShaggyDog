/* command.h */
#include <pthread.h>

#ifndef COMMAND_H__
#define COMMAND_H__

#define ATCOMMAND_SENDING_MS_PERIOD 30

/* command type : regular, landing, take off or no motion */
typedef enum _cmd_type_t {
  REGULAR_CMD, LANDING_CMD, TAKEOFF_CMD, HOVER_CMD
} cmd_type_t;

/* arguments to set inside the AT command */
typedef struct _dronecommand_t {
  cmd_type_t cmd_type;
  float roll;
  float pitch;
  float gaz;
  float yaw;
} dronecommand_t;


/* starts the thread that sends at commands to the drone */
pthread_t start_ssm_command_manager(int * stop);

/* to update the command to send */
void set_command(dronecommand_t * cmd);

/* to update thelast sequence number value to set in the AT command */
void set_seqnum (int new_seqnum);

#endif
