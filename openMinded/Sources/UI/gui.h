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
#include "controller.h"

//VP_SDK
#include <VP_Os/vp_os_signal.h>
#include <VP_Os/vp_os_malloc.h>

/**
 * @brief       Structure that define all the GUI content
 **/
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
	GtkWidget *labelAlert;                      /**< Label that introduce the alert TextView. */
	GtkWidget *labelEmergency;                  /**< Label that introduce the emergency TextView. */
	GtkWidget *labelIndicators;                 /**< Label that introduce the Indicators part of the diagnosis tab. */
	GtkWidget *labelBattery;                    /**< Label that introduce the battery state textentry. */
	GtkWidget *labelWifi;                       /**< Label that introduce the wifi quality textentry. */
	GtkWidget *labelDroneS;                     /**< Label that introduce the drone state textentry. */
	GtkWidget *labelDroneClass;                 /**< Label that introduce the drone class textentry. */
	/*-----images-----*/
	GtkWidget* imgSmartFox;                     /**< Logo displayed in the gui. */
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
	GtkWidget *textViewAlert;                   /**< TextView that displays the diagnosed alerts. */
	GtkWidget *textViewEmergency;               /**< TextView that displays the reaction module decisions in case of alert. */
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
	GtkWidget *textEntryBattery;                /**< TextEntry that displays the battery state. */
	GtkWidget *textEntryWifi;                   /**< TextEntry that displays the wifi quality. */
	GtkWidget *textEntryDroneS;                 /**< TextEntry in wich the drone state is diplayed (flying, landing, taking of ...) */
	GtkWidget *textEntryDroneClass;             /**< TextEntry in wich the recognized class is displayed (Standard fly, Obstacle front/back/left/Right...). */
	/*-----checkbuttons-----*/
	GtkWidget *checkButtonSaturation;
	GtkWidget *checkButtonSys;
	GtkWidget *checkButtonSePoser;
	GtkWidget *checkButtonSMLimited;
	GtkWidget *checkButtonDebug;
	GtkWidget *checkButtonDisableSSM;
	GtkWidget *checkButtonMission;
	GtkWidget *checkButtonReaction;
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
	GtkTooltips *tooltipsReaction;
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


/**
 * @brief       Getters of the GUI
 * @return      the GUI instance
 **/
gui_t *get_gui();

/**
 * @brief       TODO: check if used
 **/
void diagnosisPage();

/**
 * @brief       Initialise the GUI instance
 **/
void init_gui(int argc, char **argv);

/**
 * @brief       Destroys the gui
 **/
void destroyGui();

#endif
