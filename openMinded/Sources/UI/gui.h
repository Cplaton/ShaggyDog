/**
 * @file    gui.h
 * @author  shaggydogs
 * @date    10/01/15
 * @brief   Contains the definition of the GUI structure and of it's global functions (init, destroy).
 **/
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
	GtkWidget *mainWindow;                      /**< TMain windows of the application. */
	/*-----notebooks-----*/
	GtkWidget *notebook;                        /**< Notebook that contains all the application tabs. */
	/*-----frames-----*/
	GtkWidget *frameNoteBook;                   /**< Frame that contains the configure page. */
	GtkWidget *frameAlert;                      /**< Frame in wich the alert messages are displayed. */
	GtkWidget *frameEmergency;                  /**< Frame in wich the emergency messages are displayed. */
	/*-----labels-----*/
	GtkWidget *labelNoteBook;                   /**< Label of the configuration tab. */
	GtkWidget *labelConfigPageTop;              /**< Label displayed in the top of the configure page. It explains the page usage. */
	GtkWidget *labelSF;                         /**< Label displayed in the botom of the configuration page, it displays the application name. */
	GtkWidget *labelDebugGeneral;               /**< Label introducing the general part of the debug page. */
	GtkWidget *labelDebugSignature;             /**< Label introducing the signature part of the debug page. */
	GtkWidget *labelRoll;                       /**< Label introducing the roll signature into the debug page. */
	GtkWidget *labelVYaw;                       /**< Label introducing the vyaw signature into the debug page. */
	GtkWidget *labelPitch;                      /**< Label introducing the pitch signature into the debug page. */
	GtkWidget *labelVx;                         /**< Label introducing the vx signature into the debug page. */
	GtkWidget *labelVy;                         /**< Label introducing the vy signature into the debug page. */
	GtkWidget *labelVz;                         /**< Label introducing the vz signature into the debug page. */
	GtkWidget *labelSignatureState;             /**< Label introducing the signature state into the debug page. */
	GtkWidget *labelRollGeneral;                /**< Label introducing the roll into the debug page. */
	GtkWidget *labelVYawGeneral;                /**< Label introducing the vyaw into debug page. */
	GtkWidget *labelPitchGeneral;               /**< Label introducing the pitch into the debug page. */
	GtkWidget *labelVxGeneral;                  /**< Label introducing the vx into the debug page. */
	GtkWidget *labelVyGeneral;                  /**< Label introducing the vy into the debug page. */
	GtkWidget *labelVzGeneral;                  /**< Label introducing the vz into the debug page. */
	GtkWidget *labelflightState;                /**< Label introducing the fight state into the debug page. */
	GtkWidget *labelAlert;                      /**< Label that introduce the alert TextView. */
	GtkWidget *labelEmergency;                  /**< Label that introduce the emergency TextView. */
	GtkWidget *labelIndicators;                 /**< Label that introduce the Indicators part of the diagnosis tab. */
	GtkWidget *labelBattery;                    /**< Label that introduce the battery state textentry. */
	GtkWidget *labelWifi;                       /**< Label that introduce the wifi quality textentry. */
	GtkWidget *labelDroneS;                     /**< Label that introduce the drone state textentry. */
	GtkWidget *labelDroneClass;                 /**< Label that introduce the drone class textentry. */
	/*-----images-----*/
	GtkWidget* imgSmartFox;                     /**< Logo displayed in the gui. */
	GtkWidget* imgFile;                         /**< Other logo displayed in thr gui. */
	/*-----pixbuffers-----*/
	GdkPixbuf *pixbuf;                          /**< Pixel buffer used to load the imgsmartfox. */
	GdkPixbuf *pixbufFile;                      /**< Pixel buffer used to load the img file. */
	/*-----buttons-----*/
	GtkWidget *buttonFinish;                    /**< Finish button of the configuration page, it is used to validate the selection. */
	/*-----tables-----*/
	GtkWidget *tableDisgnosisPage;              /**< Table used to define the diagnosis page structure. */
	GtkWidget *tableDebugPage;                  /**< Table used to define the debug page structure. */
	GtkWidget *tableConfigPage;                 /**< Table used to define the config page structure. */
	/*-----textviews-----*/
	GtkWidget *textViewAlert;                   /**< TextView that displays the diagnosed alerts. */
	GtkWidget *textViewEmergency;               /**< TextView that displays the reaction module decisions in case of alert. */
	/*-----textentries-----*/
	GtkWidget *textEntryRollSignature;          /**< TextEntry that displays the current roll signature into the debug page. */
	GtkWidget *textEntryVYawSignature;          /**< TextEntry that displays the current vyaw signature into the debug page. */
	GtkWidget *textEntryPitchSignature;         /**< TextEntry that displays the current pitch signature into the debug page. */
	GtkWidget *textEntryVxSignature;            /**< TextEntry that displays the current vx signature into the debug page. */
	GtkWidget *textEntryVySignature;            /**< TextEntry that displays the current vy signature into the debug page. */
	GtkWidget *textEntryVzSignature;            /**< TextEntry that displays the current vz signature into the debug page. */
	GtkWidget *textEntrySignatureState;         /**< TextEntry that displays the current state signature into the debug page. */
	GtkWidget *textEntryRollGeneral;            /**< TextEntry that displays the current roll into the debug page. */
	GtkWidget *textEntryVYawGeneral;            /**< TextEntry that displays the current vyaw into the debug page. */
	GtkWidget *textEntryPitchGeneral;           /**< TextEntry that displays the current pitch into the debug page. */
	GtkWidget *textEntryVxGeneral;              /**< TextEntry that displays the current vx into the debug page. */
	GtkWidget *textEntryVyGeneral;              /**< TextEntry that displays the current vy into the debug page. */
	GtkWidget *textEntryVzGeneral;              /**< TextEntry that displays the current vz into the debug page. */
	GtkWidget *textEntryFlightState;            /**< TextEntry that displays the current flight state into the debug page. */
	GtkWidget *textEntryBattery;                /**< TextEntry that displays the battery state. */
	GtkWidget *textEntryWifi;                   /**< TextEntry that displays the wifi quality. */
	GtkWidget *textEntryDroneS;                 /**< TextEntry in wich the drone state is diplayed (flying, landing, taking of ...) */
	GtkWidget *textEntryDroneClass;             /**< TextEntry in wich the recognized class is displayed (Standard fly, Obstacle front/back/left/Right...). */
	/*-----checkbuttons-----*/
	GtkWidget *checkButtonSaturation;           /**< Checkbox used in the configuration page te activate or not the ???. */
	GtkWidget *checkButtonSys;                  /**< Checkbox used in the configuration page te activate or not the ???. */
	GtkWidget *checkButtonSePoser;              /**< Checkbox used in the configuration page te activate or not the ???. */
	GtkWidget *checkButtonSMLimited;            /**< Checkbox used in the configuration page te activate or not the smartfox limited module. */
	GtkWidget *checkButtonDebug;                /**< Checkbox used in the configuration page te activate or not the debug mode. */
	GtkWidget *checkButtonDisableSSM;           /**< Checkbox used in the configuration page te activate or not the smartfox safety module (deprecated). */
	GtkWidget *checkButtonMission;              /**< Checkbox used in the configuration page te activate or not the mission mode. */
	GtkWidget *checkButtonReaction;             /**< Checkbox used in the configuration page te activate or not the reaction module. */
	/*-----boxes-----*/
	GtkWidget *vboxCheckButton;                 /**< Checkbox used in the configuration page te activate or not the reaction module. */
	GtkWidget *hboxButtonFinish;                /**< VBox layout that contains the finish button in the configuration page. */
	GtkWidget *vboxDebugGeneral;                /**< VBox layout that contains the labels that are in the left column of the general part of the debug page. */
	GtkWidget *vboxDebugSignature;              /**< VBox layout that contains the labels that are in the left column of the signature part of the debug page. */
	GtkWidget *vboxDebugGeneral1;               /**< VBox layout that contains the labels that are in the right column of the general part of the debug page. */
	GtkWidget *vboxDebugSignature1;             /**< VBox layout that contains the labels that are in the right column of the signature part of the debug page. */
	GtkWidget *vboxDebugTextEntryGeneral;       /**< VBox layout that contains the textentrys that are in the left column of the general part of the debug page. */
	GtkWidget *vboxDebugTextEntryGeneral1;      /**< VBox layout that contains the textentrys that are in the right column of the general part of the debug page. */
	GtkWidget *vboxDebugTextEntrySignature;     /**< VBox layout that contains the textentrys that are in the left column of the signature part of the debug page. */
	GtkWidget *vboxDebugTextEntrySignature1;    /**< VBox layout that contains the textentrys that are in the right column of the signature part of the debug page. */
	GtkWidget *hboxDiagnosis;                   /**< HBox layout that contains ??? (seems deprecated). */
	GtkWidget *vboxIndicators;                  /**< VBox layout that contains all the indicators. */
	GtkWidget *vboxIndicatorsLabel;             /**< VBox layout that contains all the indicator labels. */
	/*-----alignements-----*/
    GtkWidget *valignCheckButton;               /**< Alignment of the check buttons in the configure page.*/
    GtkWidget *halignButtonFinish;              /**< Alignment of the finish button in the configure page */
    GtkWidget *valignDebugGeneral;              /**< Alignment of the labels that are in the left column of the general part of the debug page.*/

    GtkWidget *valignDebugSignature;            /**< Alignment of the labels that are in the left column of the signature part of the debug page.*/

    GtkWidget *valignDebugTextEntryGeneral;     /**< Alignment of the textentrys that are in the left column of the general part of the debug page.*/

    GtkWidget *valignDebugTextEntryGeneral1;    /**< Alignment of the textentrys that are in the right column of the general part of the debug page.*/

    GtkWidget *valignDebugTextEntrySignature;   /**< Alignment of the textentrys that are in the left column of the signature part of the debug page.*/

	GtkWidget *valignDebugTextEntrySignature1;  /**< Alignment of the textentrys that are in the right column of the signature part of the debug page.*/
	GtkWidget *valignDebugGeneral1;             /**< Alignment of the labels that are in the right column of the general part of the debug page. */
	GtkWidget *valignDebugSignature1;           /**< Alignment of the labels that are in the right column of the signature part of the debug page. */
	GtkWidget *halignDiagnosis;                 /**< Alignment of the ??? (seems deprecated). */
	GtkWidget *valignIndicators;                /**< Alignment of the indicators textenty into the diagosis page. */
	GtkWidget *valignIndicatorsLabel;           /**< Alignment of the indicators label in the diagnosis page. */
	/*-----tooltips-----*/
	GtkTooltips *tooltipsSaturation;            /**< Tooltips that gives some information about the saturation checkbox usage to the user. */
	GtkTooltips *tooltipsSePoser;               /**< Tooltips that gives some information about the se poer checkbox usage to the user. */
	GtkTooltips *tooltipsSMLimited;             /**< Tooltips that gives some information about the smlimited checkbox usage to the user. */
	GtkTooltips *tooltipsDebug;                 /**< Tooltips that gives some information about the debug checkbox usage to the user. */
	GtkTooltips *tooltipsDisableSSM;            /**< Tooltips that gives some information about the diasablessm checkbox usage to the user. */
	GtkTooltips *tooltipsSys;                   /**< Tooltips that gives some information about the sys checkbox usage to the user. */
	GtkTooltips *tooltipsFile;                  /**< Tooltips that gives some information about the file checkbox usage to the user. */
	GtkTooltips *tooltipsMission;               /**< Tooltips that gives some information about the mission checkbox usage to the user. */
	GtkTooltips *tooltipsReaction;              /**< Tooltips that gives some information about the reaction checkbox usage to the user. */
	/*-----colors-----*/
	GdkColor color;                             /**< Color used for the general informations. */
	GdkColor RED_COLOR;                         /**< Red color used to displays alerts. */
	/*-----event box-----*/
	GtkWidget *eventBoxImgFile;                 /**< Event box used to the file img. */
	/*-----dialogs-----*/
	GtkWidget *dialogFile;                      /**< Dialog used to select a file in debug mode. */
	/*-----scroll window-----*/
	GtkWidget *scrolledAlert;                   /**< Scroller of the alert panel of the diagnosis page. */
	GtkWidget *scrolledEmergency;               /**< Scroller of the emergency panel of the diagnosis page. */
	/*-----buffers-----*/
	GtkTextBuffer* bufferAlert;                 /**< Buffer of the alert panel of the diagnosis page. */
	GtkTextBuffer* bufferEmergency;             /**< Buffer of the emergency panel of the diagnosis page. */
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
