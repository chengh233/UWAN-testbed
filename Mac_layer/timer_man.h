/**
 * @file timer_man.h
 * @date Thursday, Mar 18, 2021 at 22:46:26 UTC+8
 * @author Cheng Huang (cheng_huang@zju.edu.cn)
 * 
 * This is the file contains header files for mult_timer.c
 * 
 * @version 1.0
 * 
 **/


#ifndef TIMER_MAN_H_
#define TIMER_MAN_H_ 1

#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>

#include "util.h"

// ------------------------- Definition for timing wheels -------------------------
#define TIME_WHEEL_SIZE               500

// ------------------------- Struct for timer nodes and timing wheels -------------------------
struct timer_node_struct
{
    u_int8_t src_dst_addr;
    u_int8_t cfrm_type;
    int (*handler)(u_int8_t, u_int8_t);
    struct timer_node_struct *next;
};
typedef struct timer_node_struct timer_node;

struct time_wheel_struct
{
    timer_node *front;
    timer_node *rear;
};
typedef struct time_wheel_struct time_wheel;

// ------------------------- Global arguments for timing wheels -------------------------
static time_wheel *tm_wheel;
static int cnt;

static FILE *timer_fp;
static char tm_stamp[28];

// ------------------------- Function declaration -------------------------
int ini_timer_manager();
void sig_alm_handler(int sig_num);
int add_timer(int interval, u_int8_t src_dst_addr, u_int8_t cfrm_type, int (*handler)(u_int8_t, u_int8_t));
int proc_timer();
void del_time_wheel();
int del_timer_manager();

#endif // !TIMER_MAN_H_
