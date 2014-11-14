/* 
 * user_app.h 
 * 13/01/2014 - gayraudbenoit@gmail.com
 */
#ifndef USER_APP_H__
#define USER_APP_H__
#include <linux/input.h>
#include <VP_Api/vp_api_thread_helper.h> // api pour les threads

PROTO_THREAD_ROUTINE(th_user_app, data);

/* to make the drone hovers over the floor*/
void hovering();

/* to send a specifing command to the drone while it is flying */
void apply_command(float roll, float pitch, float Vyaw, float gas);

/* to make the drone lands */
void landing();

/* to make the drone takes off */
void takeoff();

/* initialization function : reads the conf file */
void init_userapp(char * keyboard_file, size_t len);

/* extract the key that triggered the event and send the matching command to the drone */
void extract_key_event(struct input_event * ev);

/* ############################################################################################################## */

/* configuration file containing the keyboard file */
#define CONF_FILE "./Sources/UserApp/user_app.conf"

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
#define CLASS_WALL  KEY_N

#define LOW_VELOCITY	0.3


/* ############################################################################################################## */


#endif
