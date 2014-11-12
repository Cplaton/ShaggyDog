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
	printf("<link rel=\"stylesheet\" type=\"text/css\" href=\"style.css\" >\n");
	printf("<script type=\"text/javascript\" src=\"home.js\"></script>\n");
	printf("<title>MSVMpack Server</title>\n");
	printf("</head>\n");
	printf("<body>\n");
	printf("<h1>MSVMpack Server</h1>\n");
}
void HTMLfooter(void) {
	printf("<hr size=1 width=100\%% align=center color=#006090>\n");
	printf("<div id=\"footer\">\n");
	printf("MSVMpack Server version 1.3\n");
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
	
	double C = 10.0;
	int kernel = 1;
	FILE *fp, *fc;
	int nprocs = get_nprocessors(); 
	int cachemem = 2048;
	int i;
	int printinfo = 0;
	int printdatainfo = 0;
	long nb_data = 0;
	long dim = 0;
	char serverconf[200] = "admin/server.conf";
	char path_to_data[400] = "Data/";
	char path_to_models[400] = "Models/";
	int rc;
	
	HTMLheader();
	
	// Load server configuration
	fc = fopen("admin/server.conf","r");
	if(fc == NULL) {
	
		printf("Please go to the <a href=\"admin/admin.cgi\">admin page</a> to setup the server before the first use.<br>\n"); 
	
		HTMLfooter();
		exit(0);
	}
	else {
		rc = fscanf(fc,"%s",path_to_data);	
		rc = fscanf(fc,"%s",path_to_models);		
		rc = fscanf(fc,"%d",&cachemem);		
		rc = fscanf(fc,"%d",&nprocs);
		fclose(fc);
	}
	

	// Menu links
	printf("<a href=\"home.cgi#upload\">DATA</a>");
	printf(" -- <a href=\"home.cgi#train\">TRAIN</a>");
	printf(" -- <a href=\"home.cgi#test\">TEST</a>");	
	printf(" -- <a href=\"admin/admin.cgi\" title=\"Delete or download data and model files, set default parameters, change admin password... \">ADMIN</a>");	
	printf(" -- <a href=\"http://www.loria.fr/~lauer/MSVMpack/doc/\" target=\"_blank\" title=\"Online MSVMpack documentation\">HELP</a>");	
	printf("<br><hr size=1 width=100\%% align=center>\n");
	

	// UPLOAD
	
	printf("<form id=\"formupload\" enctype=\"multipart/form-data\" method=\"post\" action=\"upload.cgi\">");
        printf("\n <h2 id=\"upload\">Upload data files</h2>\n");
         printf("<input type=\"file\" name=\"file\" title=\"Data files need to be uploaded to the server before being used for training or testing.\"></input>");
         printf("<input type=\"submit\" name=\"submit\" value=\"Upload the file\" title=\"This may take some time. You can alternatively copy the file to the %s folder of the server.\" ></input>",path_to_data);
	printf("</form>\n");
	printf("This data file is in the\n");
	printf("<label ><input type=\"radio\" name=\"uploadformat\" value=\"msvmpack\" checked=\"true\" onclick=\"UploadMSVMpack();\"> MSVMpack format</label>");
	printf("<label title=\"Missing entries in a LibSVM file will be replaced by zeros\" ><input type=\"radio\" name=\"uploadformat\" value=\"libsvm\" onclick=\"UploadLibSVM();\"> LibSVM format</label>");
	printf("<label title=\"Text file with #instances lines of (#features + 1) space-separated columns. Labels are in the last column.\"><input type=\"radio\" name=\"uploadformat\" value=\"raw\"  onclick=\"UploadRaw();\"> Raw matrix format</label>");
		        
        // FORM TRAIN
	printf("<h2 id=\"train\" title=\"1) Choose a training file. 2) Set the algorithm parameters and the model name. 3) Click 'Start training'. \"> Train a model</h2>\n");
	
	printf("<FORM ACTION=\"Server.cgi\" METHOD=\"GET\"><DIV>\n");
	
	// ========== Training file ==========
	
	// Get info on selected file
	if(data != NULL) {
		pstr = strstr(data, "trainfile=");
		if(pstr != NULL) {
			sscanf(pstr,"trainfile=%[^&]",trainfile);
			sprintf(file,"%s%s",path_to_data,trainfile);
			fp=fopen(file, "r");
			if(fp != NULL) {
				rc = fscanf(fp, "%ld", &nb_data);
				rc = fscanf(fp, "%ld", &dim);				
				fclose(fp);
				printdatainfo = 1;
			}
		}
	}
	
	if(printdatainfo = 1 && dim != 0) {
		printf("<script type=\"text/javascript\"> dimension = %ld</script>\n",dim);
	}
	
	printf("\n <h3>Choose a training data file</h3>From the uploaded file list: <select id=\"trainfile\" name=\"trainfile\" onchange=\"printDataInfo();\" title=\"If your data file is not in the list, upload it with the interface above. Note: files with '.test' in their name are not allowed for training.  \">\n");
	
	sprintf(cmd,"ls -c -1 %s",path_to_data);
	fp = popen(cmd,"r");
	
	while(fgets(buf,80,fp) != NULL) {
		strncpy(file,buf,strlen(buf)-1);
		file[strlen(buf)-1] = '\0';
		// Exclude test files from the list
		if(printdatainfo && strcmp(file, trainfile) == 0) 
			printf("<option value=\"%s\" selected=\"selected\">%s</option>\n",file,file);
		else if (strstr(file,".test") == NULL) 
			printf("<option value=\"%s\">%s</option>\n",file,file);
	}
	printf("</select>\n");
	pclose(fp);
	if(printdatainfo)
		printf("#instances = %ld, #features = %ld\n",nb_data,dim);	

	printf("<p title=\"MSVMpack uses vectorized kernel functions to obtain faster computations with simple data formats.\">Data format: <select id=\"format\" name=\"format\">\n");
	  printf("<option value=\"\" selected=\"selected\">Double-precision floating point (64 bits)</option>\n");
	  printf("<option value=\"-f\">Single-precision floating point (32 bits)</option>\n");
	  printf("<option value=\"-I\">Integer (32 bits)</option>\n");
	  printf("<option value=\"-i\">Short integer (16 bits)</option>\n");
	  printf("<option value=\"-B\">Byte (8 bits)</option>\n");
	  printf("<option value=\"-b\">Bit</option>\n");
	printf("</select></p>\n");

	

	// Table
	printf("<TABLE>\n");
	
	// --- Algo
	printf("<tr valign=\"top\">\n<td style= \"width:50\%%;font-size: 80\%%;text-align:top;\"><h3>Algorithm</h3>\n");
		
	// MSVM
	printf("<p>M-SVM type: <select name=\"msvm\">\n");
	  printf("<option value=\"MSVM2\" selected=\"selected\">Guermeur and Monfrini M-SVM<sup>2</sup></option>\n");
	  printf("<option value=\"WW\">Weston and Watkins M-SVM</option>\n");
	  printf("<option value=\"CS\">Crammer and Singer M-SVM</option>\n");
	  printf("<option value=\"LLW\">Lee, Lin and Wahba M-SVM</option>\n");
	printf("</select></p>\n");
	
	// Kernel
	printf("<p>Kernel: <select id=\"kernel\" name=\"kernel\" onchange=\"chooseKernel();\">\n");
	  printf("<option value=\"1\" selected=\"selected\">Linear</option>\n");
	  printf("<option value=\"2\">Gaussian RBF</option>\n");
	  printf("<option value=\"3\">Homogeneous polynomial</option>\n");
	  printf("<option value=\"4\">Non-homo. polynomial</option>\n");
	  printf("<option value=\"5\">Custom 1</option>\n");
	  printf("<option value=\"6\">Custom 2</option>\n");
	  printf("<option value=\"7\">Custom 3</option>\n");
	printf("</select></p>\n");

	printf("<p>Kernel parameter value: <INPUT id=\"kernelpar\" NAME=\"kernelpar\" SIZE=\"5\" MAXLENGTH=\"20\" value=\"1.0\" disabled=\"true\" style=\"text-align:right;\"></p>\n");		
	
	// C
	printf("<p>Regularization constant C: <INPUT NAME=\"C\" SIZE=\"5\" MAXLENGTH=\"10\" value=\"10.0\" style=\"text-align:right;\"> <br>\n");
	printf("<label onclick=\"setMultipleC();\"><input type=\"checkbox\" name=\"multipleC\"> use a different value of C for each class </label><INPUT  title=\"Enter a space-separated list of values for C (one for each class), e.g., 10.0 2.0 3.3\" NAME=\"Clist\" SIZE=\"20\" MAXLENGTH=\"100\" disabled=\"true\" value=\"\" ></p>\n");
	
	// Normalization
	printf("<p><LABEL FOR=\"normalization\" title=\"Normalize data to zero mean and unit variance.\"><INPUT ID=\"normalization\" NAME=\"normalization\" type=checkbox> Normalize data</LABEL></p>\n");

	printf("</td><td style=\"width:5\%%;\"></td>\n");

	// --- Resources

	printf("<td style=\"width:45\%%;font-size: 80\%%;text-align:top;\"><h3>Resources</h3>\n");	
	// cache memory
	printf("<p>Memory for kernel cache:<br> <INPUT NAME=\"cachemem\" SIZE=\"6\" MAXLENGTH=\"8\" value=\"%d\" style=\"text-align:right;\"> MB </p>\n", cachemem);
	
	// nprocs
	printf("<p>Number of processors: <select id=\"nprocs\" name=\"nprocs\">\n");
	for(i=1;i<nprocs;i++) {
		printf("<option value=\"%d\">%d</option>\n",i,i);
	}
	printf("<option value=\"%d\" selected=\"selected\">%d</option>\n",nprocs,nprocs);
	printf("</select></p>\n");
		
	printf("<h3>Additional parameters</h3>\n");
	printf("<INPUT  title=\"You can enter additional arguments for trainmsvm command-line here\" NAME=\"freeargs\" SIZE=\"15\" MAXLENGTH=\"100\" value=\"\" >\n");
		
	printf("</td></tr></table>\n");
		
	// Model 
	printf("<h3>Choose a model name</h3>\n");
	printf("<p title=\"This will overwrite any file with the same name in the Models/ folder. You can check which models exist with the drop-down list below.\">Save model in: <INPUT NAME=\"model\" SIZE=\"20\" MAXLENGTH=\"50\" value=\"msvm\" style=\"text-align:right;\">.model</p>\n");
	
	// End
	printf("<br><br><INPUT TYPE=\"SUBMIT\" VALUE=\"  Start training  \" style=\"font-size: 150\%%; font-weight:bold;\"></DIV></FORM>\n");
	
	
	// FORM TEST
	
	printf("<FORM ACTION=\"test.cgi\" METHOD=\"GET\"><DIV>\n");
	
	printf("<h2 id=\"test\">Predict class labels / Test a model</h2>\n");
	
	// Model
	
	if(data != NULL) {
		pstr = strstr(data, "modeltest=");
		if(pstr != NULL) {
			sscanf(pstr,"modeltest=%[^&]",modeltest);			
			printinfo = 1;
		}
	}
		
	printf("\n <h3>Choose a model</h3>From the list: <select id=\"modeltest\" name=\"modeltest\" onchange=\"printInfo();\" title=\"Trained models are automatically added to the list. To add another model, copy the .model file to the %s folder of the server.\">\n",path_to_models);
	sprintf(cmd, "ls -c -1 %s",path_to_models);
	fp = popen(cmd,"r");
	
	while(fgets(buf,80,fp) != NULL) {
		strncpy(file,buf,strlen(buf)-1);
		file[strlen(buf)-1] = '\0';
		if(printinfo && strcmp(file, modeltest) == 0) 
			printf("<option value=\"%s\"  selected=\"selected\">%s</option>\n",file,file);		
		else
			printf("<option value=\"%s\">%s</option>\n",file,file);
	}
	printf("</select>\n");
	pclose(fp);
	
	printf("<input type=\"hidden\" name=\"pathmodels\" value=\"%s\" hidden=\"true\"> \n",path_to_models);
	
	if(printinfo) {
		printf("<pre>\n");
		fflush(stdout);
		sprintf(cmd,"cd %s;../predmsvm -i %s;cd ..",path_to_models,modeltest);
		rc = system(cmd);
		printf("</pre>\n");
	}
	
	// Test data file
	printf("\n <h3>Choose a test data file</h3>From the list: <select name=\"testfile\" title=\"If your data file is not in the list, upload it with the interface above.\">\n");

	// Could use 'ls -X', but not available on BSD / Mac OSX 
	sprintf(cmd, "ls -c -1 %s",path_to_data); 
	fp = popen(cmd,"r");
	
	while(fgets(buf,80,fp) != NULL) {
		strncpy(file,buf,strlen(buf)-1);
		file[strlen(buf)-1] = '\0';
		printf("<option value=\"%s\">%s</option>\n",file,file);
	}
	printf("</select>\n");
	pclose(fp);

	printf("<input type=\"hidden\" name=\"pathdata\" value=\"%s\" hidden=\"true\"> \n",path_to_data);
		
	printf("<br><br><INPUT TYPE=\"SUBMIT\" VALUE=\"  Test  \" style=\"font-size: 150\%%; font-weight:bold;\"></DIV></FORM>\n");
	
	HTMLfooter();

}
