/**
 * @file Net_layer.h
 * @date Tuesday, Mar 02, 2021 at 11:29:06 UTC+8
 * @author Cheng Huang (cheng_huang@zju.edu.cn)
 * 
 * @version 2.0
 * 
 **/

#ifndef APP_LAYER_H_
#define APP_LAYER_H_ 1

// Default head files
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
// types control
#include <sys/types.h>
#include <stdbool.h>
// command line control
#include <getopt.h>

#include "shm_man.h"
#include "util.h"
#include "timer_man.h"

// ------------------------- Definition for application layer and node -------------------------
#define DEFAULT_BUF_SIZE             ((int)(1025 * sizeof(u_int8_t)))         // buffer size for PHYSICAL layer

// ------------------------- Definition for initializing shared memory -------------------------
#define SHM_KEY                      (u_int8_t)0x00                             // key component of shared memory
#define SEM_KEY                      (u_int8_t)0x01                             // key component of semaphore

#define APP_MAC_KEY                  (u_int8_t)0x41                             // key component of shared memory between network layer and mac layer
#define MAC_APP_KEY                  (u_int8_t)0x14                             // key component of shared memory between network layer and mac layer

#define KEY_OF_SHM(key)              ((LOCAL_ADDRESS<<16)+(key<<8)+SHM_KEY)     // key of shared memory for cthe orresponding layer
#define KEY_OF_SEM(key)              ((LOCAL_ADDRESS<<16)+(key<<8)+SEM_KEY)     // key of semaphore for the corresponding layer

#define KEY_OF_SHM_U(key)            ((PHY_LAYER<<16)+(key<<8)+SHM_KEY)     // key of shared memory for cthe orresponding layer
#define KEY_OF_SEM_U(key)            ((PHY_LAYER<<16)+(key<<8)+SEM_KEY)     // key of semaphore for the corresponding layer

#define OFFERED_LOAD                 0.02                                        // packet per second
#define TX_PKT_NUM                   10

// ------------------------- Global arguments -------------------------
static char tm_stamp[28];
static FILE *log_fp;
static FILE *rx_data_fp;
static FILE *tx_data_fp;
int sleep_stat;
int exit_stat;
int end_rx_data;

char data_buf[DEFAULT_DATA_PKT_SIZE + 1];
static int tx_pkt_num;

// ------------------------- Function declaration -------------------------
int ini_shm(key_t shm_key, shm_oper *shm_operation, key_t sem_key, int *sem_id);
int v_sleep(u_int8_t *tx_buf, size_t data_size, shm_struct *tx_shm, int tx_sem_id);
void node_sleep(int interval);
int tx_data_pkt(u_int8_t *tx_buf, size_t data_size, shm_struct *tx_shm, int tx_sem_id);
int rx_data_pkt(u_int8_t *rx_buf, size_t data_size, shm_struct *rx_shm, int rx_sem_id);

#endif // APP_LAYER_H_

// ------------------------- function and arguments Naming Convention -------------------------
// function: verb+non
// argument: adj+non
