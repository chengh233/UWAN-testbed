/**
 * @file dly_map_man.h
 * @date Saturday, Mar 20, 2021 at 21:05:08 UTC+8
 * @author Cheng Huang (cheng_huang@zju.edu.cn)
 * 
 * This file contains the heafer files for dly_map_man.c
 * 
 * @version 1.0
 * 
 **/

#ifndef DLY_MAP_MAN_H_
#define DLY_MAP_MAN_H_ 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "util.h"

// ------------------------- Definition of delay map and collsion map -------------------------
#define DELAY_MAP_SIZE                ((int)( (MAX_NODE_NUM * MAX_NODE_NUM) - 2 ))

// ------------------------- Definition of DOTS conflict detection schedules -------------------------
#define MAX_PROP_DELAY                ((u_int16_t)150)               // Maximum propagation delay 0.1 * 150 = 15s     
#define CFRM_GUARD_TIME               ((u_int16_t)25)                // Reception guard time 0.1 * 5 = 0.5s   
#define DATA_GUARD_TIME               ((u_int16_t)65)                // Reception guard time 0.1 * 5 = 0.5s   

// ------------------------- Struct for delay map and collision map -------------------------
// struct of nodes in the collision map
struct collision_map_node
{
    struct collision_map_node *prior;
    u_int8_t src_dst_addr;
    u_int8_t cfrm_type;
    u_int16_t tmv_stmp;
    struct collision_map_node *next;
};
typedef struct collision_map_node collision_node;

// struct of nodes in the delay map
struct delay_map_node
{
    u_int8_t delay;
    struct collision_map_node *front;
    struct collision_map_node *rear;
};
typedef struct delay_map_node delay_map;

//delay_map dly_map[DELAY_MAP_SIZE];
delay_map *dly_map;
static FILE *dly_map_fp;
static char tm_stamp[28];


// ------------------------- Function for delay map and collision map -------------------------
int ini_dly_map();
int dly_harsh(u_int8_t src_dst_addr);
int add_coll_node(u_int8_t src_dst_addr, u_int8_t cfrm_type, u_int16_t tmv_stmp);
int ck_collision(u_int8_t src_dst_addr, collision_node *coll_node);
int ck_coll_map(u_int8_t src_dst_addr);
int del_coll_node(collision_node *coll_node);
int del_dly_map();


#endif // DLY_MAP_MAN_H_