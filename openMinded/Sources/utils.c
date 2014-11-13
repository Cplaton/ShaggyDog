/**
 * @file    utils.c
 * @author  ShaggyDogs and Smartfox
 * @brief   Utilitaries functionnalities based on Smartfox's file
 * @version 2.0
 * @date    November 2014
 **/

#include <stdio.h>
#include <utils.h>
#include <stdlib.h>
#include <string.h>

// open the file containing all the data collected from drone
FILE * open_navdata_file(char * name) {

    FILE * fd;
    char fullPath[1024];

    sprintf(fullPath,"%s%c%s%s%c",DATA_DIRECTORY,'F',name,".m",'\0');

    if ((fd = fopen(fullPath,"w+")) == NULL) {
        fprintf(stderr,"Impossible to open the file for collection the data\n");
        perror("=>");
        exit(1);
    }

    fprintf(fd,"%s = [\n",name);

    return fd;


}

// open the file containing all the data collected from drone
FILE * open_csv_file(char * name) {

    FILE * fd;
    char fullPath[1024];

    sprintf(fullPath,"%s%c%s%s%c",DATA_DIRECTORY,'F',name,".csv",'\0');

    if ((fd = fopen(fullPath,"w+")) == NULL) {
        fprintf(stderr,"Impossible to open the file for collection the data\n");
        perror("=>");
        exit(1);
    }
    else{
    fseek(fd,0,2);
    }

    return fd;


}

void new_data_csv(FILE * fd, float alt, float roll, float pitch, float yaw, float vx, float vy, float vz, float ax, float ay, float az) {

    fprintf(fd,"%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n",roll,pitch,yaw,vx,vy,vz);
}


void new_data(FILE * fd, uint32_t timestamp, float roll, float pitch, float yaw, float vx, float vy, float vz) {
	
    fprintf(fd,"%u %f %f %f %f %f %f;\n",timestamp/1000,roll,pitch,yaw,vx,vy,vz);
}


void close_navdata_file(FILE * fd) {
 
    fprintf(fd,"]\n");

    if (fclose(fd) !=0) {
        perror(" ");
    }
}

void close_csv_file(FILE * fd) {

    if (fclose(fd) !=0) {
        perror(" ");
    }
}


FILE * openLogFile(char * name) {

    FILE * fd;
    char fullPath[1024];

    sprintf(fullPath,"%s%s%c",DATA_DIRECTORY,name,'\0');

    if ((fd = fopen(fullPath,"w+")) == NULL) {
        fprintf(stderr,"Impossible to open the file for collection the data\n");
        perror("=>");
        exit(1);
    }
    return fd;
}


void closeLogFile(FILE * fd) {
    if (fclose(fd) !=0) {
        perror(" ");
    }
}