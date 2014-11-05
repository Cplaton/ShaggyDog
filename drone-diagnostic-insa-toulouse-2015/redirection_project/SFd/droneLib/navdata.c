/* navdata.c */
#include <stdio.h>
#include <strings.h>
#include "navdata.h"
#include "utils_cmd.h"

/* 
 * To get the command that allows to disable navdata demo
 * and get all navdata (with a 5ms period)
 * The sequence number is set to 1
 * AT_cmd : the strings filled with the AT command
 * The length of the AT_cmd parameter must be longer than 
 * MAX_AT_CMD_LENGTH defined in utils_cmd.h
 */
char * disable_navdata_demo_cmd(char * AT_cmd) {

  bzero(AT_cmd,MAX_AT_CMD_LENGTH);
  sprintf(AT_cmd,"AT*CONFIG=1,\"%s\",\"%s\"%c","general:navdata_demo","FALSE",LF_CHAR);

  return AT_cmd;

}
