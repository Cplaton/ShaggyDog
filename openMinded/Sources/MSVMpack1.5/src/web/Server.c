#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void HTMLheader(void) {
	printf("Content-Type: text/html;charset=UTF-8\n\n");
	printf("<html>\n");
	printf("<head>\n");
//	printf("<meta http-equiv=\"refresh\" content=\"2; URL=Server.cgi?on=0\">\n\n");
	printf("<link rel=\"stylesheet\" type=\"text/css\" href=\"style.css\"  >\n");
	printf("<title>MSVMpack</title>\n");
	printf("</head>\n");
	printf("<body>\n");
	printf("<h1>MSVMpack Server</h1>");
}

int main(void) {
	char cmd[2000], cmdline[2000];
	char *data = NULL;
	data = getenv("QUERY_STRING");
	char *pstr, *save_ptr, *token;
	char buf[250], file[80];
	char trainfile[400];
	char freeargs[200];
	char outfile[405] = " > training.out &";
	
	double C = 10.0;
	int kernel = 1;
	double kernelpar=1.0;
	
	char msvm[8];
	int nprocs,cachemem;
	char modelfile[400];

	char serverconf[200] = "admin/server.conf";	
	char path_to_data[400] = "Data/";
	char path_to_models[400] = "Models/";
	FILE *fp;
	int alreadytraining = 0;
	
	FILE *fc = fopen(serverconf, "r");
	if (fc != NULL) {
		fscanf(fc,"%s",path_to_data);	
		fscanf(fc,"%s",path_to_models);		
		fclose(fc);		
	}
	
	strcpy(cmd, "./trainmsvm ");
					
	if(data != NULL) {
		pstr = strstr(data, "trainfile=");
		if(pstr != NULL) {
			sscanf(pstr,"trainfile=%[^&]",file);
			sprintf(trainfile," %s%s",path_to_data,file);
			strcat(cmd, trainfile);
		}
		else {
			printf("error no train file\n");
			exit(0);
		}
		
		pstr = strstr(data, "model=");
		if(pstr != NULL) {
			sscanf(pstr,"model=%[^&]",modelfile);
			sprintf(buf," %s%s.model",path_to_models,modelfile);
			strcat(cmd, buf);
		}
				
		pstr = strstr(data, "msvm=");
		if(pstr != NULL) {
			sscanf(pstr,"msvm=%[^&]",msvm);
			sprintf(buf," -m %s",msvm);
			strcat(cmd, buf);
		}
		

		pstr = strstr(data, "Clist=");
		if(pstr != NULL) {
			// Parse list of C values
			pstr += strlen("Clist=");
			sprintf(buf, " -C ");

			do{
				token=strtok_r(pstr, "+&", &save_ptr);
				if(token != NULL && *token != '\0' && ( (*token >= '0' && *token <= '9') || *token == '.')) {
					//sscanf(token,"%lf",&C );
					strcat(buf, token);
					strcat(buf, " ");
				}
				pstr = NULL;
			} while(token != NULL && ( (*token >= '0' && *token <= '9') || *token == '.'));	
		
			strcat(cmd, buf);
		}
		else {
			pstr = strstr(data, "C=");
			if(pstr != NULL) {
				sscanf(pstr,"C=%lf",&C);
				sprintf(buf," -c %lf",C);
				strcat(cmd, buf);
			}
		}
		
		pstr = strstr(data, "kernel=");
		if(pstr != NULL) {
			sscanf(pstr,"kernel=%d",&kernel);
			sprintf(buf," -k %d",kernel);
			strcat(cmd, buf);
		}
		
		if(kernel != 1) {
			pstr = strstr(data, "kernelpar=");
			if(pstr != NULL) {
				sscanf(pstr,"kernelpar=%lf",&kernelpar);
				sprintf(buf," -p %lf",kernelpar);
				strcat(cmd, buf);
			}
		}
		
		pstr = strstr(data, "normalization=on");
		if(pstr != NULL) 
			sprintf(buf," -n");
		else
			sprintf(buf," -u");
		strcat(cmd, buf);
						
		pstr = strstr(data, "cachemem=");
		if(pstr != NULL) {
			sscanf(pstr,"cachemem=%d",&cachemem);
			sprintf(buf," -x %d",cachemem);
			strcat(cmd, buf);
		}
		
		pstr = strstr(data, "nprocs=");
		if(pstr != NULL) {
			sscanf(pstr,"nprocs=%d",&nprocs);
			sprintf(buf," -t %d",nprocs);
			strcat(cmd, buf);
		}
		
		pstr = strstr(data, "freeargs=");
		if(pstr != NULL) {
			freeargs[0] = '\0';
			sscanf(pstr,"freeargs=%[^&]", freeargs);
			pstr = freeargs;
			sprintf(buf," ");
			do{
				token=strtok_r(pstr, "+&", &save_ptr);
				if(token != NULL && *token != '\0') {
					strcat(buf, token);
					strcat(buf, " ");
				}
				pstr = NULL;
			} while(token != NULL);	
		
			strcat(cmd, buf);
		}	
		
	}

	// Check if another training is going on with the same model name
	sprintf(outfile, "%s.model.training",modelfile);	
	if( (fp = fopen (outfile, "r") ) != NULL) {
		alreadytraining = 1;
		fclose(fp);			
	}
	else {
		// start training
		strcpy(cmdline,cmd);	
		sprintf(outfile, " > %s.model.training &",modelfile);	
		strcat(cmd, outfile);

		system(cmd);
	}
	
	// HTML output
	HTMLheader();
	if(alreadytraining) {
		printf("%s.model cannot be trained: <br>a model with the same name is currently being trained.<br> \n", modelfile);	
		printf("Go <a href=\"home.cgi?trainfile=%s\">back to homepage</a>.\n",file);
	}	
	else {
		printf("Training started.<br>\n");
		printf("<a href=\"result.cgi?trainfile=%s&modelfile=%s.model&pathmodels=%s\">WATCH</a>",file,modelfile,path_to_models);
		printf(" or <a href=\"stop.cgi?trainfile=%s&modelfile=%s.model&pathmodels=%s&done=no\">CANCEL</a>",file,modelfile,path_to_models);
		printf("<br><hr size=1 width=100\%% align=center>\n");
		printf("MSVMpack Command-line:\n <pre> %s </pre>\n",cmdline);
	}
	printf("</body></html>\n");
	
}
