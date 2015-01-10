#include "debugPage.h"
extern gui_t *gui;

void open_file_callback(GtkWidget* win){
	/*-----create a dialog to open a file selector-----*/
	/*gui->dialogFile = gtk_file_chooser_dialog_new ("Open File",
	                              GTK_WINDOW(gui->mainWindow),
	                              GTK_FILE_CHOOSER_ACTION_OPEN,
	                              GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
	                              GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
	                              NULL);*/
	/*-----open the selected file with gedit-----*/
	/*if (gtk_dialog_run (GTK_DIALOG (gui->dialogFile)) == GTK_RESPONSE_ACCEPT) {
	   char *filename;
	        char t[50];
	   filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER(gui->dialogFile));
	        sprintf(t,"%s%s","gedit ",filename);
	        system(t);
	   g_free (filename);
	   }
	   gtk_widget_destroy (gui->dialogFile);   */
	if (system("gedit DataModel/logSFM &") != 0) {
		fprintf(stderr,"Failed to open the log file \n");
		perror("Problem while opening the logSFM file.");
	}
}


void debugPage(){
	/*-----Create a table of 8 rows and 6 lines-----*/
	gui->tableDebugPage = gtk_table_new(6, 8, TRUE);
	gtk_table_set_col_spacings(GTK_TABLE(gui->tableDebugPage), 10);

	/*-----create labels-----*/
	gui->labelDebugGeneral = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL (gui->labelDebugGeneral),"<big><b>Drone's general state : </b></big>");
	gui->labelDebugSignature = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL (gui->labelDebugSignature),"<big><b>Signature : </b></big>");
	gui->labelRoll = gtk_label_new ("Roll : ");
	gui->labelVYaw = gtk_label_new ("Vyaw : ");
	gui->labelPitch = gtk_label_new ("Pitch : ");
	gui->labelVx = gtk_label_new ("Vx : ");
	gui->labelVy = gtk_label_new ("Vy : ");
	gui->labelVz = gtk_label_new ("Vz : ");
	gui->labelSignatureState = gtk_label_new (" ");
	gtk_label_set_markup (GTK_LABEL (gui->labelSignatureState),"<b>Signature State : </b>");
	gui->labelRollGeneral = gtk_label_new ("Roll : ");
	gui->labelVYawGeneral = gtk_label_new ("VYaw : ");
	gui->labelPitchGeneral = gtk_label_new ("Pitch : ");
	gui->labelflightState = gtk_label_new ("Flight State : ");
	gtk_label_set_markup (GTK_LABEL (gui->labelflightState),"<b>Flight State : </b>");
	gui->labelVxGeneral = gtk_label_new ("Vx : ");
	gui->labelVyGeneral = gtk_label_new ("Vy : ");
	gui->labelVzGeneral = gtk_label_new ("Vz : ");
	gui->labelSF = gtk_label_new ("Open Source Project");

	/*-----init openMINDED images-----*/
	gui->pixbuf = gdk_pixbuf_new_from_file_at_size("logo_OM.png",40,40,NULL);
	gui->imgSmartFox = gtk_image_new_from_pixbuf(gui->pixbuf);
	gui->pixbufFile = gdk_pixbuf_new_from_file_at_size("open_file.jpg",40,40,NULL);
	gui->imgFile = gtk_image_new_from_pixbuf(gui->pixbufFile);

	/*-----create text entries-----*/
	gui->textEntryRollSignature = gtk_entry_new ();
	gui->textEntryVYawSignature = gtk_entry_new ();
	gui->textEntryPitchSignature = gtk_entry_new ();
	gui->textEntryVxSignature = gtk_entry_new ();
	gui->textEntryVySignature = gtk_entry_new ();
	gui->textEntryVzSignature = gtk_entry_new ();
	gui->textEntrySignatureState = gtk_entry_new ();
	gui->textEntryRollGeneral = gtk_entry_new ();
	gui->textEntryVYawGeneral = gtk_entry_new ();
	gui->textEntryPitchGeneral = gtk_entry_new ();
	gui->textEntryVxGeneral = gtk_entry_new ();
	gui->textEntryVyGeneral = gtk_entry_new ();
	gui->textEntryVzGeneral = gtk_entry_new ();
	gui->textEntryFlightState = gtk_entry_new ();
	gtk_entry_set_editable(GTK_ENTRY(gui->textEntryRollSignature),FALSE);
	gtk_entry_set_editable(GTK_ENTRY(gui->textEntryVYawSignature),FALSE);
	gtk_entry_set_editable(GTK_ENTRY(gui->textEntryPitchSignature),FALSE);
	gtk_entry_set_editable(GTK_ENTRY(gui->textEntryVxSignature),FALSE);
	gtk_entry_set_editable(GTK_ENTRY(gui->textEntryVySignature),FALSE);
	gtk_entry_set_editable(GTK_ENTRY(gui->textEntryVzSignature),FALSE);
	gtk_entry_set_editable(GTK_ENTRY(gui->textEntrySignatureState),FALSE);
	gtk_entry_set_editable(GTK_ENTRY(gui->textEntryRollGeneral),FALSE);
	gtk_entry_set_editable(GTK_ENTRY(gui->textEntryVYawGeneral),FALSE);
	gtk_entry_set_editable(GTK_ENTRY(gui->textEntryPitchGeneral),FALSE);
	gtk_entry_set_editable(GTK_ENTRY(gui->textEntryVxGeneral),FALSE);
	gtk_entry_set_editable(GTK_ENTRY(gui->textEntryVyGeneral),FALSE);
	gtk_entry_set_editable(GTK_ENTRY(gui->textEntryVzGeneral),FALSE);
	gtk_entry_set_editable(GTK_ENTRY(gui->textEntryFlightState),FALSE);

	/*-----create tooptips-----*/
	gui->tooltipsFile = gtk_tooltips_new ();
	gtk_tooltips_set_tip (gui->tooltipsFile,gui->imgFile,"Click to open calculate file", NULL);

	/*-----create event box to display images-----*/
	gui->eventBoxImgFile = gtk_event_box_new ();

	/*-----create vboxes-----*/
	gui->vboxDebugGeneral = gtk_vbox_new(TRUE, 30);
	gui->vboxDebugSignature = gtk_vbox_new(TRUE, 30);
	gui->vboxDebugGeneral1 = gtk_vbox_new(TRUE, 30);
	gui->vboxDebugSignature1 = gtk_vbox_new(TRUE, 30);
	gui->vboxDebugTextEntryGeneral = gtk_vbox_new(TRUE, 15);
	gui->vboxDebugTextEntryGeneral1 = gtk_vbox_new(TRUE, 15);
	gui->vboxDebugTextEntryGeneral = gtk_vbox_new(TRUE, 15);
	gui->vboxDebugTextEntryGeneral1 = gtk_vbox_new(TRUE, 15);
	gui->vboxDebugTextEntrySignature = gtk_vbox_new(TRUE, 15);
	gui->vboxDebugTextEntrySignature1 = gtk_vbox_new(TRUE, 15);

	/*-----create valigns-----*/
	gui->valignDebugGeneral = gtk_alignment_new(0, 0, 20, 0);
	gui->valignDebugSignature = gtk_alignment_new(0, 0, 20, 0);
	gui->valignDebugGeneral1 = gtk_alignment_new(0, 0, 20, 0);
	gui->valignDebugSignature1 = gtk_alignment_new(0, 0, 20, 0);
	gui->valignDebugTextEntryGeneral = gtk_alignment_new(0, 0, 0, 0);
	gui->valignDebugTextEntryGeneral1 = gtk_alignment_new(0, 0, 0, 0);
	gui->valignDebugTextEntrySignature = gtk_alignment_new(0, 0, 0, 0);
	gui->valignDebugTextEntrySignature1 = gtk_alignment_new(0, 0, 0, 0);

	/*-----add imgFile to event box and set clicable-----*/
	gtk_container_add (GTK_CONTAINER (gui->eventBoxImgFile), gui->imgFile);
	gtk_widget_set_events (gui->eventBoxImgFile, GDK_BUTTON_PRESS_MASK);
	gtk_signal_connect (GTK_OBJECT(gui->eventBoxImgFile),"button_press_event",GTK_SIGNAL_FUNC (open_file_callback), NULL);

	/*-----complete vboxes-----*/
	gtk_box_pack_start(GTK_BOX(gui->vboxDebugGeneral),gui->labelRollGeneral, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(gui->vboxDebugGeneral),gui->labelPitchGeneral, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(gui->vboxDebugGeneral),gui->labelVYawGeneral, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(gui->vboxDebugGeneral1),gui->labelVxGeneral, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(gui->vboxDebugGeneral1),gui->labelVyGeneral, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(gui->vboxDebugGeneral1),gui->labelVzGeneral, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(gui->vboxDebugTextEntryGeneral),gui->textEntryRollGeneral, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(gui->vboxDebugTextEntryGeneral),gui->textEntryPitchGeneral, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(gui->vboxDebugTextEntryGeneral),gui->textEntryVYawGeneral, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(gui->vboxDebugTextEntryGeneral1),gui->textEntryVxGeneral, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(gui->vboxDebugTextEntryGeneral1),gui->textEntryVyGeneral, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(gui->vboxDebugTextEntryGeneral1),gui->textEntryVzGeneral, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(gui->vboxDebugSignature),gui->labelRoll, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(gui->vboxDebugSignature),gui->labelPitch, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(gui->vboxDebugSignature),gui->labelVYaw, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(gui->vboxDebugSignature1),gui->labelVx, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(gui->vboxDebugSignature1),gui->labelVy, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(gui->vboxDebugSignature1),gui->labelVz, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(gui->vboxDebugTextEntrySignature),gui->textEntryRollSignature, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(gui->vboxDebugTextEntrySignature),gui->textEntryPitchSignature, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(gui->vboxDebugTextEntrySignature),gui->textEntryVYawSignature, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(gui->vboxDebugTextEntrySignature1),gui->textEntryVxSignature, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(gui->vboxDebugTextEntrySignature1),gui->textEntryVySignature, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(gui->vboxDebugTextEntrySignature1),gui->textEntryVzSignature, FALSE, TRUE, 0);

	/*-----complete valigns-----*/
	gtk_container_add(GTK_CONTAINER(gui->valignDebugGeneral), gui->vboxDebugGeneral);
	gtk_container_add(GTK_CONTAINER(gui->valignDebugGeneral1), gui->vboxDebugGeneral1);
	gtk_container_add(GTK_CONTAINER(gui->valignDebugTextEntryGeneral), gui->vboxDebugTextEntryGeneral);
	gtk_container_add(GTK_CONTAINER(gui->valignDebugTextEntryGeneral1), gui->vboxDebugTextEntryGeneral1);
	gtk_container_add(GTK_CONTAINER(gui->valignDebugTextEntrySignature), gui->vboxDebugTextEntrySignature);
	gtk_container_add(GTK_CONTAINER(gui->valignDebugTextEntrySignature1), gui->vboxDebugTextEntrySignature1);
	gtk_container_add(GTK_CONTAINER(gui->valignDebugSignature), gui->vboxDebugSignature);
	gtk_container_add(GTK_CONTAINER(gui->valignDebugSignature1), gui->vboxDebugSignature1);

	/*-----Add valigns to table-----*/
	gtk_table_attach_defaults(GTK_TABLE(gui->tableDebugPage), gui->labelDebugGeneral,0,8,0,1);
	gtk_table_attach_defaults(GTK_TABLE(gui->tableDebugPage), gui->valignDebugGeneral,0,1,1,3);
	gtk_table_attach_defaults(GTK_TABLE(gui->tableDebugPage), gui->valignDebugGeneral1,3,4,1,3);
	gtk_table_attach_defaults(GTK_TABLE(gui->tableDebugPage), gui->valignDebugTextEntryGeneral,1,4,1,3);
	gtk_table_attach_defaults(GTK_TABLE(gui->tableDebugPage), gui->valignDebugTextEntryGeneral1,4,8,1,3);
	gtk_table_attach_defaults(GTK_TABLE(gui->tableDebugPage), gui->labelflightState,6,8,1,2);
	gtk_table_attach_defaults(GTK_TABLE(gui->tableDebugPage), gui->textEntryFlightState,6,8,1,3);
	gtk_table_attach_defaults(GTK_TABLE(gui->tableDebugPage), gui->labelDebugSignature,0,8,3,4);
	gtk_table_attach_defaults(GTK_TABLE(gui->tableDebugPage), gui->valignDebugSignature,0,1,4,7);
	gtk_table_attach_defaults(GTK_TABLE(gui->tableDebugPage), gui->valignDebugSignature1,3,4,4,7);
	gtk_table_attach_defaults(GTK_TABLE(gui->tableDebugPage), gui->valignDebugTextEntrySignature,1,4,4,7);
	gtk_table_attach_defaults(GTK_TABLE(gui->tableDebugPage), gui->valignDebugTextEntrySignature1,4,8,4,7);
	gtk_table_attach_defaults(GTK_TABLE(gui->tableDebugPage), gui->labelSignatureState,6,8,4,5);
	gtk_table_attach_defaults(GTK_TABLE(gui->tableDebugPage), gui->textEntrySignatureState,6,8,4,6);
	gtk_table_attach_defaults(GTK_TABLE(gui->tableDebugPage), gui->imgSmartFox, 2,3,7,8);
	gtk_table_attach_defaults(GTK_TABLE(gui->tableDebugPage), gui->labelSF, 1,8,7,8);
	gtk_table_attach_defaults(GTK_TABLE(gui->tableDebugPage), gui->eventBoxImgFile, 5,6,3,4);

	/*-----Add table to frame associated with debug page-----*/
	gtk_container_add (GTK_CONTAINER (gui->frameNoteBook), gui->tableDebugPage);
}
