/**
 * @file axidma_DOTS.h
 * @date Thursday, Mar 04, 2021 at 02:01:59 UTC+8
 * @author Cheng Huang (cheng_huang@zju.edu.cn)
 * 
 * This is the header file for axidma_DOTS.c
 * 
 * @version 1.0
 * 
 **/

#ifndef AXIDMA_DOTS_H_
#define AXIDMA_DOTS_H_ 1
// Default head files
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdbool.h>

#include "shm_man.h"
#include "timer_man.h"
#include "pktq_man.h"
#include "dly_map_man.h"

#include "util.h"

// ------------------------- Definition of control frame size and type -------------------------
#define WAIT_FB_TIME                  ((int)275)
#define BACK_OFF_TIME(RTS_tx_num)     ((int)(((rand()%40)+40)*RTS_tx_num))

// ------------------------- Structs of network packet and mac control frame-------------------------
// struct of the packet from mac layer
struct mac_control_frame_struct
{
    u_int8_t src_dst_addr;
    u_int8_t delay;
    u_int8_t type;
    u_int16_t tmv_stmp;
};
typedef struct mac_control_frame_struct mac_frame;

static FILE *DOTS_fp;
static char *log_path;
static char tm_stamp[28];

static u_int8_t *g_phy_tx_buf;
static shm_struct *g_phy_tx_shm;
static int g_phy_tx_sem_id;

static u_int8_t *g_net_tx_buf;
static shm_struct *g_net_tx_shm ;
static int g_net_tx_sem_id;
int tx_sleep_stat;

static FILE *throughput_fp;
// ------------------------- Functions in the DOTS protocol -------------------------
int tx_v_sleep(u_int8_t src_dst_addr, u_int8_t cfrm_type);
// Parse packet received
int ini_DOTS(u_int8_t *net_tx_buf, shm_struct *net_tx_shm, int net_tx_sem_id, u_int8_t *phy_tx_buf, shm_struct *phy_tx_shm, int phy_tx_sem_id);
void parse_cfrm(u_int8_t *buf, mac_frame *cfrm);
int proc_cfrm(mac_frame *cfrm, u_int8_t *rx_buf);
int send_frm(u_int8_t src_dst_addr, u_int8_t cfrm_type);
int wait_cfrm_fdbk(u_int8_t src_dst_addr, u_int8_t cfrm_type);
int back_off(u_int8_t src_dst_addr, u_int8_t cfrm_type);
int mac_DOTS(u_int8_t *rx_buf, size_t rx_data_size, int rx_layer_id);
int Close_DOTS();

#endif