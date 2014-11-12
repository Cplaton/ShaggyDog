/**
 * @file    utils.h
 * @author  ShaggyDogs and Smartfox
 * @brief   Utilitaries functionnalities based on Smartfox's file
 * @version 2.0
 * @date    November 2014
 **/

#include <ardrone_tool/ardrone_tool.h>

#ifndef _UTILS_H
#define _UTILS_H


#define DATA_DIRECTORY "./DataModel/"
#define TMP_RECORD_FILE_NAME ".tmp_record"

/**
 * @struct	_move_command
 * @abstract	Struct containing the information that should be sent to the drone in order to command it
 **/
typedef struct _move_command {
    int record_nb;  /**< ??. */
    float roll;     /**< roll to set to the drone. */
    float pitch;    /**< pitch to set to the drone. */
    float yaw;      /**< yaw to set to the drone. */
    float gas;      /**< gas to set to the motors. */
} move_command;

/**
 * @brief   Opens in writing mode a new navdata file with the given name
 * @param   name    Name of the file where the navdata would be stored
 * @return  The opened navdata file ready to be filled.
 **/
FILE * open_navdata_file(char * name);

/**
 * @brief   Opens in writing mode a new csv file with the given name
 * @param   name    Name of the file CSV file to open
 * @return  The opened csv file ready to be filled.
 **/
FILE * open_csv_file(char * name);


/**
 * @brief   Write a new line into simple navdata format into the transmetted file
 * @param   fd      the file in wich the data would be inserted
 * @param   roll    the roll to be stored
 * @param   pitch   the pitch to be stored
 * @param   yaw     the yaw to be stored
 * @param   vx      the x speed to be stored
 * @param   vy      the y speed to be stored
 * @param   vz      the z speed to be stored
 **/
void new_data(FILE * fd,uint32_t timestamp, float roll, float pitch, float yaw, float vx, float vy, float vz);

/**
 * @brief   Write a new line into CSV format into the transmetted file
 * @param   fd      the file in wich the data would be inserted
 * @param   alt     the altitude to be stored
 * @param   roll    the roll to be stored
 * @param   pitch   the pitch to be stored
 * @param   yaw     the yaw to be stored
 * @param   vx      the x speed to be stored
 * @param   vy      the y speed to be stored
 * @param   vz      the z speed to be stored
 * @param   ax      the x acceleration to be stored
 * @param   ay      the y acceleration to be stored
 * @param   az      the z acceleration to be stored
 **/
void new_data_csv(FILE * fd, float alt, float roll, float pitch, float yaw, float vx, float vy, float vz, float ax, float ay, float az);

/**
 * @brief   Closes the tansmetted navdata file
 * @param   fd    The csv file to be closed
 **/
void close_navdata_file(FILE * fd);

/**
 * @brief   Closes the tansmetted csv file
 * @param   fd    The csv file to be closed
 **/
void close_csv_file(FILE * fd);

/**
 * @brief   Opens in writing mode a new log file with the given name
 * @param   name    Name of the file where the logs would be stored
 * @return  The opened log file ready to be filled.
 **/
FILE * openLogFile(char * name);

/**
 * @brief   Closes the tansmetted log file
 * @param   fd    The log file to be closed
 **/
void closeLogFile(FILE * fd) ;


#endif
