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

FILE * open_learning_file(char * name) {

    FILE * fd;
    char fullPath[1024];

    sprintf(fullPath,"%s%s%c","./Sources/Navdata/",name,'\0');

    if ((fd = fopen(fullPath,"w+")) == NULL) {
        fprintf(stderr,"Impossible to open the file for collection the data\n");
        perror("=>");
        exit(1);
    }
    return fd;

}

FILE * open_online_file(char * name) {

    FILE * fd;
    char fullPath[1024];

    sprintf(fullPath,"%s%s%s%c","./Navdata/tmp/",name,".txt",'\0');

    if ((fd = fopen(fullPath,"w+")) == NULL) {
        fprintf(stderr,"Impossible to open the file for collection the data\n");
        perror("=>");
        exit(1);
    }
    return fd;
}

void new_data_learning(FILE * fd, int classe, float roll, float pitch, float yaw, float vx, float vy, float vz, float ax, float ay, float az) {

    fprintf(fd,"%d 1:%f 2:%f 3:%f 4:%f 5:%f 6:%f 7:%f 8:%f 9:%f \n", classe, roll,pitch,yaw,vx,vy,vz,ax,ay,az);
}

void new_data_csv(FILE * fd, float alt, float roll, float pitch, float yaw, float vx, float vy, float vz, float ax, float ay, float az) {

    fprintf(fd,"%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n",alt,roll,pitch,yaw,vx,vy,vz,ax,ay,az);
}

void new_data_online(FILE * fd, float roll, float pitch, float yaw, float vx, float vy, float vz, float ax, float ay, float az) {

    fprintf(fd,"%f,%f,%f,%f,%f,%f,%f,%f,%f\n",roll,pitch,yaw,vx,vy,vz,ax,ay,az);
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

void close_learning_file(FILE * fd) {

    if (fclose(fd) !=0) {
        perror(" ");
    }
}

void close_online_file(FILE * fd) {

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

