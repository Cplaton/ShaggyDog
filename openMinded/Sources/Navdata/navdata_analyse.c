/**
 * @file    navdata_analyse.c
 * @author  ShaggyDogs and Smartfox
 * @brief   Navdata management based on Smartfox's file
 * @version 2.0
 * @date    November 2014
 **/

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
#include "svm-predict.h"
#include "svm-train.h"
#include "naive.h"
#include "knn_classification.h"

#define RECORD_TIME 15 //(en s)

#define NONE 0
#define SVM 1
#define KNN 2
#define NAIVE 3
#define ALL 4


/********************************FILEs************************************/

/**
 * @var		fr
 * @brief	file in which the real data would be stored
 **/
FILE * fr;

/**
 * @var		fm
 * @brief	File in which the model data would be stored
 **/
FILE * fm;

/**
 * @var		ff
 * @brief	File in which the filtered navdata would be stored
 **/
FILE * ff;

/**
 * @var		fc
 * @brief	??
 **/
FILE * fc;

/**
 * @var		fres
 * @brief	File in which the residues of the navdata would be stored
 **/
FILE * fres;

/**
 * @var		logSFM
 * @brief	File in which the logs would be stored.
 **/
FILE * logSFM;

/**
 * @var		csv
 * @brief	File in which the navdata would be stored in CSV format (in case BDD_ENABLED is 0)
 * @warning	In case BDD_ENABLED is equal to 1, this file won't be created.
 **/
FILE * csv;

/**
 * @var		LearningBase
 * @brief	File in which the navdata would be stored in txt format (to send to libSVM) will be used as learning base and test sample for cross validation
 **/
FILE * LearningBase;
FILE * KNNBase;

/**
 * @var		tmp
 * @brief
 **/
FILE * tmp;
/********************************SENSED VALUES************************************/

/**
 * @var		av_alt
 * @brief	Average altitude of the last statements.
 **/
static float av_alt = 0.0 ;

/**
 * @var		av_pitch
 * @brief	Average pitch of the last statements.
 **/
static float av_pitch = 0.0 ;

/**
 * @var		av_roll
 * @brief	Average roll of the last statements.
 **/
static float av_roll = 0.0 ;

/**
 * @var		av_Vyaw
 * @brief	Average yaw speed of the last statements.
 **/
static float av_Vyaw = 0.0 ;

/**
 * @var		av_Vx
 * @brief	Average x speed of the last statements.
 **/
static float av_Vx = 0.0 ;

/**
 * @var		av_aVy
 * @brief	Average y speed of the last statements.
 **/
static float av_Vy = 0.0 ;

/**
 * @var		av_z speed
 * @brief	Average z speed of the last statements.
 **/
static float av_Vz = 0.0 ;

/**
 * @var		av_alt
 * @brief	Average x acceleration of the last statements.
 **/
static float ax = 0.0 ;

/**
 * @var		av_alt
 * @brief	Average y acceleration of the last statements.
 **/
static float ay = 0.0 ;

/**
 * @var		av_alt
 * @brief	Average z acceleration of the last statements.
 **/
static float az = 0.0 ;

/**
 * @var		drone_state
 * @brief	Current state of the drone (taking off, flying ...).
 * @warning This variable is protected by the state_mutex. Do not forget to call it before accessing the variable.
 * @see     drone_state_t
 **/
static drone_state_t drone_state = UNKNOWN_STATE;

/**
 * @var		drone_battery
 * @brief	Current battery level  of the drone. Value is in percentage.
 * @warning This variable is protected by the battery_mutex. Do not forget to call it before accessing the variable.
 **/
static float drone_battery = 0.0 ;

/**
 * @var		drone_battery
 * @brief	Current wifi link qualité of the drone. Value is in percentage
 * @warning This variable is protected by the wifi_mutex. Do not forget to call it before accessing the variable.
 **/
static float wifi_link_quality = 0.0;


/********************************MUTEXs************************************/


/**
 * @var		state_mutex
 * @brief	Protect the drone's state variable.
 * @warning This mutex should be called before each call to drone_state variable.
 **/
static vp_os_mutex_t state_mutex;

/**
 * @var		battery_mutex
 * @brief	Protect the drone's battery level variable.
 * @warning This mutex should be called before each call to drone_battery variable.
 **/
static vp_os_mutex_t battery_mutex;

/**
 * @var		wifi_mutex
 * @brief	Protect the drone's wifi quality indicator.
 * @warning This mutex should be called before each call to wifi_link_quality variable.
 **/
static vp_os_mutex_t wifi_mutex;

/**
 * @var		class_mutex
 * @brief	Protect the drone's class.
 * @warning This mutex should be called before each call to class_id variable.
 **/
static vp_os_mutex_t class_mutex;
/********************************OTHER VARIABLES*******************************/

/**
 * @var     class_id
 * @brief   Identifier of the current class of the drone
 * @warning This variable is protected by the class_mutex. Do not forget to call it before accessing the variable.
 **/
int class_id = 0;
int class_id_aux;

/**
 * @var     indiv
 * @brief   Value used in recognition process
 **/
//specimen indiv;
specimen specimen_buffer[10];

sample specimen_naive_buffer[10];

indiv_knn * db_data;

naive_model * nv_model;
/**
 * @var     buff_counter
 * @brief   used to store 10 specimens in an array in order to recognize the situation
 **/
int buff_counter=0;

/**
 * @var     local_cmd
 * @brief   ?
 * @see     Inputs_t
 **/
Inputs_t local_cmd;         // local command taken into account here

/**
 * @var     sfm_cmd
 * @brief   ?
 * @see     Inputs_t
 **/
Inputs_t sfm_cmd;

/**
 * @var     fault_msg
 * @brief   ?
 * @see     fault_t
 **/
fault_t fault_msg = NO_FAULT;

/**
 * @var     alert_msg
 * @brief   ?
 * @see     alert_t
 **/
alert_t alert_msg = NO_ALERT;

/**
 * @var     residues
 * @brief   ?
 * @see     Residue_t
 **/
Residue_t residues;


/**
 * @var     safetyOn
 * @brief   Inform whether the safety mode is activated(1) or not (0). Safety mode is activated when some
 *          dangerous situation are detected.
 **/
unsigned char safetyOn=0;

/**
 * @var     options
 * @brief   Contains some informations about how the program should be started (debug mode ...).
 * @see     options_t
 **/
extern options_t options;

/**
 * @var     killsig
 * @brief   ?? TODO check if used somewhere
 **/
extern unsigned char killSig;

/**
 * @var     isStopped
 * @brief   Inform whether the record has been stopped(1) or not(0).
 **/
int isStopped = 0;

/**
 * @var     isInit
 * @brief   Inform whether the diagnoser has been initialised(1) or not(0).
 **/
int isInit = 0;

/**
 * @var     recordNumber
 * @brief   The number of navdata that have already been analysed.
 **/
int recordNumber = 0;

int method_selected = KNN;

int knn_buffer[10];

int knn_buffer_counter = 0;

/*************************FUNCTION DECLARATIONs********************************/

/* according to the last navdata received, resfresh the drone state */
void extract_drone_state(const navdata_demo_t *); // GGGFÖFGEOHFEOHDFFDH

/* according to the last navdata received, refresh the drone battery level */
void refresh_battery(const navdata_demo_t *);

/* according to the last navdata received, refresh the drone wifi quality */
void refresh_wifi_quality(const navdata_wifi_t *);

void extractDesiredNavdata(const navdata_demo_t * nd, Navdata_t *selectedNavdata);
void refresh_command();

/*************************FUNCTION IMPLEMENTATIONs*******************************/

/**
 * @brief   Initialize the analyse process
 * @param   data    data to analyse
 * @return  C_OK...(usefull)
 **/
inline C_RESULT navdata_analyse_init( void * data )
{
    vp_os_mutex_init(&state_mutex);
    vp_os_mutex_init(&battery_mutex);
    vp_os_mutex_init(&wifi_mutex);

	
    if(mkdir("./DataModel", 0722) != 0)
    {
        perror("navdata_analyse_init");
    };
    
    return C_OK;
}

/**
 * @brief   Analyse the navdata and fill the files
 * @return  C_OK ...
 **/
inline C_RESULT navdata_analyse_process( const navdata_unpacked_t* const navdata )
{
    static int counter = 0 ;
    static int i;
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
        || (nd->ctrl_state >> 16) ==  CTRL_TRANS_GOTOFIX)
	{ // ready to receive data

        extractDesiredNavdata(nd,&selectedNavdata);
        refresh_command();

        if (!isInit){


        	//lecture du model naive bayes
   			if((method_selected==NAIVE && options.mission!=1) || (method_selected==ALL && options.mission!=1)){
        		while(nv_model==NULL){
            		nv_model=read_Model("naive_model");
        		}
    		}	
        	if((method_selected==KNN && options.mission!=1) || (method_selected==ALL && options.mission!=1)){
     		   db_data = load_data(KNN_DATA_SET); //TODO: gérer l'erreur d'ouverture de fichier
    		}
        
        	updateNavdata(&selectedNavdata, nd);
         	initModel(&selectedNavdata,(float32_t)(nd->altitude)/1000, nd->psi/1000);
        	initFilters(&selectedNavdata);

			// get min and max from database to normalize recognition data
			if(connect_to_database()!=0){
				printf("failed to connect database");
			}
			disconnect_to_database();
			// fin get min and max

           	isInit = 1;

			if(options.debug!=0){

				fm = open_navdata_file(NAME_MODEL_DATA);
     		    ff = open_navdata_file(NAME_FILTERED_DATA);
				fr = open_navdata_file(NAME_REAL_DATA);
				fc = open_navdata_file("selectedNav");
				fres = open_navdata_file("residue");
				logSFM=openLogFile("logSFM");

				if( options.mission==1 ){
					if(connect_to_database()!=0){
						printf("failed to connect database");
					}else{
						printf("start flight\n");
						start_new_flight();
					}
				}
    		}
		}
            //récupération et traitement des nouvelles data
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
		//1 individu est constitué à partir de 10 jeux de donné
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
		//alt,pitch,roll,Vyaw,Velocity calculation
		if(counter<=9){
			av_alt += (float32_t)(nd->altitude)/10000 ;
			av_pitch += filtered_drone_output.pitch/10 ;
			av_roll += filtered_drone_output.roll/10 ;
			av_Vyaw += filtered_drone_output.Vyaw/10 ;
			av_Vx += filtered_drone_output.Vx/10 ;
			av_Vy += filtered_drone_output.Vy/10 ;
			av_Vz += filtered_drone_output.Vz/10 ;
			counter++;
		}
		if(counter==10){
		    //acceleration calculation
			ax = (filtered_drone_output.Vx - ax)/50 ;
			ay = (filtered_drone_output.Vy - ay)/50 ;
			az = (filtered_drone_output.Vz - az)/50 ;
			counter = 0 ;

			//si mission active, on rempli la BD avec le nouvelle individu
			if( options.mission == 1 ){
				vp_os_mutex_lock(&class_mutex);
				class_id_aux=class_id;
				vp_os_mutex_unlock(&class_mutex);
				insert_new_data(time,av_alt,av_pitch,av_roll,av_Vyaw,av_Vx,av_Vy,av_Vz,ax,ay,az,class_id_aux);
                
                if(options.debug!=0){
                    fprintf(logSFM,"%d 1:%f 2:%f 3:%f 4:%f 5:%f 6:%f 7:%f 8:%f 9:%f\n",
                            class_id,
                            norm_indiv(av_pitch,1),
                            norm_indiv(av_roll,2),
                            norm_indiv(av_Vyaw,3),
                            norm_indiv(av_Vx,4),
                            norm_indiv(av_Vy,5),
                            norm_indiv(av_Vz,6),
                            norm_indiv(ax,7),
                            norm_indiv(ay,8),
                            norm_indiv(az,9));
                    
                }
                
			}else{
                
                specimen indiv;
                sample naive_indiv;
                indiv_knn knn_individu;
                indiv_knn * knn_neighbors;

                
                if((method_selected==SVM && options.mission!=1) || (method_selected==ALL && options.mission!=1)){
// descripters d'un individu pour svm
				indiv.pitch = norm_indiv(av_pitch,1);
				indiv.roll = norm_indiv(av_roll,2);
				indiv.vyaw = norm_indiv(av_Vyaw,3);
				indiv.vx = norm_indiv(av_Vx,4);
				indiv.vy = norm_indiv(av_Vy,5);
				indiv.vz = norm_indiv(av_Vz,6);
				indiv.ax = norm_indiv(ax,7);
				indiv.ay = norm_indiv(ay,8);
				indiv.az = norm_indiv(az,9);
                }

                if((method_selected==NAIVE && options.mission!=1) || (method_selected==ALL && options.mission!=1)){
// descripteurs d'un individu pour naive bayes
                naive_indiv.classe=-1;
                naive_indiv.feature[0]=av_pitch;
                naive_indiv.feature[1]=av_roll;;
                naive_indiv.feature[2]=av_Vyaw;
                naive_indiv.feature[3]=av_Vx;
                naive_indiv.feature[4]=av_Vy;
                naive_indiv.feature[5]=av_Vz;
                naive_indiv.feature[6]=ax;
                naive_indiv.feature[7]=ay;
                naive_indiv.feature[8]=az;
                }

                if((method_selected==KNN && options.mission!=1) || (method_selected==ALL && options.mission!=1)){
// descripteurs d'un individu pour knn				
				knn_individu.pitch = av_pitch;
				knn_individu.roll = av_roll;
				knn_individu.vyaw = av_Vyaw;
				knn_individu.vx = av_Vx;
				knn_individu.vy = av_Vy;
				knn_individu.vz = av_Vz;
				knn_individu.ax = ax;
				knn_individu.ay = ay;
				knn_individu.az = az;
                }

				//current individu storage in a 10 indiv array in order to used the recognition on it
                //vp_os_mutex_lock(&class_mutex);
    			
                if((method_selected==SVM && options.mission!=1) || (method_selected==ALL && options.mission!=1)){
                    specimen_buffer[buff_counter]= indiv;
                }

                if((method_selected==NAIVE && options.mission!=1) || (method_selected==ALL && options.mission!=1)){
                    specimen_naive_buffer[buff_counter]= naive_indiv;
                }
                //vp_os_mutex_unlock(&class_mutex);
				//if 10 individu are stored, we launch the recognition process
				if(buff_counter == 10){
					buff_counter = 0;
				    	
					vp_os_mutex_lock(&class_mutex);

                    if((method_selected==KNN && options.mission!=1) || (method_selected==ALL && options.mission!=1)){
				        knn_neighbors = getNeighbors (db_data, knn_individu);
				        if (knn_buffer_counter == Buffer_Size) {

				        	knn_buffer_counter = 0;
				        	class_id = getResponse_mean(knn_buffer);	
				        }
				        else{
				       		knn_buffer[knn_buffer_counter] = getResponse(knn_neighbors);
				        	knn_buffer_counter++;	
				        }
                    }
                    if((method_selected==NAIVE && options.mission!=1) || (method_selected==ALL && options.mission!=1)){
                       if(nv_model!=NULL && specimen_naive_buffer != NULL){
                            class_id =  naive_predict_mean(specimen_naive_buffer,nv_model);
                        }
                    }

                    if((method_selected==SVM && options.mission!=1) || (method_selected==ALL && options.mission!=1)){
                        predict_results res_pred = recognition_process(specimen_buffer, NAME_TRAINING_MODEL);
					    class_id = res_pred.predict_class;
                    }
                    vp_os_mutex_unlock(&class_mutex);
				}else{
					buff_counter++;
				}
				
				//write data in the log in case of debug state
				if(options.debug!=0 && (method_selected==SVM || method_selected==ALL)){
					fprintf(logSFM,"%d 1:%f 2:%f 3:%f 4:%f 5:%f 6:%f 7:%f 8:%f 9:%f\n",
                    class_id,
					indiv.pitch,
					indiv.roll,
					indiv.vyaw,
					indiv.vx,
					indiv.vy,
					indiv.vz,
					indiv.ax,
					indiv.ay,
					indiv.az);
					
				}	
			}
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
    int nb_specimen;
    int i_db;
    struct augmented_navdata * specimen;
    struct augmented_navdata * specimen_naive;
    sample ** tab_indiv;
    if(options.debug!=0 && isStopped == 0){

         close_navdata_file(fr);
         close_navdata_file(fm);
         close_navdata_file(fc);
         close_navdata_file(ff);
         close_navdata_file(fres);
         closeLogFile(logSFM);
    }

    //Arret du drone
    if(isStopped == 0){

        if( options.mission == 1 ){
            
            KNNBase = open_learning_file("KNN_BaseApp");
            
            //les lignes suivantes sont d'une qualité douteuse, et probablement à jarter plus tard
			LearningBase = open_learning_file("BaseApp");
			specimen = get_normed_values_from_db(0,-1,&nb_specimen);
            
			specimen_naive = get_values_from_db(0,-1,&nb_specimen);
			tab_indiv = (sample **)vp_os_malloc(sizeof(sample)*nb_specimen);
            
            fprintf(KNNBase, "%d\n", nb_specimen);
            
            //learning file filling
            for(i_db=0;i_db<nb_specimen;i_db++){

                    tab_indiv[i_db]=(sample *)vp_os_malloc(sizeof(sample));
                    tab_indiv[i_db]->classe=specimen_naive[i_db].class_id;
                    tab_indiv[i_db]->feature[0]=specimen_naive[i_db].pitch;
                    tab_indiv[i_db]->feature[1]=specimen_naive[i_db].roll;
                    tab_indiv[i_db]->feature[2]=specimen_naive[i_db].vyaw;
                    tab_indiv[i_db]->feature[3]=specimen_naive[i_db].vx;
                    tab_indiv[i_db]->feature[4]=specimen_naive[i_db].vy;
                    tab_indiv[i_db]->feature[5]=specimen_naive[i_db].vz;
                    tab_indiv[i_db]->feature[6]=specimen_naive[i_db].ax;
                    tab_indiv[i_db]->feature[7]=specimen_naive[i_db].ay;
                    tab_indiv[i_db]->feature[8]=specimen_naive[i_db].az;
                

                
                new_data_learning(LearningBase,specimen[i_db].class_id,specimen[i_db].pitch,specimen[i_db].roll,specimen[i_db].vyaw,specimen[i_db].vx,specimen[i_db].vy,specimen[i_db].vz,specimen[i_db].ax,specimen[i_db].ay,specimen[i_db].az);
                
				new_data_learning_KNN(KNNBase,specimen_naive[i_db].class_id,specimen_naive[i_db].pitch,specimen_naive[i_db].roll,specimen_naive[i_db].vyaw,specimen_naive[i_db].vx,specimen_naive[i_db].vy,specimen_naive[i_db].vz,specimen_naive[i_db].ax,specimen_naive[i_db].ay,specimen_naive[i_db].az);
                
            }

			close_learning_file(LearningBase);
			naive_training(tab_indiv, nb_specimen);

            disconnect_to_database();
            // apprentissage ici: d'abord cross valid (10 folds, puis génération du model (0 fold)

 //               training_model_generation(NAME_TRAINING_SET,NAME_TRAINING_MODEL,10,nb_specimen);
            fclose(KNNBase);
            

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
void refresh_command() {                        //PEPITE
	commandType_t commandType;
	get_command (&local_cmd , &commandType);
}


/**
 * @brief   Refresh the battery indicator from the last navdata received
 * @param   nd      The last navdata packet received
 **/
void refresh_battery(const navdata_demo_t * nd) {

  vp_os_mutex_lock(&battery_mutex);
  drone_battery = nd->vbat_flying_percentage;
  vp_os_mutex_unlock(&battery_mutex);

}

/**
 * @brief   Gets the battery level in percentage
 * @return  The battery level in percentage
 **/
float get_battery_level() {

  float bat_level;

  vp_os_mutex_lock(&battery_mutex);
  bat_level = drone_battery;
  vp_os_mutex_unlock(&battery_mutex);

  return bat_level;

}

/**
 * @brief   Extracts the drone state from a given navdata and update global variable with it's value
 * @param   nd     The last navdata received
 **/
void extract_drone_state(const navdata_demo_t * nd) {

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

/**
 * @brief   Gets the last drone state
 * @return  The last drone state
 **/
drone_state_t get_drone_state() {

  drone_state_t state_tempo;

  vp_os_mutex_lock(&state_mutex);
  state_tempo = drone_state;
  vp_os_mutex_unlock(&state_mutex);

  return state_tempo;
}


/**
 * @brief  refresh the wifi quality indicator from the last navdata received,
 * @param  nw      the wifi indicator received into a navdata
 **/
void refresh_wifi_quality(const navdata_wifi_t * nw) {

  vp_os_mutex_lock(&wifi_mutex);
  wifi_link_quality = nw->link_quality;
  vp_os_mutex_unlock(&wifi_mutex);

}

/**
 * @brief   Gets the wifi quality indicator
 * @return  the wifi link quality into float format
 **/
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

