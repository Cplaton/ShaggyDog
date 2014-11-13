#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifndef _SC_NPROCESSORS_ONLN
#include <sys/sysctl.h>		// for BSD
#endif


/*
	Return the number of available processors
*/
int get_nprocessors(void) {
	int nprocs;
	
#ifdef _SC_NPROCESSORS_ONLN	// Linux

	nprocs = (int)sysconf(_SC_NPROCESSORS_ONLN);

#else				// BSD
	int mib[4];
	size_t len = sizeof(nprocs); 

	mib[0] = CTL_HW;
#ifdef HW_AVAILCPU
	mib[1] = HW_AVAILCPU;  
#else
	mib[1] = HW_NCPU;
#endif	
	sysctl(mib, 2, &nprocs, &len, NULL, 0);
#endif

	// Make sure nprocs >= 1
	if( nprocs < 1 ) 
		nprocs = 1;
	
	return nprocs;

}

void HTMLheader(void) {
	printf("Content-Type: text/html;charset=UTF-8\n\n");
	printf("<html>\n");
	printf("<head>\n");
	printf("<link rel=\"stylesheet\" type=\"text/css\" href=\"styleadmin.css\" >\n");
	printf("<title>MSVMpack Server</title>\n");
	printf("</head>\n");
	printf("<body>\n");
	printf("<h1>MSVMpack Server</h1>\n");
}
void HTMLfooter(void) {
	printf("<hr size=1 width=100\%% align=center color=#900000>\n");
	printf("<div id=\"footer\">\n");
	printf("MSVMpack Server version 1.1, last modified: Nov. 2011\n");
	printf("</div>\n");
	printf("</body></html>\n");
}

int main(void) {
	char cmd[1000];
	char *data = NULL;
	data = getenv("QUERY_STRING");
	char *pstr;
	char buf[80], file[80], modeltest[80];
	char trainfile[200];
	
	// Default options
	char serverconf[200] = "./server.conf";
	char path_to_data[400] = "Data/";
	char path_to_models[400] = "Models/";
	int rc;
		
	enum {DELETEDATA,DELETEMODELS} action;
	char actionstr[20];
		
	FILE *fc = fopen(serverconf, "r");
	if (fc != NULL) {
		fscanf(fc,"%s",path_to_data);	
		fscanf(fc,"%s",path_to_models);		
		fclose(fc);		
	}
	
	HTMLheader();
	
	if(data != NULL) {
	
		// Which action?
		pstr = strstr(data, "action=");
		sscanf(pstr,"action=%[^&]",actionstr);
		if( strcmp(actionstr, "deletedata") == 0)
			action = DELETEDATA;
		else if ( strcmp(actionstr, "deletemodels") == 0)
			action = DELETEMODELS;
					
		// Get file names
		pstr = data;

		while( pstr < data+strlen(data) - strlen("action=") - strlen(actionstr) ) { 
			sscanf(pstr,"%[^=&]",file);

			switch (action) {
				case  DELETEDATA:
					sprintf(cmd,"../%s%s",path_to_data,file);
				break;
				case DELETEMODELS:
					sprintf(cmd,"../%s%s",path_to_models,file);			
				break;
				default:
				break;
			}
			
			rc = remove(cmd);
			if(rc) 
				printf("Cannot remove file %s<br>\n",cmd+3); // +3 not to print '../'
			else
				printf("Removed %s<br>\n",cmd+3);

			pstr += strlen(file) + strlen("=on&");
		}
	}
	
	printf("<p>Back to <a href=\"../home.cgi\">MSVMpack server homepage</a>\n");
	printf("or the <a href=\"admin.cgi\">admin page</a></p>\n");
	
	HTMLfooter();
	
}
