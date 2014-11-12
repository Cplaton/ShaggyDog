
#include <stdio.h>
#include <string.h>
void HTMLheader(void) {
	printf("Content-Type: text/html;charset=UTF-8\n\n");
	printf("<html>\n");
	printf("<head>\n");
	printf("<link rel=\"stylesheet\" type=\"text/css\" href=\"style.css\"  >\n");
	printf("<title>MSVMpack</title>\n");
	printf("</head>\n");
	printf("<body>\n");
	printf("<h1>MSVMpack Server</h1>");
}

int main(void)
{
	FILE * fp, *fc;
	char tmp[1024];
	char buf[10];
	char filename[80];
	char fullfilename[500]; 
	char path_to_data[400] = "Data/";
	char path_to_models[400] = "Models/";
	
	int notdone = 1;
	char *rc;
	int rcf;
	char *pstr = NULL;

	HTMLheader();
	
	// Load server configuration
	fc = fopen("admin/server.conf","r");
	if(fc != NULL) {
		rcf = fscanf(fc,"%s",path_to_data);	
		rcf = fscanf(fc,"%s",path_to_models);		
		fclose(fc);
	}

	fgets(tmp,1024,stdin);
	while((pstr = strstr(tmp,"filename=")) == NULL) {
		fgets(tmp,1024,stdin);
//		printf("%s<br>\n",tmp);
	}
	sscanf(pstr,"filename=\"%[^\"]",filename);
	
	sprintf(fullfilename,"%s%s",path_to_data,filename);

	printf("Upload data file: %s.<br>\n",filename);
	printf("<a href=\"home.cgi\">Back to Homepage</a>");
	printf("<br><hr size=1 width=100\%% align=center>\n");
	
	fp = fopen(fullfilename, "w" );


	fgets(tmp,1024,stdin);	
	while((pstr = strstr(tmp,"application/octet-stream")) == NULL) {
//		printf("%s<br>\n",tmp);
		fgets(tmp,1024,stdin);
	}
	fgets(tmp,1024,stdin);

	while(notdone) {
		rc = fgets(tmp,1024,stdin);
		if(rc != NULL)
			strncpy(buf,tmp,6);
		if(rc == NULL || (strncmp(buf,"------",6) == 0)) {
			notdone = 0;
		}
		else {
			//printf("%s<br>", tmp); BREAKS for large files
			fprintf(fp,"%s",tmp);

		}
	}
	fclose(fp);
	printf("<p>File upload done.</p>\n" );
	
	printf("</body></html>\n");
	return 0;
}
