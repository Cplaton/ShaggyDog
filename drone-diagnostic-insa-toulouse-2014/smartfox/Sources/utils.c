/* utils.c */

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


void new_data(FILE * fd, uint32_t timestamp, float roll, float pitch, float yaw, float vx, float vy, float vz) {
	
    fprintf(fd,"%u %f %f %f %f %f %f;\n",timestamp/1000,roll,pitch,yaw,vx,vy,vz);
}


void close_navdata_file(FILE * fd) {
 
    fprintf(fd,"]\n");

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
