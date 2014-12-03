#ifndef _PREDICT_H
#define _PREDICT_H

struct s_specimen{
    float pitch;
    float roll;
    float vyaw;
    float vx;
    float vy;
    float vz;
    float ax;
    float ay;
    float az;
}typedef specimen;

struct s_predict_results{
	int confidence;
	int predict_class;
}typedef predict_results;

extern specimen specimen_buffer[10];

predict_results recognition_process(specimen* buffer, char* training_model);

#endif /* _PREDICT_H */
