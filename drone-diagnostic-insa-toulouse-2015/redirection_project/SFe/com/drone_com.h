/* drone_com.h */

#ifndef DRONE_COM_H__
#define DRONE_COM_H__

// port où sont redirigés les commandes de l'appli user | SFD/extapp -> SFE |
#define NOMINAL_COM_PORT 	15000
 
// port sur lequel sont envoyées les commandes de Smarfox en safety mode |SFD -> SFE|
#define SSM_COM_PORT 		15001

// port distant smartfox surlequel sont envoyées les commandes reçues |SFE -> SFD|
#define SF_COMMAND_PORT 	15002

// port distant où sont envoyés les navdata |SFE -> SFD|
#define NAVDATA_SFE_SND_PORT	15003

// port de réception des navdata émis par le drone et redirigés
#define NAVDATA_SFE_RCV_PORT	15004

// port normal où sont envoyés les AT commands au drone
#define COMMAND_PORT 		5556

// port où sont normalement envoyés les navdata
#define NAVDATA_NOMINAL_PORT	5554

#define LOCALHOSTADDR 		"127.0.0.1"

#define SF_CONFIG_HEADER_PACKET	"SF*CONFIG"

#define AT_CMD_HEADER_PACKET 	"AT*"



void init_com();

/*
 * For sending packet to the SMARTFOX DEBEDDED (different from navdata, command for example)
 * returns -1 if a problem occured during the packet sending , 0 otherwise
 */
int send_cmd_to_sfe(char * buf, size_t buflen);

/*
 * For sending navdata packet to SMARTFOX DEBEDDED and the external app
 * returns -1 if a problem occured during the packet sending , 0 otherwise
 */
int send_navdata(char * buf, size_t buflen);


/*
 * For sending data to the drone (tipically commands redirected) 
 * returns -1 if a problem occured during the packet sending , 0 otherwise
 */
int send_data_to_drone(char * buf, size_t buflen);

/*
 * For receiving redirected navdata from the drone 
 * returns -1 if a problem occured during the packet reception , 0 otherwise
 */
int receive_navdata(char * buf, size_t buflen) ;

/*
 * For receiving data from sfd in nominal mode 
 * returns -1 if a problem occured during the packet reception , 0 otherwise
 */
int receive_sfddata_nominal_mode(char * buf, size_t buflen);

/*
 * For receiving data from sfd in ssm 
 * returns -1 if a problem occured during the packet reception , 0 otherwise
 */
int receive_sfddata_ssm(char * buf, size_t buflen); 

#endif
