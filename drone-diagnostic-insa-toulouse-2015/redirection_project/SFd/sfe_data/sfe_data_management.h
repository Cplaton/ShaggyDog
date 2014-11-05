/* sfe_data_management.h */
#include <pthread.h>

#ifndef SFE_DATA_MANAGEMENT_H__
#define SFE_DATA_MANAGEMENT__

#define SF_CONFIG_NOMINAL_MODE_DEMAND 	"SF*CONFIG,0"
#define SF_CONFIG_SSM_DEMAND 		"SF*CONFIG,1"
#define SF_CONFIG_SEQ_NUM_REFRESH 	"SF*CONFIG,2,"
#define SF_CONFIG_IP_DETAIL 		"SF*CONFIG,3"
#define SFE_COMMAND 			"SF*CMD,"

/* the commands sent by the external application */
typedef struct _extapp_cmd_t {
  float roll;
  float pitch;
  float gaz;
  float yaw;
} extapp_cmd_t;

/* 
 * Launches the thread handling the data sent by the drone
 * (except navdata) like the commands sent by the external
 * application
 */
pthread_t init_sfe_data_manager();

/* Stops the sfe data manager */
void stop_sfe_data_manager();

/* to get the last command sent by the external app to the drone */
void get_extapp_cmd(extapp_cmd_t * ext_cmd);

/* 
 * Ask for switching the drone on smart safety mode 
 * returns 0 if swich ok, -1 otherwise
 */
int switch_on_ssm();

/* 
 * Ask for switching the drone on smart safety mode 
 * returns 0 if swich ok, -1 otherwise
 */
int switch_on_nominal_mode();

/*
 * Send a basic packet to allow sfe to know about the sfd ip
 */
void send_sfd_ip();

#endif
