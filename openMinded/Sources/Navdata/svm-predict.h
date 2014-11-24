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

extern specimen specimen_buffer[10];

int recognition_process(specimen specimen_buffer[10], char* training_model, char *output);

#endif /* _PREDICT_H */
