#include <ardrone_api.h>
#include <ardrone_tool/UI/ardrone_input.h>

#include <ardrone_tool/ardrone_tool_configuration.h>
#include <gdk/gdk.h>
#include <gtk-2.0/gtk/gtk.h>
#include <stdio.h>
#include <glib.h>

DEFINE_THREAD_ROUTINE(th_gui, data)
{
	//int ret = 0;
	gdk_threads_enter();
	gtk_main();
	//while (gtk_events_pending()) {gtk_main_iteration ();}
	//pthread_exit(&ret);
	gdk_threads_leave();

	return (0);
}