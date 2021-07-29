/**
 * @file util.h
 * @date Wednesday, Mar 10, 2021 at 11:01:31 UTC+8
 * @author Cheng Huang (cheng_huang@zju.edu.cn)
 * 
 * This is the header file for util.c
 * 
 * @version 1.0
 * 
 **/

#ifndef UTIL_H_
#define UTIL_H_ 1

#include <stdio.h>
#include <stdbool.h> 
#include <stdlib.h>
#include <sys/types.h>       // data type such as unsigned long
#include <unistd.h>          // provide POSIX (Portable Operating System Interface--uniX) operation system API
#include <sys/time.h>        
#include <time.h>
#include <string.h>

// ------------------------- Definition for network -------------------------
#define MAX_NODE_NUM                 4                                     // maximum number of adjant nodes
#define MAX_RETX_NUM                 2

// ------------------------- Definition for protocol -------------------------
#define DEFAULT_DATA_PKT_SIZE        ((int)(  20 * sizeof(u_int8_t)))
#define DEFAULT_APP_PKT_SIZE         ((int)(  21 * sizeof(u_int8_t)))            // data paket size
#define DEFAULT_CFRM_SIZE            ((int)(  5 * sizeof(u_int8_t)))

#define CFRM_RTS                      ((u_int8_t)0x00)
#define CFRM_CTS                      ((u_int8_t)0x01)
#define CFRM_DATA                     ((u_int8_t)0x02)
#define CFRM_ACK                      ((u_int8_t)0x03)

// ------------------------- Definition for node -------------------------
#define LOCAL_ADDRESS                (u_int8_t)0x01                        // local address of node in the network 0x00 for broadcasting
#define BROADCAST_ADDRESS            (u_int8_t)0x00

#define APP_LAYER                    (u_int8_t)0xa4                             // Id of application layer
#define TXP_LAYER                    (u_int8_t)0xa3                             // Id of application layer
#define NET_LAYER                    (u_int8_t)0xa2                             // Id of mac(datalink) layer
#define MAC_LAYER                    (u_int8_t)0xa1                             // Id of application layer
#define PHY_LAYER                    (u_int8_t)0xa0                             // Id of application layer

#define DEFAULT_BUF_SIZE             ((int)(1025 * sizeof(u_int8_t)))         // buffer size for PHYSICAL layer

// ------------------------- Definition for Error control -------------------------
#define ERROR_TITLE  "APP_LAYER ERROR"

// ------------------------- Function declaration -------------------------
void get_tm_stamp(char *tm_stmp);
void get_cfrm_tmv_stmp(u_int8_t *tmv_stmp);
u_int16_t get_cur_tmv_stmp();
u_int8_t swap_addr(u_int8_t addr);

#endif // UTIL_H_