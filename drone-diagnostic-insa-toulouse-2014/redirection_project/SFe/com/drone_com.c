/* drone_com.c */
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include "drone_com.h"

static struct sockaddr_in nominal_mode_rcv_addr, // nominal mode reception address
			  ssm_rcv_addr,		 // ssm reception address
			  navdata_snd_addr, 	 // navdata sending address (used both to send to SF and to the external app
                          navdata_recv_addr,	 // navdata receiving address (to receive navdata packet redirected)
			  cmd_snd_addr,		 // for sending command sent by the external app and also the last seq num
 			  snd_drone_addr;	 // for sending data to the drone

static int socksnd,  			// socket for sending data
	   sockrcv_navdata, 		// socket for receiving redirected navdata (from drone)
	   sockrcv_nominal_mode,	// socket for receiving data in nominal mode
	   sockrcv_ssm;			// socket for receiving data in smart safety mode

static uint32_t sfd_ip = 0;		// sfd ip address
static uint32_t ext_app_ip = 0;		// external application address

void set_sfd_ip(struct sockaddr_in * addr, char * buf, size_t buflen);
void set_external_app_ip(struct sockaddr_in * addr, char * buf, size_t buflen);

void init_com() {

  // nominal mode reception
  memset(&nominal_mode_rcv_addr,0,sizeof(nominal_mode_rcv_addr));
  nominal_mode_rcv_addr.sin_family = AF_INET;
  nominal_mode_rcv_addr.sin_addr.s_addr = INADDR_ANY;
  nominal_mode_rcv_addr.sin_port = htons(NOMINAL_COM_PORT);

  // ssm reception
  memset(&ssm_rcv_addr,0,sizeof(ssm_rcv_addr));
  ssm_rcv_addr.sin_family = AF_INET;
  ssm_rcv_addr.sin_addr.s_addr = INADDR_ANY;
  ssm_rcv_addr.sin_port = htons(SSM_COM_PORT);

  // navdata sending
  memset(&navdata_snd_addr,0,sizeof(navdata_snd_addr));
  navdata_snd_addr.sin_family = AF_INET;
 
  // navdata receiving
  memset(&navdata_recv_addr,0,sizeof(navdata_recv_addr));
  navdata_recv_addr.sin_family = AF_INET;
  navdata_recv_addr.sin_addr.s_addr = inet_addr(LOCALHOSTADDR);
  navdata_recv_addr.sin_port = htons(NAVDATA_SFE_RCV_PORT);

  
  // command sending (to SFD)
  memset(&cmd_snd_addr,0,sizeof(cmd_snd_addr));
  cmd_snd_addr.sin_family = AF_INET;
  cmd_snd_addr.sin_port = htons(SF_COMMAND_PORT);

  // to send data to the drone
  memset(&snd_drone_addr,0,sizeof(snd_drone_addr));
  snd_drone_addr.sin_family = AF_INET;
  snd_drone_addr.sin_addr.s_addr = inet_addr(LOCALHOSTADDR);
  //snd_drone_addr.sin_addr.s_addr = INADDR_ANY;
  snd_drone_addr.sin_port = htons(COMMAND_PORT);


  /* SOCKET CREATE */  
  if ((socksnd = socket(AF_INET,SOCK_DGRAM,0)) < 0) {
    perror("send socket create : ");
    exit(1);
  }

  if ((sockrcv_navdata = socket(AF_INET,SOCK_DGRAM,0)) < 0) {
    perror("navdata receive socket create : ");
    exit(1);
  }

  if ((sockrcv_nominal_mode = socket(AF_INET,SOCK_DGRAM,0)) < 0) {
    perror("receive nominal mode socket create : ");
    exit(1);
  }

  if ((sockrcv_ssm = socket(AF_INET,SOCK_DGRAM,0)) < 0) {
    perror("receive ssm socket create : ");
    exit(1);
  }

  /* SOCKET BINDING */
  if (bind(sockrcv_navdata,(struct sockaddr *)&navdata_recv_addr,
           sizeof(navdata_recv_addr)) < 0) {
    perror("bind navdata receive : ");
    exit(1);
  }

  if (bind(sockrcv_nominal_mode,(struct sockaddr *)&nominal_mode_rcv_addr,
           sizeof(nominal_mode_rcv_addr)) < 0) {
    perror("bind nominal mode receive : ");
    exit(1);
  }

  if (bind(sockrcv_ssm,(struct sockaddr *)&ssm_rcv_addr,
           sizeof(ssm_rcv_addr)) < 0) {
    perror("bind ssm receive : ");
    exit(1);
  }

}

/*
 * For sending packet to the SMARTFOX DEBEDDED (different from navdata, command for example)
 * returns -1 if a problem occured during the packet sending , 0 otherwise
 */
int send_cmd_to_sfe(char * buf, size_t buflen) {

  if (sendto(socksnd,buf,buflen,0,(struct sockaddr *)&cmd_snd_addr,sizeof(cmd_snd_addr)) < 0) {
    perror("sending data to sfd : ");
    return -1;
  }
  return 0;
}

/*
 * For sending navdata packet to SMARTFOX DEBEDDED and the external app
 * returns -1 if a problem occured during the packet sending , 0 otherwise
 */
int send_navdata(char * buf, size_t buflen) {

  // send navdata to SFD
  navdata_snd_addr.sin_port = htons(NAVDATA_SFE_SND_PORT);
  navdata_snd_addr.sin_addr.s_addr = sfd_ip;
  

  if (sendto(socksnd,buf,buflen,0,(struct sockaddr *)&navdata_snd_addr,sizeof(navdata_snd_addr)) < 0) {
    perror("sending navdata to sfd : ");
    return -1;
  }

  // send navdata to the external app
  navdata_snd_addr.sin_port = htons(NAVDATA_NOMINAL_PORT);
  navdata_snd_addr.sin_addr.s_addr = ext_app_ip;

  if (sendto(socksnd,buf,buflen,0,(struct sockaddr *)&navdata_snd_addr,sizeof(navdata_snd_addr)) < 0) {
    perror("sending navdata to the external app : ");
    return -1;
  }


  return 0;

}

/*
 * For sending data to the drone (tipically commands redirected) 
 * returns -1 if a problem occured during the packet sending , 0 otherwise
 */
int send_data_to_drone(char * buf, size_t buflen) {

  if (sendto(sockrcv_nominal_mode,buf,buflen,0,(struct sockaddr *)&snd_drone_addr,sizeof(snd_drone_addr)) < 0) {
    perror("sending data to the drone : ");
    return -1;
  }

  return 0;

}

/*
 * For receiving redirected navdata from the drone 
 * returns -1 if a problem occured during the packet reception , 0 otherwise
 */
int receive_navdata(char * buf, size_t buflen) {

  struct sockaddr_in rcvr;
  socklen_t len = sizeof(rcvr);

  if (recvfrom(sockrcv_navdata,buf,buflen,0,(struct sockaddr *)&rcvr,&len) < 0) {
    perror("receiving redirected navdata : ");
    return -1;
  }

  if (ext_app_ip == 0) { // external app ip unknow
    set_external_app_ip(&rcvr,buf,buflen);
  }

  return 0;
}

/*
 * For receiving data from sfd in nominal mode 
 * returns -1 if a problem occured during the packet reception , 0 otherwise
 */
int receive_sfddata_nominal_mode(char * buf, size_t buflen) {

  struct sockaddr_in rcvr;
  socklen_t len = sizeof(rcvr);

  if (recvfrom(sockrcv_nominal_mode,buf,buflen,0,(struct sockaddr *)&rcvr,&len) < 0) {
    perror("receiving data from sfd in nominal mode : ");
    return -1;
  }

  if (sfd_ip == 0) { // sfd ip unknown
   set_sfd_ip(&rcvr,buf,buflen);
  }

  return 0;
}

/*
 * For receiving data from sfd in ssm 
 * returns -1 if a problem occured during the packet reception , 0 otherwise
 */
int receive_sfddata_ssm(char * buf, size_t buflen) {

  struct sockaddr_in rcvr;
  socklen_t len = sizeof(rcvr);

  if (recvfrom(sockrcv_ssm,buf,buflen,0,(struct sockaddr *)&rcvr,&len) < 0) {
    perror("receiving data from sfd in smart safety mode : ");
    return -1;
  }

  return 0;
}


/* in case of SF*CONFIG packet reception : update the sfe ip */
void set_sfd_ip(struct sockaddr_in * addr, char * buf, size_t buflen) {

  if (strstr(buf,SF_CONFIG_HEADER_PACKET) != NULL) { // it does be a config message from SFd
    sfd_ip = addr->sin_addr.s_addr;
    cmd_snd_addr.sin_addr.s_addr = sfd_ip;
    printf("sfe ip set : %x\n",sfd_ip);
  }

}

/* in case of AT command received on the nominal mode port : udate the external app ip */
void set_external_app_ip(struct sockaddr_in * addr, char * buf, size_t buflen) {

  if (strstr(buf,AT_CMD_HEADER_PACKET) != NULL) { // it does be a config message from SFd
    ext_app_ip = addr->sin_addr.s_addr;  
    printf("external app ip set \n");
  }

}
