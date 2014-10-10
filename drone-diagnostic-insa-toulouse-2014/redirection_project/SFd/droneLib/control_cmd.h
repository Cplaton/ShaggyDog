/* control_cmd.h */

/*
 * Build an AT command message to control the drone motion
 * AT_cmd     : the string that will contain the AT command expected
 * seq_nb     : the sequence number of the packet 
 * other args : motion parameters (see SDK doc) - must be between -1 and 1 (if not : set to 0)
 */
void motion_command (char * AT_cmd, int seq_nb, float roll, float pitch, float gaz, float yaw);

/*
 * Build an AT command message to make the drone lands
 * AT_cmd     : the string that will contain the AT command expected
 * seq_nb     : the sequence number of the packet 
 */
void landing_command (char * AT_cmd, int seq_nb);

/*
 * Build an AT command message to make the drone takes off
 * AT_cmd     : the string that will contain the AT command expected
 * seq_nb     : the sequence number of the packet 
 */
void takeoff_command (char * AT_cmd, int seq_nb);
