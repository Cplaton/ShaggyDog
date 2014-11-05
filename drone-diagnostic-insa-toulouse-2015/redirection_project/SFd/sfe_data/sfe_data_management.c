/* sfe_data_management.c */
#include <strings.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "sfe_data_management.h"
#include "../com/drone_com.h"
#include "../command/command.h"


static int stop_ssm_management = -1;
static int stop_th_sfe_data_manager = -1;
static extapp_cmd_t extapp_cmd; // last command sent by the external app to the drone
static pthread_t th_snd_cmd;

static pthread_mutex_t extcmd_mutex = PTHREAD_MUTEX_INITIALIZER;

void analyse_data(char * buffer);

/* refresh the structure containing the last command sent by the external app to the drone */
void set_extapp_cmd(float roll, float pitch, float gaz, float yaw);

void * process_sfe_data(void * stop);


/* 
 * Launches the thread handling the data sent by the drone
 * (except navdata) like the commands sent by the external
 * application
 */
pthread_t init_sfe_data_manager() {

  pthread_t thid;

  if (pthread_create(&thid,NULL,process_sfe_data,&stop_th_sfe_data_manager) != 0) {
    perror("sfe data manager thread create : ");
    exit(1);
  }
  
  return thid;
}

/* Stops the sfe data manager */
void stop_sfe_data_manager() {
  stop_th_sfe_data_manager = 1;
}

void * process_sfe_data(void * stop) {

  int buflen = 64;
  char buffer[buflen];
  bzero(buffer,buflen);

  set_extapp_cmd(0.0f,0.0f,0.0f,0.0f);

  while (*((int *)stop) != 1) {

    recv_otherdata(buffer, (size_t) buflen);

    analyse_data(buffer);

  }
  ////////////////
  printf("stop process sfe_data\n");
  ////////////////

  if (stop_ssm_management >= 0) { // ssm management has been launched at least once
    stop_ssm_management = 1;
    pthread_join(th_snd_cmd,NULL);
  }


  return NULL;
}


void analyse_data(char * buffer) {

  int seq_num;
  float roll, pitch, gaz, yaw;

  if (strstr(buffer,SF_CONFIG_SEQ_NUM_REFRESH) != NULL) { // sequence number received
    // extract the sequence number
    sscanf(buffer + strlen(SF_CONFIG_SEQ_NUM_REFRESH),"%d",&seq_num);
    set_seqnum(seq_num);
    stop_ssm_management = 0;
    th_snd_cmd = start_ssm_command_manager(&stop_ssm_management);
    printf("seq num received : %d - starting control thread\n",seq_num);

  } else if (strstr(buffer,SFE_COMMAND) != NULL) { // command from ext app received
    sscanf(buffer + strlen(SFE_COMMAND),"%d,%d,%d,%d",(int *)&roll,(int *)&pitch,(int *)&gaz,(int *)&yaw);
    set_extapp_cmd(roll,pitch,gaz,yaw);    
  }

}

/* refresh the structure containing the last command sent by the external app to the drone */
void set_extapp_cmd(float roll, float pitch, float gaz, float yaw) {

  pthread_mutex_lock(&extcmd_mutex);

  extapp_cmd.roll = roll;
  extapp_cmd.pitch = pitch;
  extapp_cmd.gaz = gaz;
  extapp_cmd.yaw = yaw;

  pthread_mutex_unlock(&extcmd_mutex);
}

/* to get the last command sent by the external app to the drone */
void get_extapp_cmd(extapp_cmd_t * ext_cmd) {

  pthread_mutex_lock(&extcmd_mutex);

  ext_cmd->roll = extapp_cmd.roll;
  ext_cmd->pitch = extapp_cmd.pitch;
  ext_cmd->gaz = extapp_cmd.gaz;
  ext_cmd->yaw = extapp_cmd.yaw;

  pthread_mutex_unlock(&extcmd_mutex);
}

/* 
 * Ask for switching the drone on smart safety mode 
 * returns 0 if swich ok, -1 otherwise
 */
int switch_on_ssm() {

  int buflen = 64;
  char buffer[buflen];
  bzero(buffer,buflen);
  sprintf(buffer,"%s",SF_CONFIG_SSM_DEMAND);

  if (nominal_state_sending(buffer, (size_t) buflen) !=0 ) {
    fprintf(stderr,"Failed to switch on smart safety mode\n");
    return -1;
  }

  return 0;
}


/* 
 * Ask for switching the drone on smart safety mode 
 * returns 0 if swich ok, -1 otherwise
 */
int switch_on_nominal_mode() {

  int buflen = 64;
  char buffer[buflen];
  bzero(buffer,buflen);
  sprintf(buffer,"%s",SF_CONFIG_NOMINAL_MODE_DEMAND); 

  if (ssm_state_sending(buffer, (size_t) buflen) !=0 ) {
    fprintf(stderr,"Failed to switch on nominal mode\n");
    return -1;
  }

  stop_ssm_management = 1; // stops the thread handling the at command sending


  return 0;


}

/*
 * Send a basic packet to allow sfe to know about the sfd ip
 */
void send_sfd_ip() {

  char buff[16];
  bzero(buff,16);
  snprintf(buff,16,"%s",SF_CONFIG_IP_DETAIL);

  if (nominal_state_sending(buff,16) != 0) {
    fprintf(stderr,"fatal error : impossible to send the local sfd IP to sfe : application stopped\n");
    exit(1);
  }
}
