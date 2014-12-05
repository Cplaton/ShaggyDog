#include "configurePage.h"
typedef struct
{
    gint     index;
    gchar *  p_text;
}
combo_data_st;



extern gui_t *gui;

char * select_mission;

/**
 * @var 	debugModeOn
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
 * @var     options
 * @brief   informations that are sent to the rest of the program to indicate what have been selected via the gui.
 **/
options_t options;

/**
 * @var     selectMissionDialog
 * @brief   informations that are sent to the rest of the program to indicate what have been selected via the gui.
 **/
GtkComboBox *selectMissionDialog;

/**
 * @fn      check_selected_mission_id
 * @param   widget
 * @param   data        UI element that generated the call of this function
 * @brief   manage the
 */
void check_selected_mission_id(GtkWidget *widget, gpointer data);

GtkWidget *comboMission;
GList *listMission = NULL;



#if GTK_CHECK_VERSION (2, 6, 0)
#define combo_box_active_get_text(combo_box) gtk_combo_box_get_active_text (combo_box)
#else
char *combo_box_active_get_text (GtkComboBox *combo_box)
{
    gchar *s_text = NULL;
    gboolean b_ret = FALSE;
    GtkTreeIter iter;
    
    g_return_val_if_fail (combo_box != NULL, s_text);
    
    b_ret = gtk_combo_box_get_active_iter (combo_box, &iter);
    if (b_ret)
    {
        GtkTreeModel *p_model = NULL;
        
        p_model = gtk_combo_box_get_model (combo_box);
        if (p_model != NULL)
        {
            gtk_tree_model_get (p_model, &iter, 0, &s_text, -1);
        } 
    } 
    return s_text; 
} 
#endif


/*
 * Fonction qui recupere les donnees de l'element courant affiche.
 */
combo_data_st get_active_data (GtkComboBox * p_combo)
{
    GtkTreeModel   *  p_model = NULL;
    GtkTreeIter       iter;
    combo_data_st     p_st;
    
    
    /* On recupere le modele qu'on a cree. */
    p_model = gtk_combo_box_get_model (p_combo);
    
    /* On recupere le GtkTreeIter de l'element courant. */
    if (gtk_combo_box_get_active_iter (p_combo, & iter))
    {
        /*
         * On recupere les donnees de l'element courant a savoir
         * un entier et une chaine de caracteres.
         */
        gtk_tree_model_get (
                            p_model,
                            & iter,
                            0, & p_st.index,
                            1, & p_st.p_text,
                            -1
                            );
    }
    
    
    return p_st;
}


void start_flight();

/**
 * @fn      chooseMission
 * @brief   displays a dialog that ask the user to select the mission he wants to use.
 */
void chooseMission(){
    GtkWidget *Bouton;
    gint i;
    GtkTreeIter iter;
    
    printf("----Choose mission\n");

    selectMissionDialog = gtk_dialog_new();
    gtk_window_set_title(GTK_WINDOW(selectMissionDialog), "Select your mission");
    
    gtk_widget_show(selectMissionDialog);
    
    
    
    
    GtkListStore * p_model = gtk_list_store_new (2, G_TYPE_INT, G_TYPE_STRING);
    comboMission = gtk_combo_box_new_with_model (GTK_TREE_MODEL (p_model));
    
    GtkCellRenderer * p_cell = NULL;
    
    p_cell = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (comboMission), p_cell, FALSE);
    gtk_cell_layout_set_attributes (
                                    GTK_CELL_LAYOUT (comboMission),
                                    p_cell, "text", 0,
                                    NULL
                                    );
    
    p_cell = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (comboMission), p_cell, FALSE);
    gtk_cell_layout_set_attributes (
                                    GTK_CELL_LAYOUT (comboMission),
                                    p_cell, "text", 1,
                                    NULL
                                    );
    
    
    
    
    
    for(i=0; i<5; i++){
    
        /* Ajout d'un nouvel element dans le magasin. */
        gtk_list_store_append (p_model, & iter);
    
        /* On remplit le nouvel element. */
        gtk_list_store_set (
                        p_model, & iter,
                        0, i, 1,mission_select_list[i],
                        -1
                        );
    
    
    }
    /*
        listMission = g_list_append(listMission, g_strdup_printf("Mission %d", i)); // add the mission to the list
    
    comboMission = gtk_combo_new();
    gtk_combo_set_popdown_strings( GTK_COMBO(comboMission), listMission) ;
    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(comboMission)->entry), "choose your mission");
    */
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(selectMissionDialog)->vbox), comboMission, TRUE, TRUE, 0);
    gtk_widget_show(comboMission);
    
    Bouton = gtk_button_new_with_label("Select");
    gtk_signal_connect_object(GTK_OBJECT(Bouton), "clicked", (GtkSignalFunc)check_selected_mission_id, NULL);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(selectMissionDialog)->action_area), Bouton , TRUE, TRUE, 0);
    
    gtk_widget_show(Bouton);

}

/**
 * @fn      check_selected_mission_id
 * @param   widget
 * @param   data        UI element that generated the call of this function
 * @brief   manage the
 */
void check_selected_mission_id(GtkWidget *widget, gpointer data){
    printf("----Check selected mission id\n");
    
    // get the mission number
    combo_data_st index =  get_active_data(comboMission);
    printf("----mission id %s\n", index.p_text);
    if (index.p_text != NULL) {
        select_mission = index.p_text;
    
        // hide the dialog
        gtk_widget_hide(selectMissionDialog);
    

    
         // delete the dialog
    
    
        // start the mission
        start_flight();
    }    
}

/**
 * @fn      check_button_callback
 * @param   widget
 * @param   data        UI element that generated the call of this function
 * @brief   manage the click on the finish button of the main page
 */
void check_button_callback(GtkWidget *widget, gpointer data){
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gui->checkButtonSaturation))==TRUE) {
		options.saturation = 1;
	}else{
		options.saturation = 0;
	}
    
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gui->checkButtonSys))==TRUE) {
		options.sysUrgenceExtreme = 1;
	}else{
		options.sysUrgenceExtreme = 0;
	}
    
    // Emergency landing button management
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gui->checkButtonSePoser))==TRUE) {
		options.sePoser = 1;
	}else{
		options.sePoser = 0;
	}
    
    //
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gui->checkButtonSMLimited))==TRUE) {
		options.SMLimited = 1;
	}else{
		options.SMLimited = 0;
	}
    
    // Debug Mode management
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gui->checkButtonDebug))==TRUE) {
		options.debug = 1;
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
		options.debug = 0;
	}
    
    // Disable Smartfox Safety Mode management
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gui->checkButtonDisableSSM))==TRUE) {
		options.disableSSM = 1;
	}else{
		options.disableSSM = 0;
	}
    
    // Mission checkbox management
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gui->checkButtonMission))==TRUE) {
		options.debug = 1;
		options.disableSSM = 1;
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
		options.mission = 0;
        options.disableSSM = 1;
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
    char info[20];
    int i;
    float batteryLevel = 0.0, wifiLevel = 0.0;
    
    gdk_color_parse ("red", &(gui->RED_COLOR));
    printf("----Start flight function\n");
    options.mission = 1;



    /*-----switch the page to the next-----*/
    gtk_notebook_next_page(GTK_NOTEBOOK(gui->notebook));
    
    /*------write in the option structure-----*/
    writeOpt(&options);
    
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
        sprintf(info,"%d%s",(int)batteryLevel,"%");
        gtk_entry_set_text(GTK_ENTRY(gui->textEntryBattery),info);
        if(batteryLevel < 20.0) {
            gtk_widget_modify_text(GTK_WIDGET(gui->textEntryBattery), GTK_STATE_NORMAL, &(gui->RED_COLOR));
        }
        
        /*-----get wifi-----*/
        wifiLevel = get_wifi_quality();
        sprintf(info,"%d",(int)wifiLevel);
        gtk_entry_set_text(GTK_ENTRY(gui->textEntryWifi),info);
        
        /*-----get drone flight state-----*/
        droneState = get_drone_state();
        switch (droneState) {
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
        /*-----periode of 500ms-----*/
        //usleep(500000);
        while (gtk_events_pending()) {gtk_main_iteration (); }
    }
}


void configPage(){
	/*-----create chars to put texts for infobulles-----*/
	char infoBullSaturation[2000] = "It limits commands which could be sent to the drone in order to ensure that the safety procedures could be done efficiently";
	char infoBullSePoser[2000] = "The drone will land if an error is detected (if not activated, first try an automatic emergency procedure)";
	char infoBullSMLimited[2000] = "Emergency procedures are not activated for minor faults (ie: top obstacle and bottom obstacle)";
	char infoBullDebug[2000] = "Enable the user to see all informations dynamically on diagnosis and safety softwares (in the debug window) and create log files that register all information during execution";
	char infoBullDisableSSM[2000] = "The application only detects error, the drone will not try to land or to get off detected obsatcles.";
	char infoBullSys[2000] = "It will send a landing order if drone state is too dangerous,and any procedure could protect the drone. It may not work for high speeds.";
	char infoBullMission[2000] = "It will start a mission for filling the database which it is used for the learning process";

	/*-----Create a table of 8 rows and 7 lines-----*/
	gui->tableConfigPage = gtk_table_new(7, 8, TRUE);
	gtk_table_set_col_spacings(GTK_TABLE(gui->tableConfigPage), 10);

	/*-----create the labels-----*/
	gui->labelConfigPageTop = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL (gui->labelConfigPageTop),
	                      "<b>Configure the following parameters to start the application</b>");
	gui->labelSF = gtk_label_new ("Open Source Project");

	/*-----create check buttons ------*/
	gui->checkButtonSaturation = gtk_check_button_new_with_label("Saturation");
	gui->checkButtonSys = gtk_check_button_new_with_label("Extreme Emergency System ");
	gui->checkButtonSePoser = gtk_check_button_new_with_label("Landing");
	gui->checkButtonSMLimited = gtk_check_button_new_with_label("Limited Smart Safety Mode");
	gui->checkButtonDebug = gtk_check_button_new_with_label("Debug mode");
	gui->checkButtonDisableSSM = gtk_check_button_new_with_label("Disable Smart Safety Mode");
	gui->checkButtonMission = gtk_check_button_new_with_label("Mission for learning process");

	/*-----create finish button-----*/
	gui->buttonFinish = gtk_button_new_with_label("Finish");
	gtk_widget_set_size_request(gui->buttonFinish, 90, 40);
	/*-----add connect to finish button-----*/
	gtk_signal_connect (GTK_OBJECT(gui->buttonFinish), "clicked",
	                    GTK_SIGNAL_FUNC (check_button_callback), NULL);

	/*-----create tooltips-----*/
	gui->tooltipsSaturation = gtk_tooltips_new ();
	gui->tooltipsSePoser = gtk_tooltips_new ();
	gui->tooltipsSMLimited = gtk_tooltips_new ();
	gui->tooltipsDebug = gtk_tooltips_new ();
	gui->tooltipsDisableSSM = gtk_tooltips_new ();
	gui->tooltipsSys = gtk_tooltips_new ();
	gui->tooltipsMission = gtk_tooltips_new ();

	/*-----associate tooltips with its checkbutton-----*/
	gtk_tooltips_set_tip (gui->tooltipsSePoser,gui->checkButtonSePoser,infoBullSePoser, NULL);
	gtk_tooltips_set_tip (gui->tooltipsSMLimited, gui->checkButtonSMLimited,infoBullSMLimited, NULL);
	gtk_tooltips_set_tip (gui->tooltipsDebug, gui->checkButtonDebug,infoBullDebug, NULL);
	gtk_tooltips_set_tip (gui->tooltipsSys, gui->checkButtonSys,infoBullSys, NULL);
	gtk_tooltips_set_tip (gui->tooltipsDisableSSM, gui->checkButtonDisableSSM,infoBullDisableSSM, NULL);
	gtk_tooltips_set_tip (gui->tooltipsSaturation, gui->checkButtonSaturation,infoBullSaturation, NULL);
	gtk_tooltips_set_tip (gui->tooltipsSaturation, gui->checkButtonSaturation,infoBullSaturation, NULL);
	gtk_tooltips_set_tip (gui->tooltipsMission, gui->checkButtonMission,infoBullMission, NULL);

	/*-----create smart fox image-----*/
	gui->pixbuf = gdk_pixbuf_new_from_file_at_size("logo_OM.png",40,40,NULL);
	gui->imgSmartFox = gtk_image_new_from_pixbuf(gui->pixbuf);

	/*-----create vbox which contains all checkboxes -----*/
	gui->vboxCheckButton = gtk_vbox_new(TRUE, 20);

	/*-----creat aligns-----*/
	gui->valignCheckButton = gtk_alignment_new(0, 0, 0, 0);
	gui->halignButtonFinish = gtk_alignment_new(0, 0, 0, 0);

	/*-----add checkboxes to the vbox and set the size of 300*220-----*/
	gtk_container_add(GTK_CONTAINER(gui->vboxCheckButton), gui->checkButtonSaturation);
	gtk_widget_set_size_request(gui->vboxCheckButton,300,220);
	gtk_container_add(GTK_CONTAINER(gui->vboxCheckButton), gui->checkButtonSys);
	gtk_container_add(GTK_CONTAINER(gui->vboxCheckButton), gui->checkButtonSePoser);
	//gtk_container_add(GTK_CONTAINER(gui->vboxCheckButton), gui->checkButtonSMLimited);
	gtk_container_add(GTK_CONTAINER(gui->vboxCheckButton), gui->checkButtonDebug);
	gtk_container_add(GTK_CONTAINER(gui->vboxCheckButton), gui->checkButtonDisableSSM);
	gtk_container_add(GTK_CONTAINER(gui->vboxCheckButton), gui->checkButtonMission);

	/*-----add finish button to haligh-----*/
	gtk_container_add(GTK_CONTAINER(gui->halignButtonFinish), gui->buttonFinish);

	/*-----add vbox to valign-----*/
	gtk_container_add(GTK_CONTAINER(gui->valignCheckButton), gui->vboxCheckButton);

	/*-----add components to the table-----*/
	gtk_table_attach_defaults(GTK_TABLE(gui->tableConfigPage), gui->labelConfigPageTop, 0,8,0,1);
	gtk_table_attach_defaults(GTK_TABLE(gui->tableConfigPage), gui->valignCheckButton, 1,8,1,5);
	gtk_table_attach_defaults(GTK_TABLE(gui->tableConfigPage), gui->halignButtonFinish, 6,8,4,6);
	gtk_table_attach_defaults(GTK_TABLE(gui->tableConfigPage), gui->imgSmartFox, 2,3,5,6);
	gtk_table_attach_defaults(GTK_TABLE(gui->tableConfigPage), gui->labelSF, 1,8,5,6);

	/*-----add table to frame associated to this configure page-----*/
	gtk_container_add (GTK_CONTAINER (gui->frameNoteBook), gui->tableConfigPage);
}
