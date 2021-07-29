/**
 * @file Mac_layer.h
 * @date Thursday, Mar 04, 2021 at 11:57:45 UTC+8
 * @author Cheng Huang (cheng_huang@zju.edu.cn)
 * 
 * This is the header file     
 * 
 * @version 1.0
 * 
 **/

#ifndef MAC_LAYER_H_
#define MAC_LAYER_H_ 1

// Default head files
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
// types control
#include <sys/types.h>
#include <stdbool.h>

// custom haeder file
#include "shm_man.h"
#include "util.h"
#include "axidma_DOTS.h"

// ------------------------- Definition for mac layer and node-------------------------   #define DEFAULT_NET_BUF_SIZE         ((int)(1025 * sizeof(u_int8_t)))         // buffer size for NETWORK layer
#define DEFAULT_NET_BUF_SIZE             ((int)(1025 * sizeof(u_int8_t)))         // buffer size for PHYSICAL layer
#define DEFAULT_PHY_BUF_SIZE             ((int)(1030 * sizeof(u_int8_t)))         // buffer size for PHYSICAL layer

// ------------------------- Definition for initializing shared memory -------------------------
#define SHM_KEY                      ((u_int8_t)0x00)                             // key component of shared memory
#define SEM_KEY                      ((u_int8_t)0x01)                             // key component of semaphore

#define APP_MAC_KEY                  ((u_int8_t)0x41)                             // key component of shared memory between network layer and mac layer
#define MAC_APP_KEY                  ((u_int8_t)0x14)                             // key component of shared memory between network layer and mac layer

#define KEY_OF_SHM(key)              ((LOCAL_ADDRESS<<16)+(key<<8)+SHM_KEY)     // key of shared memory for cthe orresponding layer
#define KEY_OF_SEM(key)              ((LOCAL_ADDRESS<<16)+(key<<8)+SEM_KEY)     // key of semaphore for the corresponding layer

#define KEY_OF_SHM_N(area_id, key)            ((area_id<<16)+(key<<8)+SHM_KEY)     // key of shared memory for cthe orresponding layer
#define KEY_OF_SEM_N(area_id, key)            ((area_id<<16)+(key<<8)+SEM_KEY)     // key of semaphore for the corresponding layer

// ------------------------- Global arguments -------------------------
static char tm_stamp[28];
static FILE *log_fp;
int rx_sleep_stat;

int ini_shm(key_t shm_key, shm_oper *shm_operation, key_t sem_key, int *sem_id);
int rx_v_sleep(u_int8_t src_dst_addr, u_int8_t cfrm_type);


#endif // MAC_LAYER_H_

