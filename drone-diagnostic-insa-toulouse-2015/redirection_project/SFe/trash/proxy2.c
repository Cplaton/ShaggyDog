#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>

int main( int argc, char *argv[] ) {

    int sockfd, newsockfd, portno, clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr, th_cmd_addr;
    int  n;
    socklen_t len;

    /* First call to socket() function */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) 
    {
        perror("ERROR opening socket");
        exit(1);
    }
    /* Initialize socket structure */
    memset((char *) &serv_addr,0, sizeof(serv_addr));
    portno = 15000;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    memset(&th_cmd_addr,0,sizeof(th_cmd_addr));
    portno = 5556;
    th_cmd_addr.sin_family = AF_INET;
    th_cmd_addr.sin_addr.s_addr = INADDR_ANY;
    th_cmd_addr.sin_port = htons(portno);
 
    /* Now bind the host address using bind() call.*/
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
                          sizeof(serv_addr)) < 0)
    {
         perror("ERROR on binding");
         exit(1);
    }

    
    len = sizeof(serv_addr);

    while (1)
    {
      n = recvfrom(sockfd,buffer,sizeof(buffer),0,(struct sockaddr *)&serv_addr,&len);
      //printf("-------------------------------------------------------\n");
      //printf("%s\n",buffer);
      //printf("-------------------------------------------------------\n");
      sendto(sockfd,buffer,sizeof(buffer),0,(struct sockaddr *)&th_cmd_addr,(socklen_t)sizeof(th_cmd_addr));
    }

    return 0;
}
