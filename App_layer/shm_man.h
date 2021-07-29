/**
 * @file shm_man.h
 * @date Sat Feb 27 24:41:23 CST 2021
 * @author Cheng Huang (lullaby, cheng_huang@zju.edu.cn)
 * 
 * This file is the header for shm_write.c
 * 
 * ###### Fri Mar 5 22:32:52 CST 2021
 * @version 2.0
 * 
 **/

#ifndef SHM_MAN_H_
#define SHM_MAN_H_ 1

// ------------------------- Defarult head files -------------------------
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdbool.h>
#include <errno.h>              

#include <string.h>
#include <sys/shm.h>
#include <sys/sem.h>

#include "util.h"
/*
 *-----------------------------------------------------------------------------
 * Initial Definitions
 * ----------------------------------------------------------------------------
*/

// ------------------------- Definition for shared memory struct -------------------------
#define SHM_DATA_SIZE            1030                  // data size in the shared memory

// ------------------------- Definition for shared memory operation -------------------------
#define SHM_WRITE                1                     // mark of writting operation to shared memory
#define SHM_READ                 0                     // mark of reading operation  to shared memory

// ------------------------- Struct for shared memory -------------------------
struct shareuse_struct
{
	u_int8_t status;                                    // !0 shows one who wrote it 0 shows we can write it
	u_int8_t data[SHM_DATA_SIZE];                           // Save the text that is read and written
};
typedef struct shareuse_struct shm_struct;

struct shm_oper_struct
{
	int shm_id;
	void *shm_addr;
};
typedef struct shm_oper_struct shm_oper;

// Semaphore union (optional, not used in this program)
union semun
{
	int val;
	struct semid_ds *buf;
	unsigned short *arry;
};

// ------------------------- Global arguments -------------------------

// ------------------------- Functions for shared memory -------------------------
// Functions for Process Synchronization
int set_semvalue(int sem_id);
int del_semvalue(int sem_id);
static int semaphore_p(int sem_id);
static int semaphore_v(int sem_id);

// Writting function
int initialize_shm(key_t shm_key, shm_oper *shm_operation);
int initialize_sem(key_t sem_key, int *sem_id, bool set_value);
int shm_operation(u_int8_t *buf, size_t data_size, shm_struct *shm, int sem_id, u_int8_t writer_id, int operation);
int delete_shm_sem(shm_struct *shm, int shm_id, int sem_id);

#endif // !SHM_MAN_H_
