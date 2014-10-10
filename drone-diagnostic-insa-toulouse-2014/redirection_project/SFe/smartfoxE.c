/* smartfoxE.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "smartfoxE.h"
#include "com/drone_com.h"


static int current_state = NOMINAL_STATE; // NOMINAL or SAFETY MODE
static int sequence_number = 0;		       // last sequence number received

void extract_cmd(char * buffer, char * sndBackCmd);
int extract_seq_num(char * buffer);
int analyse_packet(char * buff);
void nominal_mode();
void safety_mode();


int main( int argc, char *argv[] ) {

    // test
    init_com();

    current_state = NOMINAL_STATE;

    
    while (1)
    {
        if (current_state == NOMINAL_STATE) {
            nominal_mode();
        } else if (current_state == SAFETY_MODE_STATE) {
            safety_mode();
        }
 
    }

    return 0;
}

void nominal_mode() {

    char buffer[MAX_PACKET_LENGTH];
    char sndBackCmd[128];
    int state;
    char stop = 0;
    int seq_num;

    printf("NOMINAL STATE\n");

    do {

        memset(buffer,0,MAX_PACKET_LENGTH);

 	receive_sfddata_nominal_mode(buffer,MAX_PACKET_LENGTH); 
        printf("buff : %s \n",buffer); 
        state = analyse_packet(buffer);

        if (state == -1 || state == current_state) { // not demanding a state switch

          // send the command to the drone
          send_data_to_drone(buffer,MAX_PACKET_LENGTH);

          extract_cmd(buffer,sndBackCmd);

          if ((seq_num = extract_seq_num(buffer)) != -1) {
            sequence_number = seq_num;
          }

          // send back the command parameters to smartfox
          send_cmd_to_sfe(sndBackCmd,128);

        } else {
            current_state = state;
            stop = 1;
        }
    } while (!stop);

}

void safety_mode() {

    char buffer[MAX_PACKET_LENGTH];
    char sndBackCmd[128];
    int state;
    char stop = 0;

    // send the last sequence number received
    memset(buffer,0,MAX_PACKET_LENGTH);
    sprintf(buffer,"SF*CONFIG,2,%d",sequence_number);
    send_cmd_to_sfe(buffer,MAX_PACKET_LENGTH);
    printf("seq num sent : %d \n", sequence_number);

    printf("SAFETY MODE\n");

    do {

        memset(buffer,0,MAX_PACKET_LENGTH);

        // wait for receiving command from sfd
        receive_sfddata_ssm(buffer,MAX_PACKET_LENGTH);        
 
        state = analyse_packet(buffer);

        if (state == -1 || state == current_state) { // not demanding a state switch

          // send the command to the drone
          send_data_to_drone(buffer,MAX_PACKET_LENGTH);
                    
          extract_cmd(buffer,sndBackCmd);

        } else {
            current_state = state;
            stop = 1;
        }
    } while (!stop);
  
}


/* 
  analyse a packed received to detect a configuration demand
  returns -1 if it is not a config demand
          the config command otherwise (to switch on nominal / safety mode state)
*/
int analyse_packet(char * buff) {

  int config_cmd = -1;

  printf("%s\n",buff);
  if (strstr(buff,HEADER_CONFIG) != NULL) {

    if (buff[10] == (48 + NOMINAL_STATE)) { // nominal state demand
      config_cmd = current_state == NOMINAL_STATE ? config_cmd : NOMINAL_STATE; 
    } else if (buff[10] == (48 + SAFETY_MODE_STATE)) { // ssm state demand
      config_cmd = current_state == SAFETY_MODE_STATE ? config_cmd : SAFETY_MODE_STATE;
    } // else : ip demand or bad config packet

  } // else : other packet

  return config_cmd;
}

/* extract the argument of a */
void extract_cmd(char * buffer, char * sndBackCmd) {

    float trash;
    int_or_float roll, pitch,gaz,vyaw;    

    if (strstr(buffer,HEADER_AT_PCMD_CMD) != NULL) {
        sscanf(buffer+8,"%f,%f,%d,%d,%d,%d",&trash,&trash,&(roll.i),
        	&(pitch.i),&(gaz.i),&(vyaw.i));
        sprintf(sndBackCmd,"SF*CMD,%f,%f,%f,%f",roll.f,pitch.f,gaz.f,vyaw.f);
    }
}

/* 
 * if the buffer is an AT command : extract its sequence number
 * returns the sequence number extracted or -1 if the packet does not contain sequence numbers
 */
int extract_seq_num(char * buffer) {

  char trash[MAX_PACKET_LENGTH];
  int seqnum = -1;

  if (strstr(buffer,HEADER_AT_CMD) != NULL) {
    sscanf(buffer,"%[^=]=%d,%s",trash,&seqnum,trash);
  }

  return seqnum;
}
