/* drone_com.c */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <strings.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include "drone_com.h"

/* all address for communication with the drone */
static struct sockaddr_in nom_sending_addr;
static struct sockaddr_in ssm_sending_addr;
static struct sockaddr_in other_rcv_addr;
static struct sockaddr_in rcv_navdata_addr;


/* sockets : 2 for receiving, 1 for sending */
static int sockrcv,
           sockrcvnavdata,
           socksnd;


void getip(char * ip) {

  struct sockaddr_in addr;
  socklen_t len = sizeof(addr);
  getsockname(sockrcv,(struct sockaddr *)&addr,&len);
  
  sprintf(ip,"%s",inet_ntoa(addr.sin_addr));
}

/*
 * Initialize all the communication structures with the drone
 */
void init_com() {

  struct ifreq ifr;
  uint32_t local_ip = 0;

  if ((socksnd = socket(AF_INET,SOCK_DGRAM,0)) < 0) {
    perror("socket send create : ");
    exit(1);
  }

  if ((sockrcv = socket(AF_INET,SOCK_DGRAM,0)) < 0) {
    perror("socket receive create : ");
    exit(1);
  }

  
  if ((sockrcvnavdata = socket(AF_INET,SOCK_DGRAM,0)) < 0) {
    perror("socket receive navdata create : ");
    exit(1);
  }

  ifr.ifr_addr.sa_family = AF_INET;
  strncpy(ifr.ifr_name,"wlan0",IFNAMSIZ -1);

  if (ioctl(sockrcv,SIOCGIFADDR, &ifr) < 0) {
    perror("fatal error : request to know local ip on wlan0 : ");
    exit(1);
  }

  local_ip = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr;

  /* nominal comminucation initialization */
  bzero(&nom_sending_addr,sizeof(nom_sending_addr));
  nom_sending_addr.sin_family = AF_INET;
  nom_sending_addr.sin_addr.s_addr = inet_addr(DRONE_IP);
  nom_sending_addr.sin_port=htons(NOMINAL_COM_PORT);

  /* ssm comminucation initialization */
  bzero(&ssm_sending_addr,sizeof(ssm_sending_addr));
  ssm_sending_addr.sin_family = AF_INET;
  ssm_sending_addr.sin_addr.s_addr = inet_addr(DRONE_IP);
  ssm_sending_addr.sin_port=htons(SSM_COM_PORT);

  /* receiving communication (different from navdata) initialization */
  bzero(&other_rcv_addr,sizeof(other_rcv_addr));
  other_rcv_addr.sin_family = AF_INET;
  other_rcv_addr.sin_addr.s_addr = local_ip;
  other_rcv_addr.sin_port=htons(RCV_OTHER_PORT);

  /* navdata receiving communication initialization */
  bzero(&rcv_navdata_addr,sizeof(rcv_navdata_addr));
  rcv_navdata_addr.sin_family = AF_INET;
  rcv_navdata_addr.sin_addr.s_addr = local_ip;
  rcv_navdata_addr.sin_port=htons(RCV_NAVDATA_PORT);

  if (bind(sockrcv,(struct sockaddr *)&other_rcv_addr,sizeof(other_rcv_addr)) < 0) {
    perror("bind socket recv normal data : ");
    exit(1);
  }

  if (bind(sockrcvnavdata,(struct sockaddr *)&rcv_navdata_addr,sizeof(rcv_navdata_addr)) < 0) {
    perror("bind socket recv navdata : ");
    exit(1);
  }
  
}

/* 
 * Send UDP packet to the drone on the NOMINAL_COM_PORT port
 * retuns 0 if the packet has been sent properly, -1 otherwhise
 */
int nominal_state_sending(char * buffer, size_t buflen) {

  if (sendto(socksnd, buffer, buflen, 0, (struct sockaddr *)&nom_sending_addr, 
             sizeof(nom_sending_addr)) < 0) {
    perror("sendto nominal mode : ");
    return -1;
  }
  return 0;
}

/* 
 * Send UDP packet to the drone on the SSM_COM_PORT port (smart safety mode port)
 * retuns 0 if the packet has been sent properly, -1 otherwhise
 */
int ssm_state_sending(char * buffer, size_t buflen) {

  if (sendto(socksnd, buffer, buflen, 0, (struct sockaddr *)&ssm_sending_addr, 
             sizeof(ssm_sending_addr)) < 0) {
    perror("sendto ssm mode : ");
    return -1;
  }
  
  return 0;
}

/* 
 * Waits for receiving data from the drone (different from navdata)
 * retuns the data received, NULL if issue during the reception
 */
char * recv_otherdata(char * rcvbuff, size_t buflen) {


  socklen_t len = sizeof(other_rcv_addr);

  bzero(rcvbuff,buflen);

  if (recvfrom(sockrcv, rcvbuff, buflen, 0, 
		(struct sockaddr *)&other_rcv_addr,&len) < 0) {
    perror("receive data (different from navdata) from drone : ");
    return NULL;
  }

  return rcvbuff;

}

/* 
 * Waits for receiving navdata from the drone
 * retuns the raw navdata (without parsing), NULL if issue during the reception
 */
char * recv_navdata(char * rcvbuff, size_t buflen) {


  socklen_t len = sizeof(rcv_navdata_addr);

  bzero(rcvbuff,buflen);

  if (recvfrom(sockrcvnavdata, rcvbuff, buflen, 0, 
		(struct sockaddr *)&rcv_navdata_addr,&len) < 0) {
    perror("receive navdata from drone : ");
    return NULL;
  }

  return rcvbuff;

}

/*
 * Close all the sockets
 */
void close_com() {

  if (close(sockrcv) < 0) {
    perror("close socket receive: ");
  }

  if (close(sockrcvnavdata) < 0) {
    perror("close socket navdata: ");
  }

  if (close(socksnd) < 0) {
    perror("close socket send : ");
  }

}

