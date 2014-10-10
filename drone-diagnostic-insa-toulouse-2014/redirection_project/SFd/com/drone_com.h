/* drone_com.h */

#ifndef DRONE_COM_H__
#define DRONE_COM_H__

#define NOMINAL_COM_PORT 	5556
#define SSM_COM_PORT 		15001
#define RCV_OTHER_PORT	 	15002
#define RCV_NAVDATA_PORT	15003
#define DRONE_IP		"192.168.1.1"


void getip(char * ip);

/*
 * Initialize all the communication structures with the drone
 */
void init_com();

/* 
 * Send UDP packet to the drone on the NOMINAL_COM_PORT port
 * retuns 0 if the packet has been sent properly, -1 otherwhise
 */
int nominal_state_sending(char * buffer, size_t buflen);

/* 
 * Send UDP packet to the drone on the SSM_COM_PORT port (smart safety mode port)
 * retuns 0 if the packet has been sent properly, -1 otherwhise
 */
int ssm_state_sending(char * buffer, size_t buflen);

/* 
 * Waits for receiving data from the drone (different from navdata)
 * retuns the data received, NULL if issue during the reception
 */
char * recv_otherdata(char * rcvbuff, size_t buflen);

/* 
 * Waits for receiving navdata from the drone
 * retuns the raw navdata (without parsing), NULL if issue during the reception
 */
char * recv_navdata(char * rcvbuff, size_t buflen);

/*
 * Close all the sockets
 */
void close_com();

#endif
