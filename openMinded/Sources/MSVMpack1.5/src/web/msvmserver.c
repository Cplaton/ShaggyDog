#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>

#ifndef   NI_MAXHOST
#define   NI_MAXHOST 1025
#endif

#ifndef DIR
#define DIR "./"
#endif

void printaddr(int port)
{
    struct ifaddrs *ifaddr, *ifa;
           int family, s;
           char host[NI_MAXHOST];
           char host_numeric[NI_MAXHOST];
           
           if (getifaddrs(&ifaddr) == -1) {
               perror("getifaddrs");
               exit(EXIT_FAILURE);
           }
           
           printf("MSVMpack Server is now available at \n\n");

           /* Walk through linked list, maintaining head pointer so we
              can free list later */

           for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
               family = ifa->ifa_addr->sa_family;
	
               if (family == AF_INET) {
                   s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),host, NI_MAXHOST, NULL, 0, 0);
                    
                   if (s != 0) {
                       printf("getnameinfo() failed: %s\n", gai_strerror(s));
                       exit(EXIT_FAILURE);
                   }
	            
	           if (port != 80)       
	                   printf("  http://%s:%d/", host,port);
	           else
	           	   printf("  http://%s/", host);
	           	   
                   if(host[0] < '0' || host[0] > '9') {
                     	s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),host_numeric, NI_MAXHOST, NULL, 0, NI_NUMERICHOST); 
                     	if (port != 80) 
			               	printf("\n\t ( http://%s:%d/ )", host_numeric,port);
		               	else
			               	printf("\n\t ( http://%s/ )", host_numeric);
                   }

               	printf("\n");
                  
               }
           }
		
	printf("\n");
	freeifaddrs(ifaddr);

    return;
}

void printusage(void) {
	printf("Usage:\n");
	printf("\t msvmserver start [port]\n");
	printf("\t msvmserver restart [port]\n");	
	printf("\t msvmserver stop\n");
	printf("\t msvmserver status\n\n");		
	printf(" By default msvmserver will use port 8080.\n\n");
	return;	
}

void start(int port, char *argv[]) {
	FILE *fp;
	char pidfile[400];
	char cmd[200];
	char homedir[400];
	char mongoose[600];
	int pid;
	uid_t userID = getuid();
	
	
	sprintf(pidfile,"%s/msvmserver.pid",DIR);
	sprintf(homedir,"%s/webpages",DIR);	
	sprintf(mongoose,"%s/mongoose/mongoose",DIR);	
	sprintf(cmd, "%d",port);
	
	if(port < 1024) {
		if(userID != 0 ) {
			printf("You need to be root to use port %d.\n" , port);
			printf("Try 'sudo %s start %d' to get the permission to do that.\n",argv[0],port); 
			exit(0);
		}
	}
	
	printf("Listening on port %d...\n",port);
	
	daemon(1,1);
	pid = getpid();
	fp = fopen(pidfile,"w");
	if( fp == NULL) {
		printf("Error: cannot write PID file.\n");
		exit(1);
	}
	fprintf(fp,"%d\n",pid);
	fclose(fp);
	
	printaddr(port);
	
	execlp(mongoose,mongoose,"-d","no","-p",cmd,"-r",homedir,(char *)NULL);

}
/*
	Check if PID in pidfile is correct
	
	Returns
	0 : PID in pidfile is OK
	1 : no pidfile
	2 : no process with this PID 
	3 : no permission on the process with this PID 
*/
int checkpid(char *pidfile, int *pid) {

	int check;

	FILE *fp = fopen(pidfile, "r");
	if( fp == NULL) {
		check = 1; // No PID file
	}
	else {
		fscanf(fp,"%d",pid);
		fclose(fp);
	
		if( kill(*pid, 0) != 0) {
			if(errno == ESRCH) {
				/*  Wrong PID file, probably due to
				    a server stop unhandled by msvmserver
				*/
				check = 2;
				remove(pidfile);
			}
			else {
				check = 3;
			}
		}
		else {
			check = 0; // PID OK (is a running process)
		}
	}

	return check;
}

int main(int argc, char *argv[]) {

	int port = 8080;
	int pid;
	char pidfile[400];
	int pidcheck;
	FILE *fp;	
		
	if(argc < 2) {
		printusage();
		exit(0);
	}
	
	sprintf(pidfile,"%s/msvmserver.pid",DIR);
	
	if(strcmp(argv[1],"start") == 0) {
		
		pidcheck = checkpid(pidfile, &pid); 
		
		switch(pidcheck) {
			case 0: 
				printf("MSVMpack Server is already running.\n");		
				break;
			case 1: 
				printf("Starting MSVMpack Server...\n");
				break;				
			case 2: 
				printf("Starting MSVMpack Server...\n");
				break;			
			case 3: 
				printf("MSVMpack Server is already running (started by another user).\n");
				printf("Try 'sudo %s stop' to stop this instance before starting a new one.\n",argv[0]);
				exit(0);
				break;				
			default:
				break;
		}
		

		if (argc < 3) {
			printf("Will use port 8080 (use 'msvmserver start n' to use port n)\n");
		}
		else {
			port = atoi(argv[2]);
		}

		start(port,argv);
		
	}
	else if (strcmp(argv[1],"stop") == 0) {
	
		pidcheck = checkpid(pidfile, &pid); 
		
		switch(pidcheck) {
			case 0: 
				printf("MSVMpack Server is going to stop...\n");
				kill(pid, SIGTERM);
				
				// Kill the process if it did not terminate
				sleep(1);
				if( kill(pid, 0) == 0) {
					kill(pid, SIGKILL);
				}
				remove(pidfile);
				break;
			case 1: 
				printf("MSVMpack Server is not running.\n");
				break;				
			case 2: 
				printf("MSVMpack Server is not running.\n");
				break;			
			case 3: 
				printf("Cannot stop the MSVMpack Server started by another user.\n");
				printf("Try 'sudo %s stop' to get the permission to do that.\n",argv[0]);
				break;				
			default:
				break;
		}
	}
	else if (strcmp(argv[1],"status") == 0) {
		
		pidcheck = checkpid(pidfile, &pid); 
		
		switch(pidcheck) {
			case 0: 
				printf("MSVMpack Server is running (pid %d).\n",pid);
				break;
			case 1: 
				printf("MSVMpack Server is stopped.\n");
				break;				
			case 2: 
				printf("MSVMpack Server is stopped (probably killed by another program).\n");
				break;				
			case 3: 
				printf("MSVMpack Server is running (started by another user).\n");
				printf("Try 'sudo %s {stop|restart}' to take control.\n",argv[0]);
				break;				
			default:
				break;
		}
		
	}
	else if (strcmp(argv[1],"restart") == 0) {
	
		pidcheck = checkpid(pidfile, &pid); 
		
		switch(pidcheck) {
			case 0: 
				printf("MSVMpack Server is going to restart...\n");
				kill(pid, SIGTERM);
				remove(pidfile);
		
				break;
			case 1: 
				printf("MSVMpack Server was not running, starting now.\n");
				break;				
			case 2: 
				printf("MSVMpack Server was not running, starting now.\n");
				break;			
			case 3: 
				printf("Cannot stop the MSVMpack Server started by another user.\n");
				printf("Try 'sudo %s restart' to get the permission to do that.\n",argv[0]);
				exit(0);
				break;				
			default:
				break;
		}
		
		if (argc < 3) {
			printf("Will use port 8080 (use 'msvmserver [re]start n' to use port n)\n");
		}
		else {
			port = atoi(argv[2]);
		}

		start(port,argv);

	}
	else {
		printf("Unknown command '%s'.\n\n",argv[1]);
		printusage();
	}

	return 0;
}
