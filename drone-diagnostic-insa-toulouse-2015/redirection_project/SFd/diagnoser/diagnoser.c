/* diagnoser.c */
#include <stdlib.h>
#include <stdio.h>
#include "diagnoser.h"
#include "../sfe_data/sfe_data_management.h"
#include "../command/command.h"

void * diagnostic (void * arg);

pthread_t start_diagnoser() {

  pthread_t thid;

  if ((thid = pthread_create(&thid,NULL,diagnostic,NULL)) != 0) {
    perror("diagnoser thread create : ");
    exit(1);
  }

  return thid;

}

void * diagnostic (void * arg) {

  char test[128];
  dronecommand_t cmd;

  while (1) {

    scanf("%s",test); 
    switch_on_ssm();
    scanf("%s",test); 
    cmd.cmd_type = TAKEOFF_CMD;
    set_command(&cmd);    

  }

}
