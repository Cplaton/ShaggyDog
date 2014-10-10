/* sm_manager.c */
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include "sm_manager.h"

uint32_t current_state = NOMINAL_STATE; // NOMINAL or SAFETY MODE

void extract_cmd(char * buffer, char * sndBackCmd);
int analyse_packet(char * buff);
void nominal_mode(int sockfd, struct sockaddr * rcv_cmd_addr,
                  struct sockaddr * snd_drone_cmd_addr, struct sockaddr * snd_SF_cmd_addr);
void safety_mode(int sock, struct sockaddr * rcv_cmd_addr,
                  struct sockaddr * snd_drone_cmd_addr, struct sockaddr * snd_SF_addr);

int main( int argc, char *argv[] ) {

    int sockfd;
    struct sockaddr_in serv_addr, SF_snd_addr, th_cmd_addr;
    unsigned int  n , portno;
    socklen_t len;
    char buffer[256];
    current_state = NOMINAL_STATE;

    /* First call to socket() function */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) 
    {
        perror("ERROR opening socket");
        exit(1);
    }
    /* Initialize socket structure */
    memset((char *) &serv_addr,0, sizeof(serv_addr));
    portno = NOMINAL_COM_PORT;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    memset(&th_cmd_addr,0,sizeof(th_cmd_addr));
    portno = 5556;
    th_cmd_addr.sin_family = AF_INET;
    th_cmd_addr.sin_addr.s_addr = INADDR_ANY;
    th_cmd_addr.sin_port = htons(portno);

    memset(&SF_snd_addr,0,sizeof(SF_snd_addr));
    portno = SF_COMMAND_PORT;
    SF_snd_addr.sin_family = AF_INET;
    SF_snd_addr.sin_addr.s_addr = inet_addr("192.168.1.5");
    SF_snd_addr.sin_port = htons(portno);
 

 
    /* Now bind the host address using bind() call.*/
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
                          sizeof(serv_addr)) < 0)
    {
         perror("ERROR on binding");
         exit(1);
    }


    socklen_t len_rcv_cmd_addr = sizeof(serv_addr);
    
    while (1)
    {

         recvfrom(sockfd,buffer,sizeof(buffer),0,(struct sockaddr *)&serv_addr,&len_rcv_cmd_addr);
         
          // send the command to the drone
          //sendto(sockfd,buffer,sizeof(buffer),0,(struct sockaddr *) &th_cmd_addr,(socklen_t) sizeof(th_cmd_addr));



    //    if (current_state == NOMINAL_STATE) {
      //  } else {
      //      safety_mode(sockfd,(struct sockaddr *)&serv_addr,
      //             (struct sockaddr *)&th_cmd_addr,(struct sockaddr *)&SF_snd_addr);
      //  }
 
    }

    return 0;
}
/*
void nominal_mode(int sockfd, struct sockaddr * rcv_cmd_addr, 
                  struct sockaddr * snd_drone_cmd_addr, struct sockaddr * snd_SF_cmd_addr) {

    char buffer[256];
    char sndBackCmd[128];
    uint32_t state;
    char stop = 0;

    socklen_t len_rcv_cmd_addr = sizeof(rcv_cmd_addr);
    socklen_t len_snd_drone_cmd_addr = sizeof(snd_drone_cmd_addr);
    socklen_t len_snd_SF_cmd_addr = sizeof(snd_SF_cmd_addr);


    do {
        memset(buffer,0,sizeof(buffer));
        recvfrom(sockfd,buffer,sizeof(buffer),0,rcv_cmd_addr,&len_rcv_cmd_addr);
         
        //state = analyse_packet(buffer);

        //if (state == -1 || state == current_state) { // not demanding a state switch

          //printf("send cmd to drone : %s\n",buffer);

          // send the command to the drone
          //sendto(sockfd,buffer,sizeof(buffer),0,snd_drone_cmd_addr,len_snd_drone_cmd_addr);

          //extract_cmd(buffer,sndBackCmd);

          // send back the command parameters to smartfox
          //sendto(sockfd,sndBackCmd,strlen(sndBackCmd),0,snd_SF_cmd_addr,len_snd_SF_cmd_addr);

        //} else {
        //    current_state = state;
        //    stop = 1;
        //}
    } while (!stop);

}

void safety_mode(int sock,struct sockaddr * rcv_cmd_addr, 
                  struct sockaddr * snd_drone_cmd_addr, struct sockaddr * snd_SF_addr) {

    char buffer[256];
    char sndBackCmd[128];
    uint32_t state;
    char stop = 0;

    socklen_t len_rcv_cmd_addr = sizeof(rcv_cmd_addr);
    socklen_t len_snd_drone_cmd_addr = sizeof(snd_drone_cmd_addr);
    socklen_t len_snd_SF_addr = sizeof(snd_SF_addr);


    memset(buffer,0,sizeof(buffer));
    sprintf(buffer,"SF*CONFIG,2");

    printf("SAFETY MODE\n");   
 
    // send the command to the drone
    sendto(sock,buffer,sizeof(buffer),0,snd_SF_addr,len_snd_SF_addr);

}
*/

/* 
  analyse a packed received to detect a configuration demand
  returns -1 if it is not a config demand
          the config command otherwise
*//*
int analyse_packet(char * buff) {

  int config_cmd = -1;

  if (strstr(buff,HEADER_CONFIG) != NULL) {
      printf("%s\n",buff);
      sscanf(buff+10,"%d",&config_cmd);
  }

  return config_cmd;
}

void extract_cmd(char * buffer, char * sndBackCmd) {

    float trash;
    command_t cmdToSdnBack;

    if (strstr(buffer,HEADER_AT_CMD) != NULL) {
        sscanf(buffer+8,"%f,%f,%f,%f,%f,%f<LF>",&trash,&trash,&(cmdToSdnBack.roll),
        	&(cmdToSdnBack.pitch),&(cmdToSdnBack.gas),&(cmdToSdnBack.Vyaw));
        sprintf(sndBackCmd,"SF*CMD,%f,%f,%f,%f",cmdToSdnBack.roll,cmdToSdnBack.pitch,cmdToSdnBack.gas,cmdToSdnBack.Vyaw);
    }
}*/
