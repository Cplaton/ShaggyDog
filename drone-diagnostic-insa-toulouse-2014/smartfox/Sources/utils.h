#include <ardrone_tool/ardrone_tool.h>

#ifndef _UTILS_H
#define _UTILS_H


#define DATA_DIRECTORY "./DataModel/"
#define TMP_RECORD_FILE_NAME ".tmp_record"

typedef struct _move_command {
    int record_nb;
    float roll;
    float pitch;
    float yaw;
    float gas;
} move_command;


// open the file containing all the data collected from drone
FILE * open_navdata_file(char * name);

FILE * open_csv_file(char * name);

void new_data(FILE * fd,uint32_t timestamp, float roll, float pitch, float yaw, float vx, float vy, float vz);

void new_data_csv(FILE * fd, float alt, float roll, float pitch, float yaw, float vx, float vy, float vz, float ax, float ay, float az);

void close_navdata_file(FILE * fd);

void close_csv_file(FILE * fd);

FILE * openLogFile(char * name);

void closeLogFile(FILE * fd) ;


#endif
