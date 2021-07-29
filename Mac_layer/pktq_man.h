/**
 * @file pkt_man.h
 * @date Thursday, Mar 18, 2021 at 22:49:56 UTC+8
 * @author Cheng Huang (cheng_huang@zju.edu.cn)
 * 
 * This file contains header files for pkt_man.c
 * 
 * @version 1.0
 * 
 **/


#ifndef PKTQ_MAN_H_
#define PKTQ_MAN_H_ 1

#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>

#include "util.h"
// ------------------------- Definition for packet control -------------------------
#define RTS_TX_NUM                    0         // RTS transmission number
#define CTS_RX_STAT                   1         // CTS reception status
#define CTS_RX_CK_STAT                2         // CTS reception checking status, to avoid duplicating checking
#define ACK_RX_STAT                   3         // ACK reception status

// ------------------------- Struct for packet queue node and node manager -------------------------
struct pkt_queue_node_struct
{
    struct pkt_queue_node_struct *prior;
    u_int8_t dst_addr;
    u_int8_t DATA[DEFAULT_DATA_PKT_SIZE];
    int send_stat;
    int backoff_stat;
    int cfrm_stat[4];
    struct pkt_queue_node_struct *next;
};
typedef struct pkt_queue_node_struct pkt_queue_node;

struct node_manage_struct
{
    pkt_queue_node *front;
    pkt_queue_node *rear;
    int update_stat;
};
typedef struct node_manage_struct node_manage;

// ------------------------- Global arguments for packet queue manager -------------------------
//node_manage node_man[MAX_NODE_NUM];
node_manage *node_man;
static FILE *pkt_q_fp;
static char tm_stamp[28];

// ------------------------- Function declaration -------------------------
int ini_node_manage();
int add_pkt(u_int8_t *buf, size_t data_size);
int del_pkt(pkt_queue_node *pkt_node);
//int del_node_manager();
//void del_node_manager();
int del_node_manager();

#endif // !PKTQ_MAN_H_
