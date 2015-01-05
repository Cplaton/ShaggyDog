#include "knn_classification.h"

indiv_knn * load_data(char * nomFichier) {

	FILE * fichier = NULL;

	static indiv_knn data_matrice[NB_LIGNE];
	float floatdata[9];
	int intdata;

	fichier = fopen(nomFichier, "r+");
	int i;
	for (i = 0; i<NB_LIGNE; i++) {
		
		fscanf(fichier, "%d %f %f %f %f %f %f %f %f %f", &intdata, &floatdata[0], &floatdata[1], &floatdata[2], &floatdata[3], &floatdata[4], &floatdata[5], &floatdata[6], &floatdata[7], &floatdata[8]);
		data_matrice[i].pitch = floatdata[0];
		data_matrice[i].roll  = floatdata[1];
		data_matrice[i].vyaw  = floatdata[2];
		data_matrice[i].vx    = floatdata[3];
		data_matrice[i].vy    = floatdata[4];
		data_matrice[i].vz    = floatdata[5];
		data_matrice[i].ax    = floatdata[6];
		data_matrice[i].ay    = floatdata[7];
		data_matrice[i].az    = floatdata[8];

		data_matrice[i].class_id    = intdata;
	}

	fclose(fichier);
	return data_matrice;
}

float euclideanDistance(indiv_knn instance1, indiv_knn instance2) {

	float distance = 0.0;

	distance += pow ((instance1.pitch - instance2.pitch), 2);
	distance += pow ((instance1.roll - instance2.roll), 2);
	distance += pow ((instance1.vyaw - instance2.vyaw), 2);
	distance += pow ((instance1.vx - instance2.vx), 2);
	distance += pow ((instance1.vy - instance2.vy), 2);
	distance += pow ((instance1.vz - instance2.vz), 2);
	distance += pow ((instance1.ax - instance2.ax), 2);
	distance += pow ((instance1.ay - instance2.ay), 2);
	distance += pow ((instance1.az - instance2.az), 2);

	return sqrt(distance);
}


indiv_knn * getNeighbors(indiv_knn trainingSet[NB_LIGNE], indiv_knn testInstance) {

	float distances[NB_LIGNE];
	indiv_knn data_table[NB_LIGNE];
	static indiv_knn neighbors[K];
	float dist;
	indiv_knn tmp1;
	float tmp2;

	int i;
	for(i=0;i<NB_LIGNE;i++){
		dist = euclideanDistance(testInstance, trainingSet[i]);		
		data_table[i].pitch = trainingSet[i].pitch;
		data_table[i].roll = trainingSet[i].roll;
		data_table[i].vyaw = trainingSet[i].vyaw;
		data_table[i].vx = trainingSet[i].vx;
		data_table[i].vy = trainingSet[i].vy;
		data_table[i].vz = trainingSet[i].vz;
		data_table[i].ax = trainingSet[i].ax;
		data_table[i].ay = trainingSet[i].ay;
		data_table[i].az = trainingSet[i].az;
		data_table[i].class_id = trainingSet[i].class_id;

		distances[i] = dist;
	}

	int j;
	for (i=0;i<NB_LIGNE;i++) {
		for (j=i+1;j<NB_LIGNE;j++) {
			if (distances[i] > distances[j]) {
				

				tmp1.pitch = data_table[i].pitch;
				tmp1.roll = data_table[i].roll;
				tmp1.vyaw = data_table[i].vyaw;
				tmp1.vx = data_table[i].vx;
				tmp1.vy = data_table[i].vy;
				tmp1.vz = data_table[i].vz;
				tmp1.ax = data_table[i].ax;
				tmp1.ay = data_table[i].ay;
				tmp1.az = data_table[i].az;
				tmp1.class_id = data_table[i].class_id;

				tmp2 = distances[i];

				data_table[i].pitch = data_table[j].pitch;
				data_table[i].roll = data_table[j].roll;
				data_table[i].vyaw = data_table[j].vyaw;
				data_table[i].vx = data_table[j].vx;
				data_table[i].vy = data_table[j].vy;
				data_table[i].vz = data_table[j].vz;
				data_table[i].ax = data_table[j].ax;
				data_table[i].ay = data_table[j].ay;
				data_table[i].az = data_table[j].az;
				data_table[i].class_id = data_table[j].class_id;

				distances[i] = distances[j];

				data_table[j].pitch = tmp1.pitch;
				data_table[j].roll = tmp1.roll;
				data_table[j].vyaw = tmp1.vyaw;
				data_table[j].vx = tmp1.vx;
				data_table[j].vy = tmp1.vy;
				data_table[j].vz = tmp1.vz;
				data_table[j].ax = tmp1.ax;
				data_table[j].ay = tmp1.ay;
				data_table[j].az = tmp1.az;
				data_table[j].class_id = tmp1.class_id;

				distances[j] = tmp2;

			}
		} 
	}

	for (i=0;i<K;i++) {
		neighbors[i].pitch = data_table[i].pitch;
		neighbors[i].roll = data_table[i].roll;
		neighbors[i].vyaw = data_table[i].vyaw;
		neighbors[i].vx = data_table[i].vx;
		neighbors[i].vy = data_table[i].vy;
		neighbors[i].vz = data_table[i].vz;
		neighbors[i].ax = data_table[i].ax;
		neighbors[i].ay = data_table[i].ay;
		neighbors[i].az = data_table[i].az;
		neighbors[i].class_id = data_table[i].class_id;

	}

	return neighbors;
}

int getResponse (indiv_knn neighbors[K]) {


	int resp[K];
	int response;
	int occurence = 0;
	int occurence_last = 0;
	int i;
	for (i=0;i<K;i++) {
		resp[i] = neighbors[i].class_id;
	}


	for (i=0;i<K;i++) {

		occurence = occurence_number(resp, resp[i], K);
		if (occurence > occurence_last){
			response  = resp[i];
			occurence_last = occurence;
		}
	}
	printf("knn : class_id = %d\n", response);
	return response;

}

float getAccuracy(indiv_knn * testSet, int * tab, int size) {

	float correct = 0.0;
	int i;
	for (i=0;i<size;i++){
		if (testSet[i].class_id == tab[i])
			correct++;
	}

	return (correct/(float)size) * 100.0;
}

int exists (int * tab, int val, int size) {

	int trouve = 0;

	int i = 0;
	while ((i<size) && (trouve == 0)) {
		
		if (tab[i] == val)
			trouve = 1;
		i++;
	}

	return trouve;
}

int occurence_number (int * tab, int val, int size) {

	int trouve = 0;

	int i;

	for (i=0;i<size;i++) {

		if (tab[i] == val) 
			trouve++;
	}

	return trouve;

}
/*
void main() {

	indiv_knn * matrice;

	matrice = load_data("BaseApp");

 //0.004172  0.385072  0.346904  0.224912  0.505152  0.492094  0.812270  0.042337  0.664200 
	
	indiv_knn testInstance;
	testInstance.pitch = 0.004172 ;
	testInstance.roll  = 0.385072;
	testInstance.vyaw = 0.346904;
	testInstance.vx = 0.224912;
	testInstance.vy  = 0.505152;
	testInstance.vz = 0.492094;
	testInstance.ax = 0.812270 ;
	testInstance.ay  = 0.042337;
	testInstance.az = 0.664200;


	indiv_knn * neighbors;

	neighbors = getNeighbors(matrice, testInstance);

	//printf("%f\n", neighbors[0].pitch);
	//printf("%f\n", neighbors[1].pitch);
	//printf("%f\n", neighbors[2].pitch);

	int response;

	response = getResponse(neighbors);

	

	printf("response = %d\n", response);


	
}

*/
