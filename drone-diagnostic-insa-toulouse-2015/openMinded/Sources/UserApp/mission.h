#ifndef MISSION_H

#define MISSION_H



#include <linux/input.h>

#include <VP_Api/vp_api_thread_helper.h> // api pour les threads

#include "/home/zuser/ShaggyDog/drone-diagnostic-insa-toulouse-2014/smartfox/Sources/Navdata/navdata_analyse.h"

#include "Model/model.h"

#include "Model/residue.h"



float roll(float value, int us);

float pitch(float value, int us);

float yaw(float value, int us);

float gas(float value, int us);

float hover(int us);



PROTO_THREAD_ROUTINE(mission, data);





#endif