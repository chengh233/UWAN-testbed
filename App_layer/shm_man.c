/**
 * @file shm_man.c
 * @date Saturday, Feb 27, 2021 at 01:43:12 UTC+8
 * @author Cheng Huang (cheng_huang@zju.edu.cn)
 * 
 * This file is used for shared memory writing in the program.
 * 
 * ###### Fri Mar 5 22:32:42 CST 2021
 * @version 2.0
 * 
 **/
#include "shm_man.h"

/*
 *-----------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------
*/


// Used for initialize semaphore, indispensable for the semaphore use
int set_semvalue(int sem_id)
{

	union semun sem_union;
 
	sem_union.val = 1;

	// Error control
	if(semctl(sem_id, 0, SETVAL, sem_union) == -1){
        fprintf(stderr, "%s: Failed to set semaphore value! \n", ERROR_TITLE);
		return -1;
	}
	return 0;
}
 
// Delete semaphore
int del_semvalue(int sem_id)
{

	union semun sem_union;
 
	if(semctl(sem_id, 0, IPC_RMID, sem_union) == -1){
        fprintf(stderr, "%s: Failed to delete semaphore value! \n", ERROR_TITLE);
		return -1;
	}
	return 0;
		
}

// Decrement the semaphore by 1, that is, wait for P(sv)
static int semaphore_p(int sem_id)
{

	struct sembuf sem_b;
	sem_b.sem_num = 0;
	sem_b.sem_op = -1;                      
	sem_b.sem_flg = SEM_UNDO;
	if(semop(sem_id, &sem_b, 1) == -1)
	{
		fprintf(stderr, "%s: Function semaphore_p failed! \n", ERROR_TITLE);
		return 0;
	}
	return 1;
}
// Sned V(sv) This is a release operation, which makes the semaphore available, 
// that is, sending the signal V(sv)
static int semaphore_v(int sem_id)
{
	struct sembuf sem_b;
	sem_b.sem_num = 0;
	sem_b.sem_op = 1;
	sem_b.sem_flg = SEM_UNDO;
	if(semop(sem_id, &sem_b, 1) == -1)
	{
		fprintf(stderr, "%s: Function semaphore_v failed! \n", ERROR_TITLE);
		return 0;
	}
	return 1;
}

/* 
 *-----------------------------------------------------------------------------
 * Function: *initialize_shm
 * Description: This function is used for initialization of specific shared memory
 * Calls: shmget, shmat
 * Called By: main
 * Table Accessed: none
 * Table Updated: none
 * Input: u_int8_t key_shm
 * Output: none
 * Return: function running status
 * Others: This function will return a ptr in void type, and we need to change it
 * to shm_struct type in the main function.
 *-----------------------------------------------------------------------------
 */
int initialize_shm(key_t shm_key, shm_oper *shm_operation)
{
	// Get shared memory id with shared memory key.
    shm_operation->shm_id = shmget(shm_key, sizeof(shm_struct), 0666|IPC_CREAT);
	// Shared memory error control
	if(shm_operation->shm_id == -1)
	{
		fprintf(stderr, "%s: Running shmget failed! \n", ERROR_TITLE);
		return -1;
	}

	// Link shared memory to the address of program
	shm_operation->shm_addr = shmat(shm_operation->shm_id, (void*)0, 0);
    // Link shared memory eroor control
	if(shm_operation->shm_addr == (void*)-1)
	{
		fprintf(stderr, "%s: Running shmat failed! \n", ERROR_TITLE);
		return -1;
	}
    
	return 0;
}

/* 
 *-----------------------------------------------------------------------------
 * Function: initialize_sem
 * Description: This function is used for initializing semaphore for process
 * syscronization.
 * Calls: semget, set_semvalue
 * Called By: main
 * Table Accessed: none
 * Table Updated: none
 * Input: u_int8_t key_shm, bool set_value
 * Output: none
 * Return: int sem_id
 * Others: none
 *-----------------------------------------------------------------------------
 */
int initialize_sem(key_t sem_key, int *sem_id, bool set_value)
{
	// Get id of semaphore
    *sem_id = semget(sem_key, 1, 0666 | IPC_CREAT);
	// error control missing!
	if (*sem_id == -1){
		fprintf(stderr, "%s: Running semget failed!\n", ERROR_TITLE);
		return -1;
	}

    if (set_value)	return 0;
	else
	{
		if(set_semvalue(*sem_id) < 0)
	    {
		    fprintf(stderr, "%s: Failed to initialize semaphore\n", ERROR_TITLE);
		    return -1;
	    }
		return 0;
	}
}


/* 
 *-----------------------------------------------------------------------------
 * Function: shm_operation
 * Description: This function is designed for writting or reading shared memory/
 * Calls: memcpy
 * Called By: main
 * Table Accessed: none
 * Table Updated: none
 * Input: buf, shm, sem_id, operation
 * Output: none
 * Return: status of function
 * Others: none
 *-----------------------------------------------------------------------------
 */
int shm_operation(u_int8_t *buf, size_t data_size, shm_struct *shm, int sem_id, u_int8_t writer_id, int operation)
{
    switch (operation)
	{
	case SHM_WRITE:
	{
		// Enter protection mode, protect the reading process
	    if(!semaphore_p(sem_id))
	        exit(EXIT_FAILURE);
			
        memset(shm->data, 0 ,sizeof(shm->data));
	    // Read data and save it into buf
	    memcpy(shm->data, buf, data_size);

        // Set written status and show who wrote it
	    shm->status = writer_id;
		//memset(buf, 0 ,sizeof(buf));

	    // Leave reading protection
	    if(!semaphore_v(sem_id))
		    exit(EXIT_FAILURE);
		break;
	}


	case SHM_READ:
	{
		// Enter protection mode, protect the reading process
	    if(!semaphore_p(sem_id))
	        exit(EXIT_FAILURE);
		
		// Clear data in the buffer and shoud be 1 byte larger than data packet size for correctly outputing
        memset(buf, 0 ,data_size);
	    // Read data and save it into buf
	    memcpy(buf, shm->data, data_size);

        // Set written status and claer shared memory
	    shm->status = 0;
		//memset(shm->data, 0, sizeof(shm->data));

	    // Leave reading protection
	    if(!semaphore_v(sem_id))
		    exit(EXIT_FAILURE);
		break;
	}

	
	default:
	{
	    fprintf(stderr, "%s: Please enter right operation for shared memory!\n", ERROR_TITLE);
		return -1;
		break;
	}


	return 0;
	}
}

/* 
 *-----------------------------------------------------------------------------
 * Function: shm_delete
 * Description: Delete shared memory and semaphore
 * Calls: function list
 * Called By: function list
 * Table Accessed: table name
 * Table Updated: table name
 * Input: arguments
 * Output: arguments
 * Return: text
 * Others: text
 *-----------------------------------------------------------------------------
 */
int delete_shm_sem(shm_struct *shm, int shm_id, int sem_id)
{
    
    // Release the link between program and shared memory
	if(shmdt((void*)shm) == -1)
	{
		fprintf(stderr, "%s: app shmdt failed\n", ERROR_TITLE);
		return -1;
	}
	// Remove shared memory
	if(shmctl(shm_id, IPC_RMID, 0) == -1)
	{
		fprintf(stderr, "%s: shmctl(IPC_RMID) failed\n", ERROR_TITLE);
		return -1;
	}

	// Delete the semaphore when exitting the program
	if(del_semvalue(sem_id) == -1)
	{
		fprintf(stderr, "%s: Delete semaphore value failed!\n", ERROR_TITLE);
		return -1;
	}
    return 0;
}

