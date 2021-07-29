/**
 * @file timer_man.c
 * @date Thursday, Mar 18, 2021 at 22:45:17 UTC+8
 * @author Cheng Huang (cheng_huang@zju.edu.cn)
 * 
 * This is filed used for implementing multiple timer
 * in single Linux program by using timing wheels algorithm.
 * 
 * @version 1.0
 * 
 **/

#include "timer_man.h"

// Initilize global arguments
//static time_wheel tm_wheel[TIME_WHEEL_SIZE];
static time_wheel *tm_wheel = NULL;
static int cnt = 0;

static FILE *timer_fp = NULL;
static char tm_stamp[28];

// Initialize the timer manager
int ini_timer_manager()
{
    timer_fp = fopen("timer_man.log", "w");
    memset(tm_stamp, 0, sizeof(tm_stamp));

    // Initialize timing wheel
    tm_wheel = (time_wheel *)calloc(TIME_WHEEL_SIZE, sizeof(time_wheel));
    //memset(tm_wheel, 0, sizeof(tm_wheel));

    // Attach alarm signal handler to the SIGALRM signal
    signal(SIGALRM, sig_alm_handler);
    struct itimerval olditv;
    struct itimerval itv;
    // Set the interval of one time slot
    itv.it_interval.tv_sec = 0;
    itv.it_interval.tv_usec = 100000;
    // Set the time in which the timing wheel gets started
    itv.it_value.tv_sec = 1;
    itv.it_value.tv_usec = 0;
    setitimer(ITIMER_REAL, &itv, &olditv);
    return 0;
}

// Alarm signal handler function for one time slot in the timing wheel
void sig_alm_handler(int sig_num)
{
    // Rotate timing wheel
    cnt = (cnt + 1) % TIME_WHEEL_SIZE;
    int rc = 0;
    // Execute timer processing function
    rc = proc_timer();
    if(rc < 0){
        fprintf(stderr, "%s: Executing timer processing function failed!\n", ERROR_TITLE);
    }
    //printf("%s, signal number:%d, counter:%d\n", __FUNCTION__, sig_num, cnt);
}

/* 
 *-----------------------------------------------------------------------------
 * Function: add timer
 * Description: This function aims to add new timer in the timing wheel, we should
 * pass the intervals of the timer, the handler function ptr and the arguments of
 * handler function
 * Calls: calloc
 * Called By: DOTS.C
 * Table Accessed: timing wheel
 * Table Updated: timing wheel
 * Input: interval of timer, handler function ptr, arguments of handler function
 * Output: none
 * Return: function status
 * Others: none
 *-----------------------------------------------------------------------------
 */
int add_timer(int interval, u_int8_t src_dst_addr, u_int8_t cfrm_type, int (*handler)(u_int8_t, u_int8_t))
{
    int rc = 0;
    int relative_tmv = 0;
    // Compute relative interval
    relative_tmv = (cnt + interval) % TIME_WHEEL_SIZE;
    // Allocate space for new timers on the heap
    timer_node *tmr_node = NULL;
    tmr_node = (timer_node *)calloc(1, sizeof(timer_node));
    // Initialize new timer
    tmr_node->src_dst_addr = src_dst_addr;
    tmr_node->cfrm_type = cfrm_type;
    tmr_node->handler = handler;
    tmr_node->next = NULL;

    get_tm_stamp(tm_stamp);
    fprintf(timer_fp, "\n%s ADD Timer: [%d:%d]->{%02x:%02x} \n", tm_stamp, cnt, interval, tmr_node->src_dst_addr, tmr_node->cfrm_type);
    fprintf(stdout, "\n%s ADD Timer: [%d:%d]->{%02x:%02x} \n", tm_stamp, cnt, interval, tmr_node->src_dst_addr, tmr_node->cfrm_type);

    // If timer queue in one time slot is empty
    if(tm_wheel[relative_tmv].front == NULL){
        tm_wheel[relative_tmv].front = tmr_node;
        tm_wheel[relative_tmv].rear = tm_wheel[relative_tmv].front;
        return 0;
    }
    // Add new timers to the end of timer queue in the specific time slot
    else{
        tm_wheel[relative_tmv].rear->next = tmr_node;
        tm_wheel[relative_tmv].rear = tmr_node;
        return 0;
    }
    return -1;
}

/* 
 *-----------------------------------------------------------------------------
 * Function: process timer
 * Description: This function aims to process all the timers in one time slot(0.1s)
 * Calls: none
 * Called By: sig_alm_handler
 * Table Accessed: time wheel
 * Table Updated: none
 * Input: none
 * Output: none
 * Return: function status
 * Others: none
 *-----------------------------------------------------------------------------
 */
int proc_timer()
{
    int rc  = 0;
    // If there is any timer set in this time slot
    if(tm_wheel[cnt].front != NULL){
        timer_node *tmr_node = NULL;
        tmr_node = tm_wheel[cnt].front;
        get_tm_stamp(tm_stamp);
        fprintf(timer_fp, "\n%s PROCESS AND REMOVE Timer: [%d]->{%02x:%02x} \n", tm_stamp, cnt, tmr_node->src_dst_addr, tmr_node->cfrm_type);
        fprintf(stdout, "\n%s PROCESS AND REMOVE Timer: [%d]->{%02x:%02x} \n", tm_stamp, cnt, tmr_node->src_dst_addr, tmr_node->cfrm_type);

        rc = tmr_node->handler(tmr_node->src_dst_addr, tmr_node->cfrm_type);
        if(rc < 0){
            fprintf(stderr, ": Running time wheel callback function failed!\n");
            return -1;
        }
        // Check if there is any ohter timers set in this time slot
        while(tmr_node->next != NULL){
            tmr_node = tmr_node->next;
            get_tm_stamp(tm_stamp);
            fprintf(timer_fp, "\n%s PROCESS AND REMOVE Timer: [%d]->{%02x:%02x} \n", tm_stamp, cnt, tmr_node->src_dst_addr, tmr_node->cfrm_type);
            fprintf(stdout, "\n%s PROCESS AND REMOVE Timer: [%d]->{%02x:%02x} \n", tm_stamp, cnt, tmr_node->src_dst_addr, tmr_node->cfrm_type);

            rc = tmr_node->handler(tmr_node->src_dst_addr, tmr_node->cfrm_type);
            free(tm_wheel[cnt].front);
            tm_wheel[cnt].front = tmr_node;
            if(rc < 0){
                fprintf(stderr, ": Running time wheel callback function failed!\n");
                return -1;
            }
        }
        free(tm_wheel[cnt].front);
        tm_wheel[cnt].front = NULL;
        tm_wheel[cnt].rear = NULL;
        return 0;
    }
    return 0;
}

/* 
 *-----------------------------------------------------------------------------
 * Function: delete time wheel and timer manager
 * Description: This function aims to delete all the timer in the time wheel
 * and delete timer manager
 * Calls: none
 * Called By: main
 * Table Accessed: time wheel
 * Table Updated: none
 * Input: none
 * Output: none
 * Return: function status
 * Others: none
 *-----------------------------------------------------------------------------
 */
void del_time_wheel()
{
    int i = 0;
    timer_node *tmr_node = NULL;
    for(i = 0; i < TIME_WHEEL_SIZE; i++){
        if(tm_wheel[i].front != NULL){
            tmr_node = tm_wheel[i].front;
            while(tmr_node->next != NULL){
                tmr_node = tmr_node->next;
                get_tm_stamp(tm_stamp);
                fprintf(timer_fp, "\n%s ONLY REMOVE Timer: [%d]->{%02x:%02x} \n", tm_stamp, cnt, tmr_node->src_dst_addr, tmr_node->cfrm_type);
                fprintf(stdout, "\n%s ONLY REMOVE Timer: [%d]->{%02x:%02x} \n", tm_stamp, cnt, tmr_node->src_dst_addr, tmr_node->cfrm_type);
                free(tm_wheel[i].front);
                tm_wheel[i].front = tmr_node;
            }
            get_tm_stamp(tm_stamp);
            fprintf(timer_fp, "\n%s ONLY REMOVE Timer: [%d]->{%02x:%02x} \n", tm_stamp, cnt, tmr_node->src_dst_addr, tmr_node->cfrm_type);
            fprintf(stdout, "\n%s ONLY REMOVE Timer: [%d]->{%02x:%02x} \n", tm_stamp, cnt, tmr_node->src_dst_addr, tmr_node->cfrm_type);
            free(tm_wheel[i].front);
            tm_wheel[i].front = NULL;
            tm_wheel[i].rear = NULL;
        }
    }
}

int del_timer_manager()
{
    get_tm_stamp(tm_stamp);
    fprintf(timer_fp, "\n%s Start deleting timer manager:\n", tm_stamp);
    fprintf(stdout, "\n%s Start deleting timer manager:\n", tm_stamp);

    del_time_wheel();
    setitimer(ITIMER_REAL, NULL, NULL);
    free(tm_wheel);
    tm_wheel = NULL;

    get_tm_stamp(tm_stamp);
    fprintf(timer_fp, "\n%s Delete timer manager successfully!\n", tm_stamp);
    fprintf(stdout, "\n%s Delete timer manager successfully!\n", tm_stamp);
    fclose(timer_fp);
    return 0;
}