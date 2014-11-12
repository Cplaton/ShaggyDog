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
	printf("MSVMpack Server version %1.1f\n",VERSION);
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
	int cachemem = 2048;
		
	
	double C = 10.0;
	int kernel = 1;
	FILE *fp;
	int maxnprocs = get_nprocessors(); 
	int nprocs = maxnprocs;
	int i;
	int printinfo = 0;
	int config = 0;
	
	// Table variables
	int ncols = 4;
	int col = 0;
	
	FILE *fc = fopen(serverconf, "r");
	if (fc == NULL) {
		// Conf file does not exist yet
		config = 1;
		/*
		fp = popen("cd ../Data; pwd","r");
		fscanf(fp,"%s",path_to_data);
		pclose(fp);
		fp = popen("cd ../Models; pwd","r");
		fscanf(fp,"%s",path_to_models);
		pclose(fp);
		*/
	}
	
	HTMLheader();
	
	if( !config) {
		// Menu links
		printf("<a href=\"../home.cgi\">HOME</a>");
	}
	printf("<br><hr size=1 width=100\%% align=center>\n");
	

        
	if (!config) {
		fscanf(fc,"%s",path_to_data);	
		fscanf(fc,"%s",path_to_models);		
		fscanf(fc,"%d",&cachemem);		
		fscanf(fc,"%d",&nprocs);
		fclose(fc);
		
		// FORM
		printf("<h2 id=\"admin\"> Files</h2>\n");
	
		// Training file
		printf("\n <h3>Data</h3>\n");
	
		printf("<FORM ACTION=\"delete.cgi\" METHOD=\"GET\"><DIV>\n");
	
		printf("<table> <tr>");	// Table
		
		sprintf(cmd,"ls -c -1 ../%s",path_to_data);
		fp = popen(cmd,"r");
		i = 0;
		while(fgets(buf,80,fp) != NULL) {
			strncpy(file,buf,strlen(buf)-1);
			file[strlen(buf)-1] = '\0';
			
			if(col < ncols) {
				printf("<td style=\"font-size: 80\%%;\">\n");
				col++;
			}
			else {
				printf("<tr><td style=\"font-size: 80\%%;\">");
				col = 1;
			}
			printf("<INPUT ID=\"datafile%d\" NAME=\"%s\" type=checkbox> <a href=\"../%s%s\"> %s</a>\n",i,file,path_to_data,file,file);
			
			printf("</td>\n");
			if (col >= ncols)
				printf("</tr>\n");
			
			i++;
		}
		pclose(fp);
		if(col < ncols)
			printf("</tr>");
		printf("</table>\n");
		
		printf("<p>Action: <select name=\"action\">\n");
	  	  printf("<option value=\"deletedata\">Delete</option>\n");
	    	 // printf("<option value=\"download\">Download</option>\n");
		printf("</select>\n");
	
		printf("<INPUT TYPE=\"SUBMIT\" VALUE=\" Go \" ></p></FORM>\n");
	
	
		// --- Models
		printf("<h3>Models</h3> \n");
	
		
		printf("<FORM ACTION=\"delete.cgi\" METHOD=\"GET\">\n");

		printf("<table> <tr>");	// Table
		
		sprintf(cmd,"ls -c -1 ../%s",path_to_models);
		fp = popen(cmd,"r");
		i=0;
		while(fgets(buf,80,fp) != NULL) {
			strncpy(file,buf,strlen(buf)-1);
			file[strlen(buf)-1] = '\0';
			
			if(col < ncols) {
				printf("<td style=\"font-size: 80\%%;\">\n");
				col++;
			}
			else {
				printf("<tr><td style=\"font-size: 80\%%;\">");
				col = 1;
			}			
			printf("<INPUT ID=\"modelfile%d\" NAME=\"%s\" type=checkbox> <a href=\"../%s%s\"> %s</a>\n",i,file,path_to_models,file,file);
			printf("</td>\n");
			if (col >= ncols)
				printf("</tr>\n");
			i++;
		}
		pclose(fp);
		if(col < ncols)
			printf("</tr>");
		printf("</table>\n");
		
		printf("</p><p>Action: <select name=\"action\">\n");
	  	  printf("<option value=\"deletemodels\">Delete</option>\n");
	    //	  printf("<option value=\"download\">Download</option>\n");
		printf("</select>\n");
	
		printf("<INPUT TYPE=\"SUBMIT\" VALUE=\" Go \" ></p></FORM>\n");
				
	}
	
	// --- Settings
	printf("<h2>Settings</h2>\n");	
	printf("<FORM ACTION=\"settings.cgi\" METHOD=\"GET\"><DIV>\n");

	// Paths
	printf("<h3>File paths</h3> \n");
	
//	printf("<p>Path to data repository: <input type=\"file\" name=\"pathdata\" value=\"%s\"></input>",path_to_data); 
	printf("<p>Path to data repository: <INPUT  NAME=\"pathdata\" SIZE=\"60\" MAXLENGTH=\"399\" value=\"%s\"></p>\n",path_to_data);	
	printf("<p>Path to model files: <INPUT NAME=\"pathmodels\" SIZE=\"63\" MAXLENGTH=\"399\" value=\"%s\"></p>",path_to_models);

	printf("<h3>Ressources</h3> \n");
	
	// cache memory
	printf("<p>Default memory for kernel cache: <INPUT NAME=\"cachemem\" SIZE=\"6\" MAXLENGTH=\"10\" value=\"%d\" style=\"text-align:right;\"> MB </p>\n",cachemem);
	
	// nprocs
	printf("<p>Max. number of processors: <select id=\"nprocs\" name=\"nprocs\">\n");
	for(i=1;i<=maxnprocs;i++) {
		if(i == nprocs)
			printf("<option value=\"%d\" selected=\"selected\">%d</option>\n",nprocs,nprocs);
		else
			printf("<option value=\"%d\">%d</option>\n",i,i);
	}
	printf("</select></p>\n");
		
	
	if(!config)
		printf("<INPUT TYPE=\"SUBMIT\" VALUE=\" Save settings \" ></FORM>\n");
	else
		printf("<INPUT TYPE=\"SUBMIT\" VALUE=\" Save settings and start using MSVMpack server \" ></FORM>\n");
	
	
	// FORM 
	
	printf("<FORM ACTION=\"passwd.cgi\" METHOD=\"GET\"><DIV>\n");
	
	printf("<h2 id=\"adminpasswd\">Administrator password </h2>\n");
	printf("<p>Password: <INPUT type=\"PASSWORD\" NAME=\"passwd\" SIZE=\"20\" MAXLENGTH=\"20\" value=\"\">\n");
	
	printf("<INPUT TYPE=\"SUBMIT\" VALUE=\"  Change password \" ></p></DIV></FORM>\n");
	
	HTMLfooter();
	
}
