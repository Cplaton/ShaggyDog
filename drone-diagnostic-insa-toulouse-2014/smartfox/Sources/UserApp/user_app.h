/* 
 * user_app.h 
 * 13/01/2014 - gayraudbenoit@gmail.com
 */
#ifndef USER_APP_H__
#define USER_APP_H__
#include <linux/input.h>
#include <VP_Api/vp_api_thread_helper.h> // api pour les threads

PROTO_THREAD_ROUTINE(th_user_app, data);

/* ############################################################################################################## */

/* configuration file containing the keyboard file */
#define CONF_FILE "./user_app.conf"

/* drone motions and keyboard keys association*/
#define GO_FORWARD 	KEY_W
#define GO_BACK 	KEY_S
#define GO_LEFT		KEY_A
#define GO_RIGHT	KEY_D
#define GO_UP		KEY_UP
#define GO_DOWN		KEY_DOWN
#define ROTATE_LEFT	KEY_LEFT
#define ROTATE_RIGHT	KEY_RIGHT 
#define TAKEOFF		KEY_T
#define LANDING		KEY_L
#define KILL		KEY_K

#define LOW_VELOCITY	0.3


/* ############################################################################################################## */


#endif
