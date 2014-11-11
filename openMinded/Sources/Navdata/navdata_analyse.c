#include <ardrone_tool/UI/ardrone_input.h>
#include <ardrone_api.h>
#include <stdio.h>
#include <Soft/Common/navdata_keys.h>
#include <Soft/Common/config_keys.h>
#include <Soft/Common/navdata_common.h>
#include <Soft/Common/control_states.h>

#include <ardrone_tool/ardrone_tool.h>
#include <ardrone_tool/ardrone_tool_configuration.h>
#include <ardrone_tool/Navdata/ardrone_navdata_client.h>
#include <ardrone_tool/Navdata/ardrone_general_navdata.h>

#include <unistd.h>
#include "Model/model.h"
#include "Model/residue.h"
#include "UI/gui.h"
#include "Navdata/navdata_analyse.h"
#include "Navdata/database/bd_management.h"
#include "utils.h"

#define RECORD_TIME 15 //(en s)
#define BDD_ENABLED 1

FILE * fr;
FILE * fm;
FILE * ff;
FILE * fc;
FILE * fres;
FILE * logSFM;
FILE * csv;

static int counter = 0 ;
static float av_alt = 0.0 ;
static float av_pitch = 0.0 ;
static float av_roll = 0.0 ;
static float av_Vyaw = 0.0 ;
static float av_Vx = 0.0 ;
static float av_Vy = 0.0 ;
static float av_Vz = 0.0 ;
static float ax = 0.0 ;
static float ay = 0.0 ;
static float az = 0.0 ;


static drone_state_t drone_state = UNKNOWN_STATE;	// current drone state : taking off, flying...
static float drone_battery = 0.0 ;			// current battery level
static float wifi_link_quality = 0.0;			// quantify the wifi link quality

static vp_os_mutex_t state_mutex;
static vp_os_mutex_t battery_mutex;
static vp_os_mutex_t wifi_mutex;
static vp_os_mutex_t class_mutex;

int class_id = 0;
int class_id_aux;

Inputs_t local_cmd,sfm_cmd;          // local command taken into account here
fault_t fault_msg = NO_FAULT;
alert_t alert_msg = NO_ALERT;
Residue_t residues;

unsigned char safetyOn=0;
extern options_t options;
extern unsigned char killSig;

/* according to the last navdata received, resfresh the drone state */
void extract_drone_state(navdata_demo_t *);

/* according to the last navdata received, refresh the drone battery level */
void refresh_battery(navdata_demo_t *);	

/* according to the last navdata received, refresh the drone wifi quality */
void refresh_wifi_quality(navdata_wifi_t *);


int isStopped = 0,           // has stopped the record or not
    isInit = 0,              // to know whether the diagnoser has been initialised
    recordNumber = 0;        // number of navdata analysed


void extractDesiredNavdata(const navdata_demo_t * nd, Navdata_t *selectedNavdata);
void refresh_command();


inline C_RESULT navdata_analyse_init( void* data )
{

    vp_os_mutex_init(&state_mutex); 
    vp_os_mutex_init(&battery_mutex);
    vp_os_mutex_init(&wifi_mutex);
		mkdir("./DataModel", 0777);
 		
    return C_OK;
}



/* Analyse navdata during the event loop */
inline C_RESULT navdata_analyse_process( const navdata_unpacked_t* const navdata )
{
    // navdata structures
    const navdata_demo_t *nd = &navdata->navdata_demo;
    const navdata_time_t *nt = &navdata->navdata_time;
    const navdata_wifi_t *nw = &navdata->navdata_wifi;

    Navdata_t selectedNavdata;
    Navdata_t model_output;
    Navdata_t filtered_drone_output;
    //Residue_t residues;
    static uint32_t time=0;
        
    extract_drone_state(nd);
    refresh_battery(nd);
    refresh_wifi_quality(nw);


    if (   (nd->ctrl_state >> 16) == CTRL_FLYING
        || (nd->ctrl_state >> 16) == CTRL_HOVERING
        || (nd->ctrl_state >> 16) ==  CTRL_TRANS_GOTOFIX) { // ready to receive data
 
    
        extractDesiredNavdata(nd,&selectedNavdata); 
        refresh_command();

        if (!isInit) {
        	updateNavdata(&selectedNavdata, nd);
         	initModel(&selectedNavdata,(float32_t)(nd->altitude)/1000, nd->psi/1000);
        	initFilters(&selectedNavdata);
           	isInit = 1;
		if(options.debug!=0){
      			fm = open_navdata_file(NAME_MODEL_DATA);
      			fr = open_navdata_file(NAME_REAL_DATA);
      			ff = open_navdata_file(NAME_FILTERED_DATA);
      			fc = open_navdata_file("selectedNav");
      			fres = open_navdata_file("residue");
      			logSFM=openLogFile("logSFM");
			//
			if( BDD_ENABLED ){
				if(connect_to_database()!=0){
					// ERROR
				} else {
					start_new_flight();
				}
			} else {
				csv = open_csv_file("WekaData");
			}
    		}
          }
					
          updateNavdata(&selectedNavdata, nd);          
          model(&local_cmd,&model_output);
          filters(&selectedNavdata,&filtered_drone_output);
          calcResidue(&model_output,&filtered_drone_output,&residues);

          alert_msg=residueAnalysis(&residues);
          fault_msg=diagnosis(0);
          if(options.disableSSM==0){
            safetyOn=smartSafetyMode(fault_msg, &filtered_drone_output);
          }
          if(safetyOn==4){
            fault_msg=diagnosis(1);
          }
	  if(options.debug!=0){
	  if(counter==0){
 	    av_alt = 0.0 ; 
 	    av_pitch = 0.0 ; 
 	    av_roll = 0.0 ;
 	    av_Vyaw = 0.0 ;
 	    av_Vx = 0.0 ;
 	    av_Vy = 0.0 ;
 	    av_Vz = 0.0 ;
	    ax = filtered_drone_output.Vx ;
	    ay = filtered_drone_output.Vy ;
	    az = filtered_drone_output.Vz ;
	  }
	  if(counter<9){
	    av_alt += (float32_t)(nd->altitude)/10000 ;
            av_pitch += filtered_drone_output.pitch/10 ;
            av_roll += filtered_drone_output.roll/10 ;
            av_Vyaw += filtered_drone_output.Vyaw/10 ;
            av_Vx += filtered_drone_output.Vx/10 ;
            av_Vy += filtered_drone_output.pitch/10 ;
            av_Vz += filtered_drone_output.pitch/10 ;
	    counter++;
	  }
	  if(counter==9){
	   ax = (filtered_drone_output.Vx - ax)/50 ;
	   ay = (filtered_drone_output.Vy - ay)/50 ;
	   az = (filtered_drone_output.Vx - az)/50 ;
	   counter = 0 ;
	   if( BDD_ENABLED ){
                vp_os_mutex_lock(&class_mutex);
                class_id_aux=class_id;
                vp_os_mutex_unlock(&class_mutex);
		insert_new_data(time,av_alt,av_pitch,av_roll,av_Vyaw,av_Vx,av_Vy,av_Vz,ax,ay,az,class_id_aux);
	   } else {
		new_data_csv(csv,av_alt,av_pitch,av_roll,av_Vyaw,av_Vx,av_Vy,av_Vz,ax,ay,az);  
	   }
	    
	  }
	
            fprintf(logSFM,"sign: %d\n",fault_msg);
            fprintf(logSFM,"drone state: alt:%f pitch:%f roll:%f Vyaw:%f Vx:%f Vy:%f Vz:%f \n time:%u\n\n",
                    (float32_t)(nd->altitude)/1000,
                    filtered_drone_output.pitch,
                    filtered_drone_output.roll,                     
                    filtered_drone_output.Vyaw, 
                    filtered_drone_output.Vx, 
                    filtered_drone_output.Vy, 
                    filtered_drone_output.Vz,
                    time/1000);

              new_data(fm, time, model_output.roll*1000, model_output.pitch*1000,						  
                       model_output.Vyaw*1000,model_output.Vx*1000,model_output.Vy*1000,
                       model_output.Vz *1000);
              new_data(fr, nt->time , nd->phi/1000, nd->theta/1000, nd->psi/1000, nd->vx/1000,nd->vy/1000,nd->altitude);
              new_data(fc, time, selectedNavdata.roll*1000, selectedNavdata.pitch*1000,
                       selectedNavdata.Vyaw*1000, selectedNavdata.Vx*1000, 
                       selectedNavdata.Vy*1000, selectedNavdata.Vz*1000);
              new_data(ff, time, filtered_drone_output.roll*1000, filtered_drone_output.pitch*1000,
                       filtered_drone_output.Vyaw*1000, filtered_drone_output.Vx*1000, 
                       filtered_drone_output.Vy*1000, filtered_drone_output.Vz*1000);
              new_data(fres,time,residues.r_roll,residues.r_pitch,residues.r_Vyaw,residues.r_Vx,residues.r_Vy,residues.r_Vz);
             }
             recordNumber++;
             time+=5000;

    }else{
      isInit=0;
    }

    return C_OK;
}

/* Relinquish the local resources after the event loop exit */
inline C_RESULT navdata_analyse_release( void )
{
  if(options.debug!=0 && isStopped == 0){
         close_navdata_file(fr);
         close_navdata_file(fm);
         close_navdata_file(ff);
         close_navdata_file(fc);
         close_navdata_file(fres);
         closeLogFile(logSFM);
	 //close_navdata_file(csv);

	 if( BDD_ENABLED ){
		disconnect_to_database();
	 } else {
		close_navdata_file(csv);
	 }
         printf("closed\n");
         isStopped = 1;
  }

  vp_os_mutex_destroy(&state_mutex);
  vp_os_mutex_destroy(&battery_mutex);  
  vp_os_mutex_destroy(&wifi_mutex);

  gtk_main();
  return C_OK;
}

void extractDesiredNavdata(const navdata_demo_t * nd, Navdata_t *selectedNavdata) {

        selectedNavdata->roll = nd->phi/1000;
        selectedNavdata->pitch = nd->theta/1000;
        selectedNavdata->Vyaw = nd->psi/1000;
        selectedNavdata->Vx = nd->vx/1000;
        selectedNavdata->Vy = nd->vy/1000;
        selectedNavdata->Vz = nd->vz/1000;

}

// keep updated the data concerning current
// motion command sent to the drone
void refresh_command() {
	commandType_t commandType;
	get_command (&local_cmd , &commandType);
}


/* according to the last navdata received, refresh the drone battery level ( in percentage) */
void refresh_battery(navdata_demo_t * nd) {

  vp_os_mutex_lock(&battery_mutex);

  drone_battery = nd->vbat_flying_percentage;

  vp_os_mutex_unlock(&battery_mutex);

} 

/* returns the battery level in percentage */
float get_battery_level() {
  
  float bat_level;

  vp_os_mutex_lock(&battery_mutex);
  bat_level = drone_battery;
  vp_os_mutex_unlock(&battery_mutex);

  return bat_level;
 
}

/*
 * according to the last navdata received, resfresh the drone state
 */
void extract_drone_state(navdata_demo_t * nd) {

  vp_os_mutex_lock(&state_mutex);

  switch (nd->ctrl_state >> 16) {
    case CTRL_FLYING:
    case CTRL_HOVERING:
    case CTRL_TRANS_GOTOFIX:
    case CTRL_TRANS_LOOPING:
      drone_state = FLYING;
      break;

    case CTRL_TRANS_TAKEOFF:
      drone_state = TAKING_OFF;
      break;

    case CTRL_TRANS_LANDING:
      drone_state = LANDING;
      break;

    case CTRL_DEFAULT:
    case CTRL_LANDED:
    default:
      drone_state = LANDED;
      break;
  }
  
  vp_os_mutex_unlock(&state_mutex);
 
}

/* 
 * To get the last drone state
 */
drone_state_t get_drone_state() {

  drone_state_t state_tempo;

  vp_os_mutex_lock(&state_mutex);
  state_tempo = drone_state;
  vp_os_mutex_unlock(&state_mutex);

  return state_tempo;
}


/* according to the last navdata received, refresh the drone wifi quality */
void refresh_wifi_quality(navdata_wifi_t * nw) {
 
  vp_os_mutex_lock(&wifi_mutex);
  wifi_link_quality = nw->link_quality;
  vp_os_mutex_unlock(&wifi_mutex);

}

/* returns the wifi link quality */
float get_wifi_quality() {
 
  float link_quality;

  vp_os_mutex_lock(&wifi_mutex);
  link_quality = wifi_link_quality;
  vp_os_mutex_unlock(&wifi_mutex);

  return link_quality;

}

/* Registering to navdata client */
BEGIN_NAVDATA_HANDLER_TABLE
  NAVDATA_HANDLER_TABLE_ENTRY(navdata_analyse_init, navdata_analyse_process, navdata_analyse_release, NULL)
END_NAVDATA_HANDLER_TABLE

