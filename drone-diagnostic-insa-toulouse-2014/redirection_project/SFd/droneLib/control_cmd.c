/* control_cmd.c */
#include <strings.h>
#include <stdio.h>
#include "control_cmd.h"
#include "utils_cmd.h"

/*
 * Build an AT command message to control the drone motion
 * AT_cmd     : the string that will contain the AT command expected
 * seq_nb     : the sequence number of the packet 
 * other args : motion parameters (see SDK doc) - must be between -1 and 1 (if not : set to 0)
 */
void motion_command (char * AT_cmd, int seq_nb, float roll, float pitch, float gaz, float yaw) {

    // check params are ok + convertion float to int
    int roll_int = MIN_MOTION_ARG <= roll && roll <= MAX_MOTION_ARG ? 
			*((int *) &roll) : NO_MOTION_ARG ;
    int pitch_int = MIN_MOTION_ARG <= pitch && pitch <= MAX_MOTION_ARG ? 
			*((int *) &pitch) : NO_MOTION_ARG ; 
    int gaz_int = MIN_MOTION_ARG <= gaz && gaz <= MAX_MOTION_ARG ? 
			*((int *) &gaz) : NO_MOTION_ARG ; 
    int yaw_int =  MIN_MOTION_ARG <= yaw && yaw <= MAX_MOTION_ARG ? 
			*((int *) &yaw) : NO_MOTION_ARG ; 

    unsigned int activationFlag = 1;

    bzero(AT_cmd,MAX_AT_CMD_LENGTH);

    sprintf(AT_cmd,"AT*PCMD=%d,%d,%d,%d,%d,%d%c",seq_nb,activationFlag,roll_int,pitch_int,gaz_int,yaw_int,LF_CHAR);

}

/*
 * Build an AT command message to make the drone lands
 * AT_cmd     : the string that will contain the AT command expected
 * seq_nb     : the sequence number of the packet 
 */
void landing_command (char * AT_cmd, int seq_nb) {

    unsigned int param = 0;
    param |= (0x1 << 18);
    param |= (0x1 << 20);
    param |= (0x1 << 22);
    param |= (0x1 << 24);
    param |= (0x1 << 28);
    // bit 9 = 0 to land

    bzero(AT_cmd,MAX_AT_CMD_LENGTH);

    sprintf(AT_cmd,"AT*REF=%d,%d%c",seq_nb,param,LF_CHAR);

}

/*
 * Build an AT command message to make the drone takes off
 * AT_cmd     : the string that will contain the AT command expected
 * seq_nb     : the sequence number of the packet 
 */
void takeoff_command (char * AT_cmd, int seq_nb) {

    unsigned int param = 0;
    param |= (0x1 << 18);
    param |= (0x1 << 20);
    param |= (0x1 << 22);
    param |= (0x1 << 24);
    param |= (0x1 << 28);
    param |= (0x1 << 9); // bit 9 = 1 to take off

    bzero(AT_cmd,MAX_AT_CMD_LENGTH);

    sprintf(AT_cmd,"AT*REF=%d,%d%c",seq_nb,param,LF_CHAR);

}
