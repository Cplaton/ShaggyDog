/* command.c */
#include <signal.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <strings.h>
#include "command.h"
#include "../droneLib/utils_cmd.h"
#include "../droneLib/control_cmd.h"
#include "../com/drone_com.h"

/* allows to periodically send AT commands */
static timer_t tperiod;

/* buffer containing the AT command to send */
static char buffsnd[MAX_AT_CMD_LENGTH];

/* arguments to build the next AT command*/
static dronecommand_t ATcmd;
static dronecommand_t oldATcmd;

/* mutex for protecting the current at command to send */
static pthread_mutex_t cmd_mutex = PTHREAD_MUTEX_INITIALIZER;

/* sequence number in the AT command */
static int seq_num;


void * droneControl (void *stop);
void send_ATcommand (int sig);
void get_command();
int get_seqnum();

/*
 * starts the thread that sends at commands to the drone
 */
pthread_t start_ssm_command_manager(int * stop) {

  pthread_t thid;

  if (pthread_create(&thid,NULL,droneControl,stop) != 0) {
    perror("command thread creation : ");
    exit(1);
  }

  return thid;

}

/*
 * Initialize the thread
 */
void * droneControl (void * stop) {
 
  // configure the signal fired by the timer
  sigset_t mask;
  struct sigaction sig;
 

  sigemptyset(&mask);
  sig.sa_handler = send_ATcommand;
  sig.sa_flags = 0;
  sig.sa_mask = mask;

  // configure the timer (period)
  struct itimerspec tspec;
  tspec.it_interval.tv_sec = 0;
  tspec.it_interval.tv_nsec = ATCOMMAND_SENDING_MS_PERIOD * 1000000;
  tspec.it_value.tv_sec = 0;
  tspec.it_value.tv_nsec = 1000000;

  if (sigaction(SIGALRM, &sig ,NULL) != 0) {
    perror("sigaction : ");
  }

  if (timer_create(CLOCK_REALTIME,NULL,&tperiod) != 0) {
    perror("timer_create : ");
  }

  // arm the timer
  if (timer_settime(tperiod,0,&tspec,NULL) != 0) {
    perror("timer_settime : ");
  }

  printf("control thread initialiazed\n");

  while (*((int *)(stop)) == 0) { // continue while no stop demand
    pause();
  }

  timer_delete(tperiod); // stop timer
  pthread_mutex_destroy(&cmd_mutex);

    return NULL;
}

/*
 * Send the motion command to the drone
 */
void send_ATcommand (int sig) {

  if (sig == SIGALRM) { // new period

    bzero(buffsnd,MAX_AT_CMD_LENGTH);
    get_command();

    switch (oldATcmd.cmd_type) {
      case REGULAR_CMD : // regular command (roll, pitch,...)
        motion_command(buffsnd,get_seqnum(),oldATcmd.roll,oldATcmd.pitch,oldATcmd.gaz,oldATcmd.yaw);
        break;
      case LANDING_CMD : // landing
        landing_command(buffsnd,get_seqnum());
        break;
      case TAKEOFF_CMD : // take off
        takeoff_command(buffsnd,get_seqnum());
        break;
      case HOVER_CMD : // no motion
        motion_command(buffsnd,get_seqnum(),0.0f,0.0f,0.0f,0.0f);
        break;
      default: ;
    }

    ssm_state_sending(buffsnd, (size_t)MAX_AT_CMD_LENGTH);

  }
}

/*
 * Update the AT command to send to the drone
 */
void set_command(dronecommand_t * cmd) {

  pthread_mutex_lock(&cmd_mutex);

  ATcmd.cmd_type = cmd->cmd_type;
  ATcmd.roll = cmd->roll;
  ATcmd.pitch = cmd->pitch;
  ATcmd.gaz = cmd->gaz;
  ATcmd.yaw = cmd->yaw;

  pthread_mutex_unlock(&cmd_mutex);

}

/*
 * Allows to get the last AT command asked to send
 */
void get_command() {
 
  pthread_mutex_lock(&cmd_mutex);

  oldATcmd.cmd_type = ATcmd.cmd_type;
  oldATcmd.roll = ATcmd.roll;
  oldATcmd.pitch = ATcmd.pitch;
  oldATcmd.gaz = ATcmd.gaz;
  oldATcmd.yaw = ATcmd.yaw;

  pthread_mutex_unlock(&cmd_mutex);
}

/* to get the sequence number to set in the AT command */
int get_seqnum() {
  return ++seq_num;
}

/* to update the last sequence number value to set in the AT command */
void set_seqnum(int new_seqnum) {
  seq_num = new_seqnum;
}
