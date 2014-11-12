#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*
	Convert a data file in the Crammer&Singer format 
	to MSVMpack format
*/
void convert_data_CrammerSinger(char *data_file, char *output, long nb_data)
{
	FILE *fs, *fp;
	long i,j,dim_input;
	double value_y,value_X;
	char buf[10000];
	long nr = 0;
	char *token;
	char *pbuf, *save_ptr;
	
	if((fs=fopen(data_file, "r"))==NULL)
	  {
	  printf("\nFile of data %s cannot be open.\n", data_file);
	  exit(0);
	  }
	
	// Get first line...
	if(fgets(buf, 10000, fs) == NULL) {
		printf("Error while reading data file.\n");
		exit(1);
	}
	
	// ... to set dim_input 
	dim_input = -1;
	pbuf = buf;
	do{
		token=strtok_r(pbuf, " ", &save_ptr);
		if(token != NULL && *token != '\0' && *token != '\n' && *token != '\r') {
			dim_input++;
			printf("diminput = %ld ; token = %s\n",dim_input,token);
		}
		pbuf = NULL;	
	} while(token != NULL);
	
	printf("diminput = %ld\n",dim_input);
	
	// Prepare files
	rewind(fs);	// for input
	
	// and output
	if((fp=fopen(output, "w"))==NULL)	
	  {
	  printf("\n Output file %s cannot be open.\n", output);
	  exit(0);
	  }
	
	fprintf(fp,"%ld\n",nb_data);
	fprintf(fp,"%ld\n",dim_input);
	
	// Read data
	for(i=1; i<=nb_data; i++)
	  {
  
	  nr += fscanf(fs, "%lf", &value_y);
	
	    
	  for(j=1; j<=dim_input; j++) {
	    nr += fscanf(fs, "%lf", &value_X);
	    fprintf(fp,"%lf ",value_X);
	  }
	  
	  // write label
	  fprintf(fp,"%ld\n",(long)value_y);	  
	  
	}
	
	fclose(fs);
	fclose(fp);
	
	
	return;
}

/*
	Convert MSVMpack data set to CS data file
*/
void convert_data_to_CrammerSinger(char *input_file, char *data_file)
{
	FILE *fs, *fp;
	long i,j;
	long dim_input;
	long nb_data;
	double value_y,value_X;
	char buf[10000];
	long nr = 0;
	
	if((fs=fopen(input_file, "r"))==NULL)
	  {
	  printf("\nFile of data %s cannot be open.\n", input_file);
	  exit(0);
	  }
	
	
	if((fp=fopen(data_file, "w"))==NULL)
	  {
	  printf("\nFile of data %s cannot be open.\n", data_file);
	  exit(0);
	  }
	
	nr += fscanf(fs, "%ld", &nb_data);
	nr += fscanf(fs, "%ld", &dim_input);

	double *X = (double*)malloc(sizeof(double) * (dim_input+1));
	

	// Read first line with data
	for(j=1; j<=dim_input; j++) {
	    nr += fscanf(fs, "%lf", &value_X);
	    X[j] = value_X;
	}
	// Check if there is a label at the end of the line
	if(fgets(buf, 10000, fs) == NULL) {
		printf("Error while reading data file.\n");
		exit(1);
	}
	if(sscanf(buf, "%lf", &value_y) > 0) {
		fprintf(fp, "%ld ",(long) value_y -1 );
		for(j=1; j<=dim_input; j++) 
	    		fprintf(fp,"%1.8lf ",X[j]);
	}
	else {
		printf("Conversion in CS format cannot work without labels in data file.\n");
		exit(0);
	}
	fprintf(fp,"\n");
	// Read the rest of the file
	for(i=2; i<=nb_data; i++)
	  {
	  for(j=1; j<=dim_input; j++) {
	    nr += fscanf(fs, "%lf", &value_X);
	    X[j] = value_X;
	  }
	  nr += fscanf(fs, "%lf", &value_y);
	   fprintf(fp, "%ld ",(long)value_y - 1);// Labels must be in the range [0,Q-1]
	   
	    for(j=1; j<=dim_input; j++) {
	    	fprintf(fp,"%1.8lf ",X[j]);
	    }
	    fprintf(fp,"\n");
	  }
	
	fclose(fp);
	fclose(fs);
	
	if(nr < dim_input * nb_data)
		printf("\nError in reading file %s, not enough data.\n", data_file);

	
	free(X);
	return;
}

/*
	Convert MSVMpack data set to Libsvm data file
*/
void convert_data_to_libsvm(char *input_file, char *data_file)
{
	FILE *fs, *fp;
	long i,j;
	long dim_input;
	long nb_data;
	double value_y=0,value_X;
	char buf[10000];
	long nr = 0;
	
	if((fs=fopen(input_file, "r"))==NULL)
	  {
	  printf("\nFile of data %s cannot be open.\n", input_file);
	  exit(0);
	  }
	
	
	if((fp=fopen(data_file, "w"))==NULL)
	  {
	  printf("\nFile of data %s cannot be open.\n", data_file);
	  exit(0);
	  }
	
	nr += fscanf(fs, "%ld", &nb_data);
	nr += fscanf(fs, "%ld", &dim_input);

	double *X = (double*)malloc(sizeof(double) * (dim_input+1));
	

	// Read first line with data
	for(j=1; j<=dim_input; j++) {
	    nr += fscanf(fs, "%lf", &value_X);
	    X[j] = value_X;
	}
	// Check if there is a label at the end of the line
	if(fgets(buf, 10000, fs) == NULL) {
		printf("Error while reading data file.\n");
		exit(1);
	}
	if(sscanf(buf, "%lf", &value_y) > 0) {
		fprintf(fp, "%ld ",(long) value_y -1 );
		for(j=1; j<=dim_input; j++) 
	    		fprintf(fp,"%ld:%1.8lf ",j,X[j]);
	}
	else {
		value_y=0;
		fprintf(fp, "0 ");
		for(j=1; j<=dim_input; j++) 
	    		fprintf(fp,"%ld:%1.8lf ",j,X[j]);
	}
	fprintf(fp,"\n");
	// Read the rest of the file
	for(i=2; i<=nb_data; i++)
	  {
	  for(j=1; j<=dim_input; j++) {
	    nr += fscanf(fs, "%lf", &value_X);
	    X[j] = value_X;
	  }
	  if(value_y == 0)
	   	fprintf(fp, "0 ");// dummy label
	  else {	  	
		nr += fscanf(fs, "%lf", &value_y);
		fprintf(fp, "%ld ",(long)value_y - 1);// Labels in the range [0,Q-1]
	   }
	    for(j=1; j<=dim_input; j++) {
	    	fprintf(fp,"%ld:%1.8lf ",j,X[j]);
	    }
	    fprintf(fp,"\n");
	  }
	
	fclose(fp);
	fclose(fs);
	
	if(nr < dim_input * nb_data)
		printf("\nError in reading file %s, not enough data.\n", data_file);

	
	free(X);
	return;
}

/*
	Convert a data file in libsvm format to MSVMpack format
*/
void convert_data_libsvm_int(char *data_file, char *output, long nb_data)
{
	FILE *fs, *fp;
	long i,j,dim_input;
	int value_y,value_X;
	char buf[10000];
	long nr = 0;
	char *token;
	char *pbuf, *save_ptr;
	
	if((fs=fopen(data_file, "r"))==NULL)
	  {
	  printf("\nFile of data %s cannot be open.\n", data_file);
	  exit(0);
	  }
	
	// Get first line...
	if(fgets(buf, 10000, fs) == NULL) {
		printf("Error while reading data file.\n");
		exit(1);
	}
	
	// ... to set dim_input 
	dim_input = -1;
	pbuf = buf;
	do{
		token=strtok_r(pbuf, " ", &save_ptr);
		if(token != NULL && *token != '\0' && *token != '\n' && *token != '\r') {
			dim_input++;
			//printf("diminput = %ld ; token = %s\n",dim_input,token);
		}
		pbuf = NULL;	
	} while(token != NULL);
	
	printf("diminput = %ld\n",dim_input);
	
	// Prepare files
	rewind(fs);	// for input
	
	// and output
	if((fp=fopen(output, "w"))==NULL)	
	  {
	  printf("\n Output file %s cannot be open.\n", output);
	  exit(0);
	  }
	
	fprintf(fp,"%ld\n",nb_data);
	fprintf(fp,"%ld\n",dim_input);
	
	// Read data
	for(i=1; i<=nb_data; i++)
	  {
  
	  nr += fscanf(fs, "%d", &value_y);
	 	    
	  for(j=1; j<=dim_input; j++) {
	    nr += fscanf(fs, "%d:", &value_X);
	    nr += fscanf(fs, "%d", &value_X);
	    fprintf(fp,"%d ",value_X);
	  }
	  
	  // write label
	  fprintf(fp,"%d\n",value_y);	  
	  
	}
	
	fclose(fs);
	fclose(fp);
	
	
	return;
}

 
/*
	Convert a data file in libsvm format to MSVMpack format
*/
void convert_data_libsvm(char *data_file, char *output, long nb_data)
{
	FILE *fs, *fp;
	long i,j,dim_input,index;
	int value_y;
	double value_X, *Xi;
	char buf[10000];
	char tmp[20];
	long nr = 0;
	char *token;
	char *pbuf, *save_ptr;
	
	if((fs=fopen(data_file, "r"))==NULL)
	  {
	  printf("\nFile of data %s cannot be open.\n", data_file);
	  exit(0);
	  }
	
	dim_input = -1;	
	// Set dim_input : 
	for(i = 0; i< nb_data; i++) {	
		// Get line
		if(fgets(buf, 10000, fs) == NULL) {
			printf("Error while reading data file.\n");
			exit(1);
		}
	
		// Find number of features

		pbuf = buf;
		save_ptr = NULL;
		token=strtok_r(pbuf, " ", &save_ptr); // Skip label
		pbuf = NULL;
		do{
			token=strtok_r(pbuf, " ", &save_ptr);
			if(token != NULL && *token != '\0' && *token != '\n' && *token != '\r') {
				sscanf(token,"%ld",&index );

				if(index > dim_input) 
					dim_input = index;
				//printf("diminput = %ld ;  token = %s\n",dim_input,token);
			}

		} while(token != NULL);	
	}
	rewind(fs);
	
	printf("Found %ld features... \n",dim_input);	
	
	// Read data
	
	Xi = calloc(dim_input, sizeof(double));
	
	// and write output
	if((fp=fopen(output, "w"))==NULL)	
	  {
	  printf("\n Output file %s cannot be open.\n", output);
	  exit(0);
	  }
	
	fprintf(fp,"%ld\n",nb_data);
	fprintf(fp,"%ld\n",dim_input);
	
	
	for (i=0;i<nb_data;i++) {
		if(fgets(buf, 10000, fs) == NULL) {
			printf("Error while reading data file.\n");
			exit(1);
		}
		
		pbuf = buf;
		save_ptr = NULL;
		token=strtok_r(pbuf, " ", &save_ptr); // Label
		pbuf = NULL;
		
		nr = sscanf(token, "%d", &value_y);
		do{
			token=strtok_r(pbuf, " ", &save_ptr);
			if(token != NULL && *token != '\0' && *token != '\n' && *token != '\r') {
				sscanf(token,"%ld:%lf",&index,&value_X );

				Xi[index-1] = value_X;
				//printf("diminput = %ld ;  token = %s\n",dim_input,token);
			}

		} while(token != NULL);	
		for (j=0;j< dim_input; j++) {
		    fprintf(fp, "%lf ", Xi[j]);
		}
		fprintf(fp,"%d\n",value_y);
	}
	
	
	fclose(fs);
	fclose(fp);
	
	printf("Done.\n");
	return;
}
/*
	Convert from Raw matrix format to MSVMpack format
*/
void convert_data_raw(char *data_file, char *output, long nb_data)
{
	FILE *fs, *fp;
	long i,j,dim_input;
	double value_y,value_X;
	char buf[10000];
	long nr = 0;
	char *token;
	char *pbuf, *save_ptr;
	
	if((fs=fopen(data_file, "r"))==NULL)
	  {
	  printf("\nFile of data %s cannot be open.\n", data_file);
	  exit(0);
	  }
	
	// Get first line...
	if(fgets(buf, 10000, fs) == NULL) {
		printf("Error while reading data file.\n");
		exit(1);
	}
	
	// ... to set dim_input 
	dim_input = -1;
	pbuf = buf;
	do{
		token=strtok_r(pbuf, " ", &save_ptr);
		if(token != NULL && *token != '\0' && *token != '\n' && *token != '\r') {
			dim_input++;
			printf("diminput = %ld ; token = %s\n",dim_input,token);
		}
		pbuf = NULL;
	} while(token != NULL);
	
	printf("Dimension of data  = %ld\n",dim_input);
	
	// Prepare files
	rewind(fs);	// for input
	
	// and output
	if((fp=fopen(output, "w"))==NULL)	
	  {
	  printf("\n Output file %s cannot be open.\n", output);
	  exit(0);
	  }
	
	fprintf(fp,"%ld\n",nb_data);
	fprintf(fp,"%ld\n",dim_input);
	
	// Read data
	for(i=1; i<=nb_data; i++)
	  {		    
	  for(j=1; j<=dim_input; j++) {
	    nr += fscanf(fs, "%lf", &value_X);
	    fprintf(fp,"%lf ",value_X);
	  }
	  
	  nr += fscanf(fs, "%lf", &value_y);

	  // write label
	  fprintf(fp,"%ld\n",(long)value_y);	  
	  
	}
	
	fclose(fs);
	fclose(fp);
	
	
	return;
}
int main(int argc, char *argv[]) {

	if(argc<4) {
		printf("No input file or no number of data.\n");
		printf("Usage:\n");		
		printf("        convert filename output_file number_of_data \n");
		printf(" Easiest way is :\n");
		printf("      convert filename filenmae.mpd `cat filename | wc -l` \n");
		exit(0);
	}
	
	
	long nb_data = atol(argv[3]);
	
	if(argc < 5) {	
		printf("Converting %ld data from %s to %s...\n",nb_data,argv[1],argv[2]);
		convert_data_CrammerSinger(argv[1],argv[2],nb_data);
	}
	else {
		switch(atoi(argv[4])) {
		case 1:
		printf("Converting %s (MSVMpack format) to %s (CS format)...\n",argv[1],argv[2]);
		convert_data_to_CrammerSinger(argv[1],argv[2]);
		break;
		case 2:
		printf("Converting %s (MSVMpack format) to %s (Libsvm format)...\n",argv[1],argv[2]);
		convert_data_to_libsvm(argv[1],argv[2]);
		break;
		case 3:
		printf("Converting %s (libsvm format) to %s (MSVMpack INT format)...\n",argv[1],argv[2]);
		convert_data_libsvm_int(argv[1],argv[2],nb_data);
		break;
		case 4:
		printf("Converting %ld data from %s (LibSVM format) to %s (MSVMpack format)...\n",nb_data,argv[1],argv[2]);
		convert_data_libsvm(argv[1],argv[2],nb_data);
		break;
		case 5:
		printf("Converting %ld data from %s (RAW format) to %s (MSVMpack format)...\n",nb_data,argv[1],argv[2]);
		convert_data_raw(argv[1],argv[2],nb_data);
		break;
		default:
		break;
		}
	}
	
	return 0;
}
