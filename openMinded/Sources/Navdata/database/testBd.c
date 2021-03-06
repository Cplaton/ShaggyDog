/**
 * @file    testBd.c
 * @author  Arnaud LECHANTRE  - ShaggyDogs
 * @brief   Test file of the db_management lib (lib_openMinded_db)
 * @version 1.0
 * @date    1 november 2014
 **/

#include "bd_management.h"
 
int
main()
{
	int i;
	if(connect_to_database()==0)
	{
		printf("Connected\n");

		// Test d'insertion de donnée :
		printf("\nTest d'insertion de donnée:\n");

		if(start_new_flight() == 0)
		{
			printf("Nouveau vol créé:\n");
			//Oussama El Fatayri à ajouter  1 argument à la fonction insert_new_data
			if(insert_new_data( 1, 72.0, 1.0, 5.0, 4.0, 65.0, 45.0, 0.6, 0.4, 0.6, 0.678, 0) == 0)
			{
				printf("Ajout de donnée réussi\n");
			
				struct augmented_navdata * data;
				int nb_res;
				data = get_values_from_db(0, -1, &nb_res);
				for( i=0; i<nb_res; i++)
				{
					printf("Donnée récupérée: %d, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f\n", 
						data[i].time, 		
						data[i].alt, 
						data[i].pitch, 
						data[i].roll, 
						data[i].vyaw,
						data[i].vx, 
						data[i].vy, 
						data[i].vz, 
						data[i].ax, 
						data[i].ay, 
						data[i].az);
				}				
				printf("Fin de la récupération de donnée\n");		

				disconnect_to_database();
			}
			else
			{
				printf("Erreur lors de l'ajout de donnée");
			}
		}		
		else 
		{	
			printf("Erreur de création du vol\n");
		}

	}
	else 
	{
		printf("Error during connection\n");
	}
} 
