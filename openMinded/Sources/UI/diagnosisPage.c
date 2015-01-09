#include "diagnosisPage.h"
extern gui_t *gui;

void diagnosisPage(){
	/*-----Create a table of 8 rows and 6 lines-----*/
	gui->tableDisgnosisPage = gtk_table_new(6, 8, TRUE);
	gtk_table_set_col_spacings(GTK_TABLE(gui->tableDisgnosisPage), 10);

	/*----init color of frame-----*/
	gdk_color_parse ("#AFAFAF", &(gui->color));

	/*-----get openMINDED image-----*/
	gui->pixbuf = gdk_pixbuf_new_from_file_at_size("logo_OM.png",40,40,NULL);
	gui->imgSmartFox = gtk_image_new_from_pixbuf(gui->pixbuf);

	/*-----create frames-----*/
	gui->frameAlert = gtk_frame_new (NULL);
	gui->frameEmergency = gtk_frame_new (NULL);

	/*-----create textviews-----*/
	gui->textViewAlert = gtk_text_view_new();
	gui->textViewEmergency = gtk_text_view_new();

	/*-----create text entries-----*/
	gui->textEntryBattery = gtk_entry_new();
	gtk_widget_set_size_request(gui->textEntryBattery, 100, 30);
	gtk_entry_set_editable(GTK_ENTRY(gui->textEntryBattery),FALSE);

	gui->textEntryWifi = gtk_entry_new();
	gtk_widget_set_size_request(gui->textEntryWifi, 100, 30);
	gtk_entry_set_editable(GTK_ENTRY(gui->textEntryWifi),FALSE);

	gui->textEntryDroneS = gtk_entry_new();
	gtk_widget_set_size_request(gui->textEntryDroneS, 100, 30);
	gtk_entry_set_editable(GTK_ENTRY(gui->textEntryDroneS),FALSE);

	gui->textEntryDroneClass = gtk_entry_new();
	gtk_widget_set_size_request(gui->textEntryDroneClass, 100, 30);
	gtk_entry_set_editable(GTK_ENTRY(gui->textEntryDroneClass),FALSE);

	/*-----create labels-----*/
	gui->labelAlert = gtk_label_new(NULL);
	gtk_label_set_markup (GTK_LABEL (gui->labelAlert),"<big><b>Alerts : </b></big>");
	gui->labelEmergency = gtk_label_new(NULL);
	gtk_label_set_markup (GTK_LABEL (gui->labelEmergency),"<big><b>Emergency : </b></big>");
	gui->labelIndicators = gtk_label_new(NULL);
	gtk_label_set_markup (GTK_LABEL (gui->labelIndicators),"<big><b>Indicators : </b></big>");
	gui->labelSF = gtk_label_new ("Open Source Project");
	gui->labelBattery = gtk_label_new(NULL);
	gtk_label_set_markup (GTK_LABEL (gui->labelBattery),"<b>Battery : </b>");
	gui->labelWifi = gtk_label_new(NULL);
	gtk_label_set_markup (GTK_LABEL (gui->labelWifi),"<b>Wifi : </b>");
	gui->labelDroneS = gtk_label_new(NULL);
	gtk_label_set_markup (GTK_LABEL (gui->labelDroneS),"<b>Drone State : </b>");
	gui->labelDroneClass = gtk_label_new(NULL);
	gtk_label_set_markup (GTK_LABEL (gui->labelDroneClass),"<b>Class : </b>");

	/*-----get associated buffer of this text view Alert------*/
	gui->bufferAlert = gtk_text_view_get_buffer(GTK_TEXT_VIEW(gui->textViewAlert));
	gtk_text_view_set_editable(GTK_TEXT_VIEW(gui->textViewAlert),FALSE);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(gui->textViewAlert),FALSE);

	/*-----creat tags associated with the buffer-----*/
	gtk_text_buffer_create_tag(gui->bufferAlert,"fault_msg","foreground","red",NULL);
	gtk_text_buffer_create_tag(gui->bufferAlert,"normal_msg","foreground","black",NULL);

	/*-----get associated buffer of this text view Emergency------*/
	gui->bufferEmergency = gtk_text_view_get_buffer(GTK_TEXT_VIEW(gui->textViewEmergency));
	gtk_text_view_set_editable(GTK_TEXT_VIEW(gui->textViewEmergency),FALSE);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(gui->textViewEmergency),FALSE);

	/*-----creat tags associated with the buffer-----*/
	gtk_text_buffer_create_tag(gui->bufferEmergency,"no_emergency","foreground","red",NULL);
	gtk_text_buffer_create_tag(gui->bufferEmergency,"have_emergency","foreground","black",NULL);

	/*-----add scroll bar to display fault text view------*/
	gui->scrolledAlert = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW (gui->scrolledAlert),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
	gtk_container_add (GTK_CONTAINER (gui->scrolledAlert),gui->textViewAlert);

	/*-----add scroll bar to display emergency text view------*/
	gui->scrolledEmergency = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW (gui->scrolledEmergency),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
	gtk_container_add (GTK_CONTAINER (gui->scrolledEmergency),gui->textViewEmergency);

	/*-----set frames colors-----*/
	gtk_widget_modify_bg(gui->frameAlert, GTK_STATE_NORMAL, &(gui->color));
	gtk_widget_modify_bg(gui->frameEmergency, GTK_STATE_NORMAL, &(gui->color));

	/*-----create boxes-----*/
	gui->hboxDiagnosis = gtk_hbox_new(TRUE, 40);
	gui->vboxIndicators = gtk_vbox_new(TRUE, 35);
	gui->vboxIndicatorsLabel = gtk_vbox_new(TRUE, 50);

	/*-----create align-----*/
	gui->halignDiagnosis = gtk_alignment_new(1, 0, 0, 0);
	gui->valignIndicators = gtk_alignment_new(1, 0, 0, 0);
	gui->valignIndicatorsLabel = gtk_alignment_new(1, 0, 0, 0);

	/*-----add text views to frames-----*/
	gtk_container_add(GTK_CONTAINER(gui->frameAlert),gui->scrolledAlert);
	gtk_container_add(GTK_CONTAINER(gui->frameEmergency),gui->scrolledEmergency);

	/*-----add text entries to vboxes-----*/
	gtk_container_add(GTK_CONTAINER(gui->vboxIndicators),gui->textEntryBattery);
	gtk_container_add(GTK_CONTAINER(gui->vboxIndicators),gui->textEntryWifi);
	gtk_container_add(GTK_CONTAINER(gui->vboxIndicators),gui->textEntryDroneS);
	gtk_container_add(GTK_CONTAINER(gui->vboxIndicators),gui->textEntryDroneClass);

	/*-----add lebels to vboxes-----*/
	gtk_container_add(GTK_CONTAINER(gui->vboxIndicatorsLabel),gui->labelBattery);
	gtk_container_add(GTK_CONTAINER(gui->vboxIndicatorsLabel),gui->labelWifi);
	gtk_container_add(GTK_CONTAINER(gui->vboxIndicatorsLabel),gui->labelDroneS);
	gtk_container_add(GTK_CONTAINER(gui->vboxIndicatorsLabel),gui->labelDroneClass);

	/*-----add boxes to aligns-----*/
	gtk_container_add(GTK_CONTAINER(gui->valignIndicatorsLabel),gui->vboxIndicatorsLabel);
	gtk_container_add(GTK_CONTAINER(gui->valignIndicators),gui->vboxIndicators);

	/*-----attach all components to the table-----*/
	gtk_table_attach_defaults(GTK_TABLE(gui->tableDisgnosisPage), gui->labelAlert,0,3,0,1);
	gtk_table_attach_defaults(GTK_TABLE(gui->tableDisgnosisPage), gui->labelEmergency,3,6,0,1);
	gtk_table_attach_defaults(GTK_TABLE(gui->tableDisgnosisPage), gui->labelIndicators,6,8,0,1);
	gtk_table_attach_defaults(GTK_TABLE(gui->tableDisgnosisPage), gui->frameAlert,0,3,1,5);
	gtk_table_attach_defaults(GTK_TABLE(gui->tableDisgnosisPage), gui->frameEmergency,3,6,1,5);
	gtk_table_attach_defaults(GTK_TABLE(gui->tableDisgnosisPage), gui->imgSmartFox, 2,3,5,6);
	gtk_table_attach_defaults(GTK_TABLE(gui->tableDisgnosisPage), gui->labelSF, 1,8,5,6);
	gtk_table_attach_defaults(GTK_TABLE(gui->tableDisgnosisPage), gui->valignIndicatorsLabel,6,7,1,5);
	gtk_table_attach_defaults(GTK_TABLE(gui->tableDisgnosisPage), gui->valignIndicators,6,8,1,5);

	/*-----add table to frame associated with diagnosisPage-----*/
	gtk_container_add (GTK_CONTAINER (gui->frameNoteBook), gui->tableDisgnosisPage);
}
