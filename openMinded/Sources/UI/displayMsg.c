/**
 * @file    displayMsg.c
 * @author  shaggydogs
 * @date    10/01/15
 * @brief   Manage the display of informations into the GUI.
 **/
#include "displayMsg.h"
extern gui_t *gui;
extern int debugModeOn;


void showRecognizedClass(int classid){

	char msg[20];

	/*-----switch different fault_t message-----*/
	switch (classid) {
	case 2:
		sprintf(msg,"%s","Front Obs");
		break;
	case 3:
		sprintf(msg,"%s","Rear Obs");
		break;
	case 4:
		sprintf(msg,"%s","Right Obs");
		break;
	case 5:
		sprintf(msg,"%s","Left Obs");
		break;
	case 6:
		sprintf(msg,"%s","Top Obs");
		break;
	case 7:
		sprintf(msg,"%s","Bottom Obs");
		break;
	case 1:
		sprintf(msg,"%s","Stationary");
		break;
	case 0:
		sprintf(msg,"%s","Unknown");
		break;
	default:
		sprintf(msg,"%s","No obstacle");
	}

	gtk_entry_set_text(GTK_ENTRY(gui->textEntryDroneClass),msg);
}

/*-----computation different fault message according to fault_t message------*/
void displayAlertMsg (fault_t msg) {
	char alertMsg[50];
	char alertMsg_tmp[200];
	time_t msgTime;
	GtkTextIter endAlert;

	/*-----switch different fault_t message-----*/
	switch (msg) {
	case OBSTACLE_DEVANT:
		sprintf(alertMsg,"%s","Obstacle in the front");
		break;
	case OBSTACLE_ARRIERE:
		sprintf(alertMsg,"%s","Obstacle at the rear");
		break;
	case OBSTACLE_DROITE:
		sprintf(alertMsg,"%s","Obstacle on the right");
		break;
	case OBSTACLE_GAUCHE:
		sprintf(alertMsg,"%s","Obstacle on the left");
		break;
	case OBSTACLE_HAUT:
		sprintf(alertMsg,"%s","Obstacle at the top");
		break;
	case OBSTACLE_BAS:
		sprintf(alertMsg,"%s","Obstacle at the bottom");
		break;
	case VENT_AVANT:
		sprintf(alertMsg,"%s","Wind from the bottom");
		break;
	case VENT_ARRIERE:
		sprintf(alertMsg,"%s","Wind from the back");
		break;
	case OBSTACLE_DROITE_AVANT:
		sprintf(alertMsg,"%s","Obstacle on the front right");
		break;
	case OBSTACLE_DROITE_ARRIERE:
		sprintf(alertMsg,"%s","Obstacle on the back right");
		break;
	case OBSTACLE_GAUCHE_ARRIERE:
		sprintf(alertMsg,"%s","Obstacle on the back left");
		break;
	case OBSTACLE_GAUCHE_DEVANT:
		sprintf(alertMsg,"%s","Obstacle on the front left");
		break;
	case OBSTACLE_DEVANT_GAUCHE:
		sprintf(alertMsg,"%s","Obstacle on the left front");
		break;
	case OBSTACLE_DEVANT_DROITE:
		sprintf(alertMsg,"%s","Obstacle on the right front");
		break;
	case OBSTACLE_ARRIERE_DROITE:
		sprintf(alertMsg,"%s","Obstacle on the right back");
		break;
	case OBSTACLE_ARRIERE_GAUCHE:
		sprintf(alertMsg,"%s","Obstacle on the left back");
		break;
	case OBSTACLE_AVANT_DESSOUS:
		sprintf(alertMsg,"%s","Obstacle on the bottom front");
		break;
	case OBSTACLE_ARRIERE_DESSOUS:
		sprintf(alertMsg,"%s","Obstacle on the bottom back");
		break;
	case OBSTACLE_ARRIERE_DESSUS:
		sprintf(alertMsg,"%s","Obstacle on the top back");
		break;
	case OBSTACLE_AVANT_DESSUS:
		sprintf(alertMsg,"%s","Obstacle top front");
		break;
	case OBSTACLE_DESSOUS_DROITE:
		sprintf(alertMsg,"%s","Obstacle on the right bottom");
		break;
	case OBSTACLE_DESSOUS_GAUCHE:
		sprintf(alertMsg,"%s","Obstacle on the left bottom");
		break;
	case OBSTACLE_DESSUS_GAUCHE:
		sprintf(alertMsg,"%s","Obstacle on the left top");
		break;
	case OBSTACLE_DESSUS_DROIT:
		sprintf(alertMsg,"%s","Obstacle on the right top");
		break;
	case NO_FAULT:
		sprintf(alertMsg,"%s","No obstacle");
		break;
	case UNKNOWN_FAULT:
		sprintf(alertMsg,"%s","Unknown fault");
		break;
	default:
		sprintf(alertMsg,"%s","No obstacle");
	}

	msgTime = time(NULL);
	if(debugModeOn == 1)
		gtk_entry_set_text(GTK_ENTRY(gui->textEntrySignatureState),alertMsg);
	//strcat(alertMsg_tmp,alertMsg);

	/*-----insert message with colors into bufferAlert-----*/
	sprintf(alertMsg_tmp,"%s%s%s\n\n",ctime(&msgTime),"    ",alertMsg);
	gtk_text_buffer_get_end_iter(gui->bufferAlert, &(endAlert));
	if(strstr(alertMsg,"No") == NULL) {
		gtk_text_buffer_insert_with_tags_by_name(gui->bufferAlert,&endAlert,alertMsg_tmp,-1,"fault_msg",NULL);
	}else{
		gtk_text_buffer_insert_with_tags_by_name(gui->bufferAlert,&endAlert,alertMsg_tmp,-1,"normal_msg",NULL);
	}
	while (gtk_events_pending()) {gtk_main_iteration (); }
}

void displayEmergencyMsg (emergency_state em) {
	GtkTextIter endEmergency;
	char alertMsg[50];
	char alertMsg_tmp[200];
	time_t msgTime;

	switch (em) {
	case NO_EMERGENCY:
		sprintf(alertMsg,"%s","No emergency\n");
		break;
	case GO_BACK:
		sprintf(alertMsg,"%s","Go back\n");
		break;
	case GO_FORWARD:
		sprintf(alertMsg,"%s","Go forward\n");
		break;
	case GO_LEFT:
		sprintf(alertMsg,"%s","Go left\n");
		break;
	case GO_RIGHT:
		sprintf(alertMsg,"%s","Go right\n");
		break;
	case GO_UP:
		sprintf(alertMsg,"%s","Go up\n");
		break;
	case GO_DOWN:
		sprintf(alertMsg,"%s","Go down\n");
		break;
	case E_STABILISATION:
		sprintf(alertMsg,"%s","Stabilisation\n");
		break;
	case LAND:
		sprintf(alertMsg,"%s","Land\n");
		break;
	case E_VERIFICATION:
		sprintf(alertMsg,"%s","Verification\n");
		break;
	case VERIFICATION_FAILED:
		sprintf(alertMsg,"%s","Verification failed\n");
		break;
	default:
		sprintf(alertMsg,"%s","No emergency\n");
	}

	msgTime = time(NULL);

	/*-----insert message with colors into bufferAlert-----*/
	sprintf(alertMsg_tmp,"%s%s%s\n",ctime(&msgTime),"    ",alertMsg);
	gtk_text_buffer_get_end_iter(gui->bufferEmergency, &(endEmergency));
	if(strstr(alertMsg,"No") == NULL) {
		gtk_text_buffer_insert_with_tags_by_name(gui->bufferEmergency,&endEmergency,alertMsg_tmp,-1,"no_emergency",NULL);
	}else{
		gtk_text_buffer_insert_with_tags_by_name(gui->bufferEmergency,&endEmergency,alertMsg_tmp,-1,"have_emergency",NULL);
	}
	while (gtk_events_pending()) {gtk_main_iteration (); }
}


void showDroneState(int state){
    static char info[20];

    switch (state) {
        case 0:
            sprintf(info,"%s","FLYING");
            break;
        case 1:
            sprintf(info,"%s","LANDING");
            break;
        case 2:
            sprintf(info,"%s","LANDED");
            break;
        case 3:
            sprintf(info,"%s","TAKING_OFF");
            break;
        case 4:
            sprintf(info,"%s","UNKNOWN_STATE");
            break;
        default:
            sprintf(info,"%s","UNKNOWN_STATE");
            break;
    }
    if(debugModeOn == 1) {
        gtk_entry_set_text(GTK_ENTRY(gui->textEntryFlightState),info);
    }
    gtk_entry_set_text(GTK_ENTRY(gui->textEntryDroneS),info);

}

void showDroneBatteryLevel(float level){
    static char info[20];
    
    sprintf(info,"%d%s",(int)level,"%");
    gtk_entry_set_text(GTK_ENTRY(gui->textEntryBattery),info);
    if(level < 20.0) {
        gtk_widget_modify_text(GTK_WIDGET(gui->textEntryBattery), GTK_STATE_NORMAL, &(gui->RED_COLOR));
    }
    
}

void showDroneWifiQualityLevel(float level){
    static char info[20];
    sprintf(info,"%d",(int)level);
    gtk_entry_set_text(GTK_ENTRY(gui->textEntryWifi),info);
}

