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


static void exit_nicely()
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


	res_bd_req = PQexec(conn_bd, "SELECT MAX(class_id) as max from \"Classes\"");
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

	// Finally return	
	return 0;
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

