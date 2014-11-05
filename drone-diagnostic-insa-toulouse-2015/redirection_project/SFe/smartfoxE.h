/* smartfoxE.h  */

#define HEADER_CONFIG "SF*CONFIG"

#define HEADER_AT_PCMD_CMD "AT*PCMD="

#define HEADER_AT_CMD	"AT*"

#define MAX_PACKET_LENGTH 1024

#define NOMINAL_STATE 0
#define SAFETY_MODE_STATE 1
/*
typedef struct command {
    float roll;
    float pitch;
    float gas;
    float Vyaw;
} command_t;
*/
typedef union _int_or_float {
    float f;
    int i;
} int_or_float;
