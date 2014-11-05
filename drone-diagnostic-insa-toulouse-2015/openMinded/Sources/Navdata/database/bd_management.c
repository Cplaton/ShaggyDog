#include "bd_management.h"



int next_flight_id_bd;		// Next flight id
int next_data_id_bd;		// Next data id
PGresult * res_bd_req;		// DB results informations
PGconn *conn_bd;		// DB connection informations

/** 
@brief : Closes nicely the DB connection and exits
@param psql: Contains the database informations
 **/
static void exit_nicely()
{
	PQfinish(conn_bd);
	//exit(1);
}

/** 
@brief : Opens a connection with the database
@return : 0 if connection successes, 1 in case of error. Error messages are printed in standard error.
 **/
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

	// Finally return	
	return 0;
}


/**
@brief: get values from the database
@param number: a limit for the number of values that should be getted, 0 if no limit
@param flight_id: the id of the flight that should be getted, -1 if get all 
@param result: would be filled with the number of line founded corresponding to the request
@return: an array filled with the data matching the request, null in case of error 
*/
struct augmented_navdata * get_values_from_db(int number, int flight_id, int * nb_res_bd_req)
{

	char request[300];
	char limit_req[15] ="";
	char where_req[30] ="";
	int  i;
	struct augmented_navdata* temp_data; 
	struct augmented_navdata* result; 
	*nb_res_bd_req=0;

	if(number >0) {
		sprintf(limit_req ," LIMIT %d", number);
	}

	if (flight_id >=0){
		sprintf(where_req ," WHERE flight=%d", flight_id);
	}

	sprintf( request, "SELECT time, altitude, pitch, roll, vyaw, vx, vy, vz, ax, ay, az FROM \"BasicSensors\" %s %s", where_req, limit_req);
	printf("\n %s \n", request);
	res_bd_req = PQexec(conn_bd, request);

	if (PQresultStatus(res_bd_req) != PGRES_TUPLES_OK) {
		fprintf(stderr, "libpq error: PGress tuples was not OK.\n\n");
		fprintf(stderr, "libpq error: %s\n\n", PQresultErrorMessage(res_bd_req));
		return 0;
	}
	else 
	{

		*nb_res_bd_req = PQntuples(res_bd_req);
		result = malloc( *nb_res_bd_req * sizeof(struct augmented_navdata));

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
		}
				for( i=0; i<*nb_res_bd_req; i++)
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

		return result;
	}

}

/** 
@brief : Create a new flight in the database
@return : 0 if connection successes, 1 in case of error. Error messages are printed in standard error.
 **/
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

/** 
@brief : Insert a new data into the database
@param time : time in second spend since the begining of the flight
@param alt : altitude of the dronne
@param pitch : pitch of the drone
@param roll : roll of the drone
@param vyaw : yaw speed of the drone
@param vx : x speed of the drone
@param vy : y speed of the drone
@param vz : z speed of the drone
@param ax : x acceleration of the drone
@param ay : y acceleration of the drone
@param az : z acceleration of the drone
@return : 0 if connection successes, 1 in case of error. Error messages are printed in standard error.
 **/
int insert_new_data( int time, float alt, float pitch, float roll, float vyaw, float vx, float vy, float vz, float ax, float ay, float az)
{

	// Sends the request
	char request[512];

	// TODO protéger du débordement
	sprintf(request, "INSERT INTO \"BasicSensors\" VALUES (%d, %d, %d, %f, %f, %f , %f, %f, %f, %f, %f, %f, %f) ",
			 next_data_id_bd, next_flight_id_bd -1, time, alt, pitch, roll, vyaw, vx, vy, vz, ax, ay, az);

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

/** 
@brief : Closes a connection with the database
@return : 0 if connection successes, 1 in case of error. Error messages are printed in standard error.
 **/
int disconnect_to_database()
{
	/* close the connection to the database and cleanup */
	PQfinish(conn_bd);
}

