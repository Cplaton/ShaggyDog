/* navdata.h */

#ifndef NAVDATA_H__
#define NAVDATA_H__
/* 
 * To get the command that allows to disable navdata demo
 * and get all navdata (with a 5ms period)
 * The sequence number is set to 1
 * AT_cmd : the strings filled with the AT command
 * The length of the AT_cmd parameter must be longer than 
 * MAX_AT_CMD_LENGTH defined in utils_cmd.h
 */
char * disable_navdata_demo_cmd(char * AT_cmd);

#endif
