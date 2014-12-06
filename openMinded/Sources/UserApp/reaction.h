#ifndef REACTION_H
#define REACTION_H


#include <linux/input.h>
#include <VP_Api/vp_api_thread_helper.h> // api pour les threads
#include "Navdata/navdata_analyse.h"
#include "Model/model.h"
#include "Model/residue.h"
#include "UI/configurePage.h"

extern int enable_openMinded_safety_mode;

PROTO_THREAD_ROUTINE(reaction, data);

void avoid_front_wall();
void avoid_left_wall();
void avoid_back_wall();
void avoid_right_wall();
void check_situation();


#endif