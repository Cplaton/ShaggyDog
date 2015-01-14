/* 
 * smartfox_api.h 
 * 13/01/2014 - gayraudbenoit@gmail.com
 */

/**
 * @file    smartfox_api.h
 * @author  Benoit Gayraud
 * @brief   Contains the function that refreshes the command to send and the faults getter (for smartfox diagnosis)
 * @date    January 2014
 **/

#ifndef SF_DRONE_COMMAND_H__
#define SF_DRONE_COMMAND_H__
#include "../ardrone_move_cmd.h"
#include "../Model/residue.h"
/* -------------- NAVIGATION function ---------------- */

/*
 * To refresh the command to send to the drone
 * newCommand : the flight parameter of the command (yaw, pitch...)
 * type : the command type to land, take...
 * NOTE : this fonction does nothing if the drone is in safety mode
 */
void set_command (Inputs_t * newCommand , commandType_t type);


/* ---------------- DIAGNOSIS functions ----------------- */

/* 
 * Gives the last fault detected by the diagnoser
 * fault : out parameter, must not be NULL
 */
void getFault(fault_t *fault);

/*
 * Gives the last signature established by the diagnoser
 * signature : out parameter, must not be NULL
 */
void getSignature(alarm_t * signature);

/*
 * Gives the last residues computed (= Difference between the mathematical model
 * outputs and the real values given by the drone)
 *
 * parameters:
 * in model       : mathematical model outputs
 * in real        : new drone data values
 * out residue    : difference between model and real
 */
void getResidue(Navdata_t * model, Navdata_t * real, Residue_t * residue );


#endif
