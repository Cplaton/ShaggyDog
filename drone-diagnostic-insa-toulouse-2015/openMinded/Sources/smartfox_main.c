/*
 * smartfox_main.c
 * 2013-2014
 */

#include <smartfox_main.h>

//ARDroneLib
#include <utils/ardrone_time.h>
#include <ardrone_tool/Navdata/ardrone_navdata_client.h>
#include <ardrone_tool/Control/ardrone_control.h>
#include <ardrone_tool/UI/ardrone_input.h>

//Common
#include <stdlib.h>
#include <config.h>
#include <ardrone_api.h>

//VP_SDK
#include <ATcodec/ATcodec_api.h>
#include <VP_Os/vp_os_print.h>
#include <VP_Api/vp_api_thread_helper.h>
#include <VP_Os/vp_os_signal.h>

// configuration
#include <ardrone_tool/ardrone_tool_configuration.h>

// smartfox files
#include <UserApp/smartfox_api.h>
#include <disable_navdata_demo.h>
#include <utils.h>
#include <UI/gui.h>
#include <UI/ardrone_gui.h>
#include <Model/model.h>
#include <Navdata/navdata_analyse.h>


/* ############# USER APPLICATION ############ */
/* #### add here the .h files of your app #### */
#include <UserApp/user_app.h>


// to inform navdata to start/stop the acquisition
vp_os_mutex_t stop_mutex;

static int stop_request_val = 0;

Inputs_t current_command;

/* Implementing Custom methods for the main function of an ARDrone application */
int main(int argc, char** argv)
{
  init_gui(argc,argv);
  return ardrone_tool_main(1,argv); 
}


/* The delegate object calls this method during initialization of an ARDrone application */
C_RESULT ardrone_tool_init_custom(void)
{

  /*init the mutex*/
  vp_os_mutex_init(&stop_mutex);
	
  
  START_THREAD( th_move_cmd, (void *)&current_command );
  START_THREAD( th_dis_navdata_demo,NULL);
  START_THREAD( th_gui, NULL);

  /* ####################### USER APPLICATION ##################### */
  /* ## add here the statement that start the thread of your app ## */
  START_THREAD (th_user_app, NULL);

  return C_OK;
}

/* The delegate object calls this method when the event loop exit */
C_RESULT ardrone_tool_shutdown_custom(void)
{

  vp_os_mutex_destroy(&stop_mutex); 

  JOIN_THREAD( th_move_cmd);
  JOIN_THREAD( th_dis_navdata_demo);

  /* ####################### USER APPLICATION ##################### */
  /* ## add here the statement that join the thread of your app ## */
  JOIN_THREAD(th_user_app);

  return C_OK;
}

/* The event loop calls this method for the exit condition */
bool_t ardrone_tool_exit()
{
  int stop = 0;
  vp_os_mutex_lock(&stop_mutex);
   stop = stop_request_val;
  vp_os_mutex_unlock(&stop_mutex);

  return stop;
}

C_RESULT signal_exit()
{
  return C_OK;
}

/* request to stop the app */
void stop_request() {

  if (get_drone_state() != LANDED) {
    set_command(NULL,LANDING_REQUEST);
  }
  stop_sending_commands();
  vp_os_mutex_lock(&stop_mutex);
  stop_request_val = 1;
  vp_os_mutex_unlock(&stop_mutex);
}


/* Implementing thread table in which you add routines of your application and those provided by the SDK */
BEGIN_THREAD_TABLE
  THREAD_TABLE_ENTRY( ardrone_control, 20 )
  THREAD_TABLE_ENTRY( navdata_update, 10 )
  THREAD_TABLE_ENTRY( th_move_cmd, 10 ) 
  THREAD_TABLE_ENTRY( th_dis_navdata_demo, 20 )
  THREAD_TABLE_ENTRY( th_gui,20)
  /* ####################### USER APPLICATION ##################### */
  /* ## states the threads that you are creating  ## */
  THREAD_TABLE_ENTRY( th_user_app,20)
END_THREAD_TABLE

