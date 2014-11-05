/* main.c */
#include <stdio.h>
#include "droneLib/control_cmd.h"
#include "droneLib/utils_cmd.h"
#include "com/drone_com.h"
#include "diagnoser/diagnoser.h"
#include "sfe_data/sfe_data_management.h"

static pthread_t th_sfe_data_manager;
static pthread_t th_diagnoser;

void init_sfd();
void stop_sfd();

int main(void) {

  init_sfd();

  while(1);

  stop_sfd();

  return 0;
}

void init_sfd() {

  char buf[MAX_AT_CMD_LENGTH];

  init_com(); // initializes all structures needed for the communication
  th_sfe_data_manager = init_sfe_data_manager();
  th_diagnoser = start_diagnoser();

  // send local ip
  send_sfd_ip();

  // disabling navdata demo
  disable_navdata_demo_cmd(buf);
  nominal_state_sending(buf,MAX_AT_CMD_LENGTH);

}

void stop_sfd() {

  stop_sfe_data_manager();
  pthread_join(th_sfe_data_manager,NULL);
}
