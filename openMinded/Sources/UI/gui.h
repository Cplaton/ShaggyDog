#ifndef GUI_H_
#define GUI_H_

#include <gtk/gtk.h>
#include <gtk-2.0/gtk/gtk.h>
#include <stdlib.h>
#include <glib.h>
#include <string.h>
#include <stdio.h>
#include "smartfox_main.h"
#include "diagnosisPage.h"
#include "debugPage.h"
#include "configurePage.h"

//VP_SDK
#include <VP_Os/vp_os_signal.h>
#include <VP_Os/vp_os_malloc.h>

typedef struct gui
{
	/*-----windows-----*/
	GtkWidget *mainWindow;
	/*-----notebooks-----*/
	GtkWidget *notebook;
	/*-----frames-----*/
	GtkWidget *frameNoteBook;
	GtkWidget *frameAlert;
	GtkWidget *frameEmergency;
	/*-----labels-----*/
	GtkWidget *labelNoteBook;
	GtkWidget *labelConfigPageTop;
	GtkWidget *labelSF;
	GtkWidget *labelDebugGeneral;
	GtkWidget *labelDebugSignature;
	GtkWidget *labelRoll;
	GtkWidget *labelVYaw;
	GtkWidget *labelPitch;
	GtkWidget *labelVx;
	GtkWidget *labelVy;
	GtkWidget *labelVz;
	GtkWidget *labelSignatureState;
	GtkWidget *labelRollGeneral;
	GtkWidget *labelVYawGeneral;
	GtkWidget *labelPitchGeneral;
	GtkWidget *labelVxGeneral;
	GtkWidget *labelVyGeneral;
	GtkWidget *labelVzGeneral;
	GtkWidget *labelflightState;
	GtkWidget *labelAlert;
	GtkWidget *labelEmergency;
	GtkWidget *labelIndicators;
	GtkWidget *labelBattery;
	GtkWidget *labelWifi;
	GtkWidget *labelDroneS;
	/*-----images-----*/
	GtkWidget* imgSmartFox;
	GtkWidget* imgFile;
	/*-----pixbuffers-----*/
	GdkPixbuf *pixbuf;
	GdkPixbuf *pixbufFile;
	/*-----buttons-----*/
	GtkWidget *buttonFinish;
	/*-----tables-----*/
	GtkWidget *tableDisgnosisPage;
	GtkWidget *tableDebugPage;
	GtkWidget *tableConfigPage;
	/*-----textviews-----*/
	GtkWidget *textViewAlert;
	GtkWidget *textViewEmergency;
	/*-----textentries-----*/
	GtkWidget *textEntryRollSignature;
	GtkWidget *textEntryVYawSignature;
	GtkWidget *textEntryPitchSignature;
	GtkWidget *textEntryVxSignature;
	GtkWidget *textEntryVySignature;
	GtkWidget *textEntryVzSignature;
	GtkWidget *textEntrySignatureState;
	GtkWidget *textEntryRollGeneral;
	GtkWidget *textEntryVYawGeneral;
	GtkWidget *textEntryPitchGeneral;
	GtkWidget *textEntryVxGeneral;
	GtkWidget *textEntryVyGeneral;
	GtkWidget *textEntryVzGeneral;
	GtkWidget *textEntryFlightState;
	GtkWidget *textEntryBattery;
	GtkWidget *textEntryWifi;
	GtkWidget *textEntryDroneS;
	/*-----checkbuttons-----*/
	GtkWidget *checkButtonSaturation;
	GtkWidget *checkButtonSys;
	GtkWidget *checkButtonSePoser;
	GtkWidget *checkButtonSMLimited;
	GtkWidget *checkButtonDebug;
	GtkWidget *checkButtonDisableSSM;
	GtkWidget *checkButtonMission;
	/*-----boxes-----*/
	GtkWidget *vboxCheckButton;
	GtkWidget *hboxButtonFinish;
	GtkWidget *vboxDebugGeneral;
	GtkWidget *vboxDebugSignature;
	GtkWidget *vboxDebugGeneral1;
	GtkWidget *vboxDebugSignature1;
	GtkWidget *vboxDebugTextEntryGeneral;
	GtkWidget *vboxDebugTextEntryGeneral1;
	GtkWidget *vboxDebugTextEntrySignature;
	GtkWidget *vboxDebugTextEntrySignature1;
	GtkWidget *hboxDiagnosis;
	GtkWidget *vboxIndicators;
	GtkWidget *vboxIndicatorsLabel;
	/*-----alignements-----*/
	GtkWidget *valignCheckButton;
	GtkWidget *halignButtonFinish;
	GtkWidget *valignDebugGeneral;
	GtkWidget *valignDebugSignature;
	GtkWidget *valignDebugTextEntryGeneral;
	GtkWidget *valignDebugTextEntryGeneral1;
	GtkWidget *valignDebugTextEntrySignature;
	GtkWidget *valignDebugTextEntrySignature1;
	GtkWidget *valignDebugGeneral1;
	GtkWidget *valignDebugSignature1;
	GtkWidget *halignDiagnosis;
	GtkWidget *valignIndicators;
	GtkWidget *valignIndicatorsLabel;
	/*-----tooltips-----*/
	GtkTooltips *tooltipsSaturation;
	GtkTooltips *tooltipsSePoser;
	GtkTooltips *tooltipsSMLimited;
	GtkTooltips *tooltipsDebug;
	GtkTooltips *tooltipsDisableSSM;
	GtkTooltips *tooltipsSys;
	GtkTooltips *tooltipsFile;
	GtkTooltips *tooltipsMission;
	/*-----colors-----*/
	GdkColor color;
	GdkColor RED_COLOR;
	/*-----event box-----*/
	GtkWidget *eventBoxImgFile;
	/*-----dialogs-----*/
	GtkWidget *dialogFile;
	/*-----scroll window-----*/
	GtkWidget *scrolledAlert;
	GtkWidget *scrolledEmergency;
	/*-----buffers-----*/
	GtkTextBuffer* bufferAlert;
	GtkTextBuffer* bufferEmergency;
} gui_t;

gui_t *get_gui();
void diagnosisPage();
void init_gui(int argc, char **argv);

#endif
