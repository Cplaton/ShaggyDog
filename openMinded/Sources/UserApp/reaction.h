/**
 * @file    reaction.h
 * @author  ShaggyDogs
 * @brief   Reaction Module
 * @version 1.0
 * @date    December 2014
 **/

#ifndef REACTION_H
#define REACTION_H


#include <linux/input.h>
#include <VP_Api/vp_api_thread_helper.h> // api pour les threads
#include "Navdata/navdata_analyse.h"
#include "Model/model.h"
#include "Model/residue.h"
#include "UI/configurePage.h"

/**
 * @brief   shared variable that enable/disable the reaction module of openMINDED
 **/
extern int enable_openMinded_safety_mode;
extern int modeReaction;

PROTO_THREAD_ROUTINE(reaction, data);

/**
 * @brief   Reaction to a frontal impact with a wall.
 **/
void avoid_front_wall();

/**
 * @brief  Reaction to a wall impact on the left.
 **/
void avoid_left_wall();

/**
 * @brief  Reaction to an impact on the back of the drone.
 **/
void avoid_back_wall();

/**
 * @brief Reaction to a wall impact ont the right.
 **/
void avoid_right_wall();

/**
 * @brief In case of impact run the adapted reaction's function.
 **/
void check_situation();


#endif
