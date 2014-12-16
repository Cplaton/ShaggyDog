/**
 * @file    bd_management.c
 * @author  Arnaud LECHANTRE  - ShaggyDogs
 * @brief   Librairy that manage a specific database that save each sensor values available on AR - Drone 
 * @version 1.0
 * @date    1 november 2014
 **/

#include "bd_management.h"

/**
 * @var		next_flight_id_bd
 * @brief	Id of the next flight to create in the database (current max id + 1)
 * @warning	Take care, the management of this value suppose that no value can be inserted into the database by another program
 **/
int next_flight_id_bd;		// Next flight id

/**
 * @var		next_data_id_bd
 * @brief	Id of the next group of sensor data to insert in the database (current max id +1)
 * @warning	Take care, the management of this value suppose that no value can be inserted into the database by another program
 **/
int next_data_id_bd;		// Next data id

/**
 * @var		next_class_id
 * @brief	Id of the next class that would be created in the database (current max id + 1
 * @warning 	Take care, the management of this value suppose that no value can be inserted into the database by another program
 */
int next_class_id_bd;

/**
 * @var 	res_bd_req
 * @brief	Result of the last database request
 * @warning	In order to preserve memory, this variable should be cleared as soon as it content is has been readed (and used)
 **/
PGresult * res_bd_req;		// DB results informations

/**
 * @var		conn_bd
 * @brief	Informations about the current DB connection
 **/
PGconn *conn_bd;		// DB connection informations

// variables necessaires pour normaliser en live
float min_alt = 0.0;
float max_alt = 10.0;
float min_pitch = 0.0;
float max_pitch = 5.6;
float min_roll = -6.0;
float max_roll = 6.0;
float min_vyaw = -160;
float max_vyaw = 300;
float min_vx = -0.3;
float max_vx = 0.6;
float min_vy = -6.0;
float max_vy = 6.0;
float min_vz = -6.0;
float max_vz = 6.0;
float min_ax = -0.03;
float max_ax = 0.007;
float min_ay = -0.000378;
float max_ay = 0.0077;
float min_az = -0.03;
float max_az = 0.015;


void exit_nicely()
{
	PQfinish(conn_bd);
	//exit(1);
}

int connect_to_database()
{
	int i;
	char *temp;

	conn_bd = PQconnectdb("hostaddr = '127.0.0.1' port = '' dbname = 'zuser' user = 'zuser' password = 'zinsa2010'");
	
	if (!conn_bd) {
		fprintf(stderr, "libpq error: PQconnectdb returned NULL.\n\n");
		return 1;
	} 
	if (PQstatus(conn_bd) != CONNECTION_OK) {
		fprintf(stderr, "libpq error: PQstatus(psql) != CONNECTION_OK\n\n");
		return 1;
	} else if(DEBUG_MODE== 1){
		fprintf(stdout, "Connected.\n\n");
	}


	// Initialize the next ids

	res_bd_req = PQexec(conn_bd, "SELECT MAX(flight_id) as max FROM \"Flight\"");
	if (PQresultStatus(res_bd_req) != PGRES_TUPLES_OK) {

		fprintf(stderr, "libpq error: PGress tuples was not OK.\n\n");
		fprintf(stderr, "libpq error: %s\n\n", PQresultErrorMessage(res_bd_req));
		next_flight_id_bd = 0;
	}
	else 
	{
		for(i=0; i<PQntuples(res_bd_req); i++){
			temp = PQgetvalue(res_bd_req, i, PQfnumber(res_bd_req, "max"));
		}
		next_flight_id_bd = atoi(temp) + 1;
	}
	PQclear(res_bd_req);

	res_bd_req = PQexec(conn_bd, "SELECT MAX(data_id) as max FROM \"BasicSensors\"");
	if (PQresultStatus(res_bd_req) != PGRES_TUPLES_OK) {
		fprintf(stderr, "libpq error: %s\n\n", PQresultErrorMessage(res_bd_req));
		next_data_id_bd = 0;
	}
	else 
	{
		for(i=0; i<PQntuples(res_bd_req); i++){
			temp = PQgetvalue(res_bd_req, i, PQfnumber(res_bd_req, "max"));
		}
		next_data_id_bd = atoi(temp) + 1;
	}	
	PQclear(res_bd_req);


	res_bd_req = PQexec(conn_bd, "SELECT MAX(classe_id) as max from \"Classes\"");
	if (PQresultStatus(res_bd_req) != PGRES_TUPLES_OK) {
		fprintf(stderr, "libpq error: %s\n\n", PQresultErrorMessage(res_bd_req));
		next_class_id_bd = 0;
	}
	else 
	{
		for(i=0; i<PQntuples(res_bd_req); i++){
			temp = PQgetvalue(res_bd_req, i, PQfnumber(res_bd_req, "max"));
		}
		next_class_id_bd = atoi(temp) + 1;
	
	}
	PQclear(res_bd_req);
	
/*
    // First get current fields limits
    printf("min pitch:\t%f", min_pitch);
    printf("min pitch:\t%f", max_pitch);
	min_alt = get_min("altitude", "BasicSensors");
	max_alt = get_max("altitude", "BasicSensors");
	min_pitch = get_min("pitch", "BasicSensors");
	max_pitch = get_max("pitch", "BasicSensors");
	min_roll = get_min("roll", "BasicSensors");
	max_roll = get_max("roll", "BasicSensors");
	min_vyaw = get_min("vyaw", "BasicSensors");
	max_vyaw = get_max("vyaw", "BasicSensors");
	min_vx = get_min("vx", "BasicSensors");
	max_vx = get_max("vx", "BasicSensors");
	min_vy = get_min("vy", "BasicSensors");
	max_vy = get_max("vy", "BasicSensors");
	min_vz = get_min("vz", "BasicSensors");
	max_vz = get_max("vz", "BasicSensors");
	min_ax = get_min("ax", "BasicSensors");
	max_ax = get_max("ax", "BasicSensors");
	min_ay = get_min("ay", "BasicSensors");
	max_ay = get_max("ay", "BasicSensors");
	min_az = get_min("az", "BasicSensors");
	max_az = get_max("az", "BasicSensors");
    
    
    
    printf("min pitch:\t%f", min_pitch);
    printf("max pitch:\t%f", max_pitch);
    printf("min roll:\t%f", min_roll);
    printf("max roll:\t%f", max_roll);
    printf("min vyaw:\t%f", min_vyaw);
    printf("max vyaw:\t%f", max_vyaw);
    printf("min vx:\t%f", min_vx);
    printf("max vx:\t%f", max_vx);
    printf("min vy:\t%f", min_vy);
    printf("max vy:\t%f", max_vy);
    printf("min vz:\t%f", min_vz);
    printf("max vz:\t%f", max_vz);
    printf("min ax:\t%f", min_ax);
    printf("max ax:\t%f", max_ax);
    printf("min ay:\t%f", min_ay);
    printf("max ay:\t%f", max_ay);
    printf("min az:\t%f", min_az);
    printf("max az:\t%f", max_az);
*/	
	
	// Finally return	
	return 0;
}

float get_max(char * columnName, char * tableName)
{
	char request[300] ; 
	char * temp;
	int i;
	sprintf(request, "SELECT MAX (%s) AS max FROM \"%s\"", columnName, tableName);

	res_bd_req = PQexec(conn_bd, request);
	if (PQresultStatus(res_bd_req) != PGRES_TUPLES_OK) {

		fprintf(stderr, "libpq error: PGress tuples was not OK.\n\n");
		fprintf(stderr, "libpq error: %s\n\n", PQresultErrorMessage(res_bd_req));
		PQclear(res_bd_req);
		return 0;
	}
	else 
	{
		for(i=0; i<PQntuples(res_bd_req); i++){
			temp = PQgetvalue(res_bd_req, i, PQfnumber(res_bd_req, "max"));
		}
		PQclear(res_bd_req);
		return atof(temp);
	}

}

float get_min(char * columnName, char * tableName)
{
	char request[300] ; 
	char * temp;
	int i;
	sprintf(request, "SELECT MIN (%s) AS max FROM \"%s\"", columnName, tableName);

	res_bd_req = PQexec(conn_bd, request);
	if (PQresultStatus(res_bd_req) != PGRES_TUPLES_OK) {

		fprintf(stderr, "libpq error: PGress tuples was not OK.\n\n");
		fprintf(stderr, "libpq error: %s\n\n", PQresultErrorMessage(res_bd_req));
		PQclear(res_bd_req);
		return 0;
	}
	else 
	{
		for(i=0; i<PQntuples(res_bd_req); i++){
			temp = PQgetvalue(res_bd_req, i, PQfnumber(res_bd_req, "max"));
		}
		PQclear(res_bd_req);
		return atof(temp);
	}

}

float norm_value(float init, float min, float max)
{
	//return (init - ((min+max)/2.0)) / ((max-min) / 2.0);
	return ((init - min) / (max - min));
}

double norm_indiv(double init, int type){
	float min = 0;
	float max = 0;

	switch(type)
	{
	case 0: //DATA_ALT: 
		min = min_alt;
		max = max_alt;
	break;
	case 1: //DATA_PITCH:
		min = min_pitch;
		max = max_pitch;
	break;
	case 2 : //DATA_ROLL:
		min = min_roll;
		max = max_roll;
	break;
	case 3: // DATA_VYAW:
		min = min_vyaw;
		max = max_vyaw;
	break;
	case 4: //DATA_VX:
		min = min_vx;
		max = max_vy;
	break;
	case 5: //DATA_VY:
		min = min_vy;
		max = max_vy;
	break;
	case 6 : //DATA_VZ:
		min = min_vz;
		max = max_vz;
	break;
	case 7: //DATA_AY:
		min = min_ax;
		max = max_ax;
	break;
	case 8 ://DATA_AZ:
		min = min_ay;
		max = max_ay;
	break;
	case 9 ://DATA_AZ:
		min = min_az;
		max = max_az;
	break;
	}
	/*printf("min : %f\n",min);
	printf("max : %f\n",max);*/
	
	return (double)norm_value((float)init,min,max);

}

struct augmented_navdata * get_values_from_db(int number, int flight_id, int * nb_res)
{
	char request[300];
	char limit_req[15] ="";
	char where_req[30] ="";
	int  i;
	struct augmented_navdata* temp_data; 
	struct augmented_navdata* result; 
	*nb_res=0;

	if(number >0) {
		sprintf(limit_req ," LIMIT %d", number);
	}

	if (flight_id >=0){
		sprintf(where_req ," WHERE flight=%d", flight_id);
	}

	sprintf( request, "SELECT time, altitude, pitch, roll, vyaw, vx, vy, vz, ax, ay, az, classe FROM \"BasicSensors\" %s %s", where_req, limit_req);
	printf("\n %s \n", request);
	res_bd_req = PQexec(conn_bd, request);

	if (PQresultStatus(res_bd_req) != PGRES_TUPLES_OK) {
		fprintf(stderr, "libpq error: PGress tuples was not OK.\n\n");
		fprintf(stderr, "libpq error: %s\n\n", PQresultErrorMessage(res_bd_req));
		return 0;
	}
	else 
	{

		*nb_res = PQntuples(res_bd_req);
		result = malloc( *nb_res * sizeof(struct augmented_navdata));

		for(i=0; i<PQntuples(res_bd_req); i++){
			temp_data = &result[i];
			temp_data->time  = atoi(PQgetvalue(res_bd_req, i, PQfnumber(res_bd_req, "time"))); 
			temp_data->alt   = atof(PQgetvalue(res_bd_req, i, PQfnumber(res_bd_req, "altitude")));
			temp_data->pitch = atof(PQgetvalue(res_bd_req, i, PQfnumber(res_bd_req, "pitch")));
			temp_data->roll  = atof(PQgetvalue(res_bd_req, i, PQfnumber(res_bd_req, "roll")));
			temp_data->vyaw  = atof(PQgetvalue(res_bd_req, i, PQfnumber(res_bd_req, "vyaw"))); 
			temp_data->vx    = atof(PQgetvalue(res_bd_req, i, PQfnumber(res_bd_req, "vx")));
			temp_data->vy    = atof(PQgetvalue(res_bd_req, i, PQfnumber(res_bd_req, "vy"))); 
			temp_data->vz    = atof(PQgetvalue(res_bd_req, i, PQfnumber(res_bd_req, "vz"))); 
			temp_data->ax    = atof(PQgetvalue(res_bd_req, i, PQfnumber(res_bd_req, "ax"))); 
			temp_data->ay    = atof(PQgetvalue(res_bd_req, i, PQfnumber(res_bd_req, "ay"))); 
			temp_data->az    = atof(PQgetvalue(res_bd_req, i, PQfnumber(res_bd_req, "az")));
			temp_data->class_id = atoi(PQgetvalue(res_bd_req, i, PQfnumber(res_bd_req, "classe")));
		}
		if(DEBUG_MODE) 
		{
			for( i=0; i<*nb_res; i++)
			{
				printf("Donnée récupérée: %d, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f\n", 
					result[i].time, 		
					result[i].alt, 
					result[i].pitch, 
					result[i].roll, 
					result[i].vyaw,
					result[i].vx, 
					result[i].vy, 
					result[i].vz, 
					result[i].ax, 
					result[i].ay, 
					result[i].az);
			}	
		}
		return result;
	}

}

struct augmented_navdata * get_normed_values_from_db(int number, int flight_id, int * nb_res)
{

	// Declarations 
	struct augmented_navdata * data;
	int i;
	/*
	// First get current fields limits 
	min_alt = get_min("altitude", "BasicSensors");
	max_alt = get_max("altitude", "BasicSensors");
	min_pitch = get_min("pitch", "BasicSensors");
	max_pitch = get_max("pitch", "BasicSensors");
	min_roll = get_min("roll", "BasicSensors");
	max_roll = get_max("roll", "BasicSensors");
	min_vyaw = get_min("vyaw", "BasicSensors");
	max_vyaw = get_max("vyaw", "BasicSensors");
	min_vx = get_min("vx", "BasicSensors");
	max_vx = get_max("vx", "BasicSensors");
	min_vy = get_min("vy", "BasicSensors");
	max_vy = get_max("vy", "BasicSensors");
	min_vz = get_min("vz", "BasicSensors");
	max_vz = get_max("vz", "BasicSensors");
	min_ax = get_min("ax", "BasicSensors");
	max_ax = get_max("ax", "BasicSensors");
	min_ay = get_min("ay", "BasicSensors");
	max_ay = get_max("ay", "BasicSensors");
	min_az = get_min("az", "BasicSensors");
	max_az = get_max("az", "BasicSensors");
	*/

	// Then get DB values 
	data = get_values_from_db(number, flight_id, nb_res);

	// Then norm the values
	for( i=0; i< *nb_res; i++)
	{
		data[i].alt		= norm_value(data[i].alt, min_alt, max_alt);
		data[i].pitch	= norm_value(data[i].pitch, min_pitch, max_pitch);
		data[i].roll	= norm_value(data[i].roll, min_roll, max_roll);
		data[i].vyaw	= norm_value(data[i].vyaw, min_vyaw, max_vyaw);
		data[i].vx		= norm_value(data[i].vx, min_vx, max_vx);
		data[i].vy		= norm_value(data[i].vy, min_vy, max_vy);
		data[i].vz		= norm_value(data[i].vz, min_vz, max_vz);
		data[i].ax		= norm_value(data[i].ax, min_ax, max_ax);
		data[i].ay		= norm_value(data[i].ay, min_ay, max_ay);
		data[i].az		= norm_value(data[i].az, min_az, max_az);
	}
	
	// And finaly return
	return data;

}

int write_data_to_csv(char * csvFileName, int number, int flight_id, int should_norm)
{
 /*   
	// Declarations 
	struct augmented_navdata * data;
	int nb_res;
	FILE * csv;
	int i;

	// First open the CSV file 
	csv= open_csv_file(csvFileName);

	// Then get the data from the database 

	// If should norm is set to 1, get the limits of each field
	if(should_norm == 1)
	{
		data = get_normed_values_from_db(number, flight_id, &nb_res);
	}
	else 
	{
		data = get_values_from_db(number, flight_id, &nb_res);
	}
	

	// Then write each values into the CSV file
	for(i=0; i<nb_res; i++)
	{
		new_data_csv(csv, data[i].alt, data[i].pitch, data[i].roll, data[i].vyaw, data[i].vx, data[i].vy, data[i].vy, data[i].vz, data[i].ax, data[i].ay, data[i].az, data[i].class_id);
	}

	// Finally, close the file
	close_navdata_file(csv);
    */
	return 0; 
}

int start_new_flight()
{

	// Sends the request
	char request[100];
	sprintf( request, "INSERT INTO \"Flight\" VALUES ( %d ) ",  next_flight_id_bd );
	res_bd_req = PQexec(conn_bd, request);
	if (PQresultStatus(res_bd_req) != PGRES_COMMAND_OK)
	{
		fprintf(stderr, "BEGIN command failed: %s", PQerrorMessage(conn_bd));
		PQclear(res_bd_req);
		exit_nicely(conn_bd);
		return 1;
	}
	else {
		next_flight_id_bd ++;
		PQclear(res_bd_req);
		return 0;
	}
	
}


int insert_new_data( int time, float alt, float pitch, float roll, float vyaw, float vx, float vy, float vz, float ax, float ay, float az, int class_id)
{

	// Sends the request
	char request[512];

	// TODO protéger du débordement
	sprintf(request, "INSERT INTO \"BasicSensors\" VALUES (%d, %d, %d, %f, %f, %f , %f, %f, %f, %f, %f, %f, %f, %d) ",
			 next_data_id_bd, next_flight_id_bd -1, time, alt, pitch, roll, vyaw, vx, vy, vz, ax, ay, az, class_id);

	res_bd_req = PQexec(conn_bd, request);
	if (PQresultStatus(res_bd_req) != PGRES_COMMAND_OK)
	{
		fprintf(stderr, "BEGIN command failed: %s", PQerrorMessage(conn_bd));
		PQclear(res_bd_req);
		exit_nicely(conn_bd);
		return 1;
	}
	else {
		next_data_id_bd ++;
		PQclear(res_bd_req);
		return 0;
	}
	
}


int disconnect_to_database()
{
	/* close the connection to the database and cleanup */
	PQfinish(conn_bd);
}

