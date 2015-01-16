/**
 * @file    controller.c
 * @author  shaggydogs
 * @date    10/01/15
 * @brief   Manage the events generated by the gui.
 **/

#include "controller.h"

extern gui_t *gui;

char * select_mission;

/**
 * @var         debugModeOn
 * @brief	inform whether the debug mode is activated or not
 **/
int debugModeOn = 0;

/**
 * @var		stop
 * @brief	inform whether the drone is flying or if it has been stoped
 **/
int stop = 0;

/**
 * @var		isOpen
 * @brief	inform whether the mission window is open or not
 **/
int isOpen = 0;

/**
 * @var		missionModeOn
 * @brief	inform whether the mission mode is activated or not
 */
int missionModeOn = 0;

/**
 * @var     opt
 * @brief   informations that are sent to the rest of the program to indicate what have been selected via the gui.
 **/
options_t opt;


void OnDestroy(GtkWidget *pWidget, gpointer pData) {
    /*-----close other threads-----*/
    stop_request();
    /*-----close the gui-----*/
    stop=1;
    destroyGui();
    signal_exit();
    gtk_main_quit();
}

void check_selected_mission_id(GtkWidget *widget, gpointer data){
    printf("----Check selected mission id\n");
    
    // get the mission number
    combo_data_st index =  getSelectedMissionFromCombo();
    
    if (index.p_text != NULL) {
        // If the user has selected a mission
        printf("----mission id %s\n", index.p_text);
        select_mission = index.p_text;
        
        // hide the dialog
        hideMissionSelectionDialog();
        
        //Set the mission option's to 1 in order to activate the mission
        opt.mission = 1;
        
        // and start the mission
        start_flight();
    } else {
        // If the user does not select a mission
        printf ("Error : no mission selected, please select a mission.");
    }
}


/**
 * @fn      check_button_callback
 * @param   widget
 * @param   data        UI element that generated the call of this function
 * @brief   manage the click on the finish button of the main page
 */
void check_button_callback(GtkWidget *widget, gpointer data){
    
    opt.disableSSM = 1;
    
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gui->checkButtonSaturation))==TRUE) {
        opt.saturation = 1;
    }else{
        opt.saturation = 0;
    }
    
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gui->checkButtonSys))==TRUE) {
        opt.sysUrgenceExtreme = 1;
    }else{
        opt.sysUrgenceExtreme = 0;
    }
    
    // Emergency landing button management
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gui->checkButtonSePoser))==TRUE) {
        opt.sePoser = 1;
    }else{
        opt.sePoser = 0;
    }
    
    //
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gui->checkButtonSMLimited))==TRUE) {
        opt.SMLimited = 1;
    }else{
        opt.SMLimited = 0;
    }
    
    // Debug Mode management
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gui->checkButtonDebug))==TRUE) {
        opt.debug = 1;
        debugModeOn = 1;
        if(isOpen != 1) {
            gui->frameNoteBook = gtk_frame_new (NULL);
            gtk_container_set_border_width (GTK_CONTAINER (gui->frameNoteBook), 10);
            debugPage();
            gui->labelNoteBook = gtk_label_new ("Debug");
            gtk_notebook_append_page (GTK_NOTEBOOK (gui->notebook), gui->frameNoteBook, gui->labelNoteBook);
            gtk_widget_show_all(gui->frameNoteBook);
            isOpen = 1;
        }
    }else{
        opt.debug = 0;
    }
    
    // open Minded Safety Mode checkbox management
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gui->checkButtonReaction))==TRUE) {
        enable_openMinded_safety_mode = 1;
    }else{
        enable_openMinded_safety_mode = 0;
    }
    
    // Mission checkbox management
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gui->checkButtonMission))==TRUE) {
        opt.debug = 1;
        debugModeOn = 1;
        if(isOpen != 1) {
            gui->frameNoteBook = gtk_frame_new (NULL);
            gtk_container_set_border_width (GTK_CONTAINER (gui->frameNoteBook), 10);
            debugPage();
            gui->labelNoteBook = gtk_label_new ("Debug");
            gtk_notebook_append_page (GTK_NOTEBOOK (gui->notebook), gui->frameNoteBook, gui->labelNoteBook);
            gtk_widget_show_all(gui->frameNoteBook);
            isOpen = 1;
            
            // Ask the user to select it's mission
            chooseMission();
            
        }
    }else{
        opt.mission = 0;
        start_flight();
    }
}


void start_flight(){
    Navdata_t navdata;
    fault_t prec_fault_msg = NO_FAULT;
    emergency_state prec_emergency_msg = NO_EMERGENCY;
    fault_t faultMsg = NO_FAULT;
    emergency_state emergencyMsg = NO_EMERGENCY;
    alarm_t alarm[6];
    drone_state_t droneState;
    char msg[1000];
    char tabAlarm[6][15] = {"","","","","",""};
    int i;
    float batteryLevel = 0.0, wifiLevel = 0.0;
    
    gdk_color_parse ("red", &(gui->RED_COLOR));
    printf("----Start flight function\n");
    //    opt.mission = 1;
    
    
    
    /*-----switch the page to the next-----*/
    gtk_notebook_next_page(GTK_NOTEBOOK(gui->notebook));
    
    /*------write in the option structure-----*/
    writeOpt(&opt);
    
    /*-----begin the display of alerts-----*/
    while (stop==0) {
        //affect emergency message
        readEmergency(&emergencyMsg);
        if(prec_emergency_msg != emergencyMsg) {
            displayEmergencyMsg(emergencyMsg);
            prec_emergency_msg = emergencyMsg;
        }
        
        //affect fault message
        readFault(&faultMsg);
        if(prec_fault_msg != faultMsg && emergencyMsg == NO_EMERGENCY) {
            displayAlertMsg(faultMsg);
            prec_fault_msg = faultMsg;
            
            //In debug pages, text entries for drone's signature state
            if(debugModeOn ==1) {
                readSignature(alarm);
                for(i=0; i<6; i++) {
                    switch(alarm[i]) {
                        case ALARM_N:
                            sprintf(tabAlarm[i],"%s","Negatif");
                            break;
                        case ALARM_Z:
                            sprintf(tabAlarm[i],"%s","Ok");
                            break;
                        case ALARM_P:
                            sprintf(tabAlarm[i],"%s","Positif");
                            break;
                        default:
                            sprintf(tabAlarm[i],"%s","Inconnu");
                            break;
                    }
                }
                gtk_entry_set_text(GTK_ENTRY(gui->textEntryRollSignature),tabAlarm[0]);
                gtk_entry_set_text(GTK_ENTRY(gui->textEntryPitchSignature),tabAlarm[1]);
                gtk_entry_set_text(GTK_ENTRY(gui->textEntryVYawSignature),tabAlarm[2]);
                gtk_entry_set_text(GTK_ENTRY(gui->textEntryVxSignature),tabAlarm[3]);
                gtk_entry_set_text(GTK_ENTRY(gui->textEntryVySignature),tabAlarm[4]);
                gtk_entry_set_text(GTK_ENTRY(gui->textEntryVzSignature),tabAlarm[5]);
            }
        }
        
        //In debug pages, text entries for drone's general state
        if(debugModeOn == 1) {
            readNavdata(&navdata);
            sprintf(msg,"%f%s",navdata.roll,"°");
            gtk_entry_set_text(GTK_ENTRY(gui->textEntryRollGeneral),msg);
            sprintf(msg,"%f%s",navdata.Vyaw,"°/s");
            gtk_entry_set_text(GTK_ENTRY(gui->textEntryVYawGeneral),msg);
            sprintf(msg,"%f%s",navdata.pitch,"°");
            gtk_entry_set_text(GTK_ENTRY(gui->textEntryPitchGeneral),msg);
            sprintf(msg,"%f%s",navdata.Vx,"m/s");
            gtk_entry_set_text(GTK_ENTRY(gui->textEntryVxGeneral),msg);
            sprintf(msg,"%f%s",navdata.Vy,"m/s");
            gtk_entry_set_text(GTK_ENTRY(gui->textEntryVyGeneral),msg);
            sprintf(msg,"%f%s",navdata.Vz,"m/s");
            gtk_entry_set_text(GTK_ENTRY(gui->textEntryVzGeneral),msg);
        }
        
        /*----get battery level-----*/
        batteryLevel = get_battery_level();
        showDroneBatteryLevel(batteryLevel);
        
        /*-----get wifi-----*/
        wifiLevel = get_wifi_quality();
        showDroneWifiQualityLevel(wifiLevel);
        
        /*-----get drone flight state-----*/
        droneState = get_drone_state();
        showDroneState (droneState);
        
        /*-----periode of 500ms-----*/
        //usleep(500000);
        while (gtk_events_pending()) {gtk_main_iteration (); }
    }
}