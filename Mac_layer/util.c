/**
 * @file util.c
 * @date Wednesday, Mar 10, 2021 at 10:57:47 UTC+8
 * @author Cheng Huang (cheng_huang@zju.edu.cn)
 * 
 * This file is used for generating time stamp for log file
 * and mac control frame.
 * 
 * @version 1.0
 * 
 **/

#include "util.h"

void get_tm_stamp(char *tm_stmp)
{
    // Set time stamp for data and log
    struct timeval tm_val;
    struct tm *tm_st;
    gettimeofday(&tm_val, NULL);
    tm_st = localtime(&tm_val.tv_sec);

    sprintf(tm_stmp, "[%04d-%02d-%02d %02d:%02d:%02d.%04ld]",\
    tm_st->tm_year + 1900,tm_st->tm_mon + 1,tm_st->tm_mday,\
    tm_st->tm_hour,tm_st->tm_min,tm_st->tm_sec,tm_val.tv_usec / 100);
}

void get_cfrm_tmv_stmp(u_int8_t *tmv_stmp)
{
    // Set time stamp for control frame in the mac layer
    struct timeval tm_val;
    gettimeofday(&tm_val, NULL);
    // Get time stamp, 0 - 6553.5 (s), 2 bytes
    tmv_stmp[0] = (u_int8_t)( ( ( (tm_val.tv_sec%10000)*10+(tm_val.tv_usec/100000) ) % 65535) >> 8);
    tmv_stmp[1] = (u_int8_t)( ( (tm_val.tv_sec%10000)*10+(tm_val.tv_usec/100000) ) % 65535);
}

u_int16_t get_cur_tmv_stmp()
{
    u_int16_t tmv_stmp;
    struct timeval tm_val;
    gettimeofday(&tm_val, NULL);
    tmv_stmp = (u_int16_t)(( (tm_val.tv_sec % 10000) * 10 + (tm_val.tv_usec / 100000) ) % 65535);
    return tmv_stmp;
}

u_int8_t swap_addr(u_int8_t addr)
{
	u_int8_t swapped_addr = addr;
	u_int8_t temp;
	temp = (u_int8_t)(addr & 0x0f);
	swapped_addr = (u_int8_t)(((swapped_addr & 0xf0) >> 4) + (temp << 4));
	return swapped_addr;
}
