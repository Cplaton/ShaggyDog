#include "gui.h"

gui_t *gui = NULL;
extern int stop;

gui_t *get_gui() {
  return gui;
}

void OnDestroy(GtkWidget *pWidget, gpointer pData) {
		/*-----close other threads-----*/
		stop_request();
    /*-----close the gui-----*/
		stop=1;
		gtk_widget_destroy(gui->mainWindow);
  	signal_exit();
    gtk_main_quit();		
}

/*-----main window of application-----*/
void init_gui(int argc, char **argv)
{
  gui = vp_os_malloc(sizeof (gui_t));
  gdk_threads_init();
  gtk_init(&argc, &argv);

	/*-----create main window 800*620-----*/
 	gui->mainWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_position(GTK_WINDOW(gui->mainWindow), GTK_WIN_POS_CENTER);
  gtk_window_set_default_size(GTK_WINDOW(gui->mainWindow), 800, 620);
  gtk_window_set_title(GTK_WINDOW(gui->mainWindow), "Smart Fox");
  gtk_window_set_resizable(GTK_WINDOW(gui->mainWindow), TRUE);
  gtk_container_set_border_width(GTK_CONTAINER(gui->mainWindow), 10);
	/*-----close application when click on the "X"------*/
	g_signal_connect_swapped(G_OBJECT(gui->mainWindow),"destroy",G_CALLBACK(OnDestroy),G_OBJECT(gui->mainWindow));
	
  /*-----create a new notebook, place the position of the tabs on the top and add it to the main window-----*/
  gui->notebook = gtk_notebook_new ();
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (gui->notebook), GTK_POS_TOP);
	gtk_container_add (GTK_CONTAINER (gui->mainWindow), gui->notebook);

	/*-----create a frame for the configure page and add to the notebook-----*/
	gui->frameNoteBook = gtk_frame_new (NULL);
  gtk_container_set_border_width (GTK_CONTAINER (gui->frameNoteBook), 10);
	/*-----call function configPage() to display contenues in this page-----*/
 	configPage();
  gui->labelNoteBook = gtk_label_new ("Configuration");
  gtk_notebook_append_page (GTK_NOTEBOOK (gui->notebook), gui->frameNoteBook, gui->labelNoteBook);

	/*-----create a frame for the diagnosis page and add to the notebook-----*/
	gui->frameNoteBook = gtk_frame_new (NULL);
  gtk_container_set_border_width (GTK_CONTAINER (gui->frameNoteBook), 10);
	/*-----call function diagnosisPage() to display contenues in this page-----*/
  diagnosisPage();
  gui->labelNoteBook = gtk_label_new ("Diagnosis");
  gtk_notebook_append_page (GTK_NOTEBOOK (gui->notebook), gui->frameNoteBook, gui->labelNoteBook);

  gtk_widget_show_all(gui->mainWindow);
}

