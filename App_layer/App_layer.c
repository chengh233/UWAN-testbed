/**
 * @file APP_layer.c
 * @date Wednesday, Mar 03, 2021 at 11:39:45 UTC+8
 * @author Cheng Huang (cheng_huang@zju.edu.cn)
 * 
 * This program is used for collect data from user we can transmit data by 
 * using command line or collect from data file automatically.
 * 
 * @version 1.0
 * 
 * Fri Mar 5 22:32:11 CST 2021  // press ctrl+F5 to show the time stamp :)
 * @version 2.0
 * 
 **/

// custom haeder file
#include "App_layer.h"

// ------------------------- Log file time stamp -------------------------
static char tm_stamp[28];
static FILE *log_fp = NULL;
static FILE *rx_data_fp = NULL;
static FILE *tx_data_fp = NULL;

int sleep_stat = 0;
int exit_stat = 0;
int end_rx_data = 0;

static int tx_pkt_num = 0;
char data_buf[DEFAULT_DATA_PKT_SIZE + 1];

int ini_shm(key_t shm_key, shm_oper *shm_operation, key_t sem_key, int *sem_id)
{
    int rc = 0;
    rc = initialize_shm(shm_key, shm_operation);
    if (rc < 0){
        get_tm_stamp(tm_stamp);
        fprintf(log_fp, "%s %s: Initialize shared memory for application layer failed!\n", tm_stamp, ERROR_TITLE);
        fprintf(stderr, "%s %s: Initialize shared memory for application layer failed!\n", tm_stamp, ERROR_TITLE);
        return -1;
    }
    rc = initialize_sem(sem_key, sem_id, false);
    if (rc < 0){
        get_tm_stamp(tm_stamp);
        fprintf(log_fp, "%s %s: Initialize semaphore for application layer failed!\n", tm_stamp, ERROR_TITLE);
        fprintf(stderr, "%s %s: Initialize semaphore for application layer failed!\n", tm_stamp, ERROR_TITLE);
        return -1;
    }
    return 0;
}
int v_sleep(u_int8_t *tx_buf, size_t data_size, shm_struct *tx_shm, int tx_sem_id)
{
    sleep_stat = 0;
    return 0;
}

void node_sleep(int interval)
{
    sleep_stat = 1;
    add_timer(interval, NULL, 0, NULL, 0, v_sleep);
    while(sleep_stat){
        
    }
}

int tx_data_pkt(u_int8_t *tx_buf, size_t data_pkt_size, shm_struct *tx_shm, int tx_sem_id)
{
    int rc = 0;
    char ch = 0;
    memset(data_buf, 0, sizeof(data_buf));
    if(fgets(data_buf, data_pkt_size + 2, tx_data_fp) == NULL){
        //while ((ch = getchar()) != EOF && ch != '\n');    // clean the stdin buffer, indispensable!
        
        tx_pkt_num++;
        
        if(tx_pkt_num > TX_PKT_NUM){
            get_tm_stamp(tm_stamp);
            fprintf(log_fp, "\n%s Completely send all data in the file!\n", tm_stamp);
            fprintf(stdout, "\n%s Completely send all data in the file!\n", tm_stamp);
            exit_stat = 1;
            return 0;
        }
        add_timer((int)( (double)(1 / OFFERED_LOAD) * 10), tx_buf, data_pkt_size, tx_shm, tx_sem_id, tx_data_pkt);
        return 0;

    }
    //while ((ch = getchar()) != EOF && ch != '\n');    // clean the stdin buffer, indispensable!
    memset(tx_buf, 0, DEFAULT_BUF_SIZE);
    memcpy(tx_buf, data_buf, data_pkt_size);

    

    // Save data into log file
    get_tm_stamp(tm_stamp);
    fprintf(log_fp, "%s APP_LAYER: Data entered by the user is: [%s]\n", tm_stamp, tx_buf);
    fprintf(stdout, "%s APP_LAYER: Data entered by the user is: [%s]\n", tm_stamp, tx_buf);

    tx_buf[data_pkt_size] = (u_int8_t)(0x0f & tx_buf[0]);



    if( (tx_buf[data_pkt_size] == LOCAL_ADDRESS)){

        get_tm_stamp(tm_stamp);
        fprintf(log_fp, "\n%s DATA Format ERROR! Destination address should NOT be locall address!\n", tm_stamp);
        fprintf(stdout, "\n%s DATA Format ERROR! Destination address should NOT be locall address!\n", tm_stamp);
        exit_stat = 1;
        return 0;
    }
    if( (((int)tx_buf[data_pkt_size]) <= 0 || ((int)tx_buf[data_pkt_size]) > MAX_NODE_NUM)){
        get_tm_stamp(tm_stamp);
        fprintf(log_fp, "\n%s DATA Format ERROR! Destination address should WITHIN (0, %d)!\n", tm_stamp, MAX_NODE_NUM);
        fprintf(stdout, "\n%s DATA Format ERROR! Destination address should WITHIN (0, %d)!\n", tm_stamp, MAX_NODE_NUM);
        exit_stat = 1;
        return 0;
    }

    rc = shm_operation(tx_buf, (size_t)(data_pkt_size + 1), tx_shm, tx_sem_id, APP_LAYER, SHM_WRITE);
    if (rc < 0){
        get_tm_stamp(tm_stamp);
        fprintf(log_fp, "%s APP_LAYER ERROR: Write data to MAC layer failed!\n", tm_stamp);
        fprintf(stderr, "APP_LAYER ERROR: Write data to MAC layer failed!\n");
        exit_stat = 1;
        return 0;
    }
    get_tm_stamp(tm_stamp);
    fprintf(log_fp, "\n%s DATA send to MAC layer successfully!\n", tm_stamp);
    fprintf(stdout, "\n%s DATA send to MAC layer successfully!\n", tm_stamp);

    tx_pkt_num++;
    add_timer((int)( (double)(1 / OFFERED_LOAD) * 10), tx_buf, data_pkt_size, tx_shm, tx_sem_id, tx_data_pkt);

    return 0;

}

int rx_data_pkt(u_int8_t *rx_buf, size_t data_size, shm_struct *rx_shm, int rx_sem_id)
{
    int rc = 0;
    if (rx_shm->status == MAC_LAYER && !end_rx_data)
    {
        rc = shm_operation(rx_buf, DEFAULT_BUF_SIZE, rx_shm, rx_sem_id, APP_LAYER, SHM_READ);
        if (rc < 0) {
            get_tm_stamp(tm_stamp);
            fprintf(log_fp, "%s %s: Read data from MAC layer failed!\n", tm_stamp, ERROR_TITLE);
            fprintf(stderr, "%s %s: Read data from MAC layer failed!\n", tm_stamp, ERROR_TITLE);
            return -1;
        }
        // operation for data received in the application layer
        get_tm_stamp(tm_stamp);
        fprintf(rx_data_fp, "\n%s RECEIVE DATA from MAC layer: [%s]\n", tm_stamp, rx_buf);
        fprintf(stdout, "\n%s RECEIVE DATA from MAC layer: [%s]\n", tm_stamp, rx_buf);
        //fprintf(stdout, "\nPlease enter data (%d bytes): ", DEFAULT_DATA_PKT_SIZE);
        add_timer(100, rx_buf, data_size, rx_shm, rx_sem_id, rx_data_pkt);
        return 0;
    }
    if(end_rx_data){
        return 0;
    }
    add_timer(100, rx_buf, data_size, rx_shm, rx_sem_id, rx_data_pkt);
    return 0;
}

int main(int argc, int *argv[])
{
	int rc = 0;
    char ch = 0;
    int cnt = 0;
    int re_enter_data = 0;

    char exit_p[3];

    // Set file ptr for log file
    tx_data_fp = fopen("tx_data.txt", "r");
    if(tx_data_fp == NULL){
        fprintf(stderr, "%s: OPEN data file failed!\n", ERROR_TITLE);
        return 0;
    }

    log_fp = fopen("app_layer.log", "w");
    rx_data_fp = fopen("rx_data.txt", "w");


    fprintf(log_fp, "------------------- Log file for APPLICATION layer -------------------\n\n");
    
    // Data packet parameters
    size_t data_pkt_size = 0;
    data_pkt_size = DEFAULT_DATA_PKT_SIZE;
    size_t app_pkt_size = 0;
    app_pkt_size = DEFAULT_APP_PKT_SIZE;
    
    // ------------------------- Initialize buffer-------------------------
    
    // Create transmission buffer for communication with MAC layer
	u_int8_t *tx_buf = NULL;
    size_t tx_buf_size = 0;
    tx_buf_size = DEFAULT_BUF_SIZE;
    // Allocate dynamic memory for the buffer and Initialize the buffers for application MAC layer
	tx_buf = (u_int8_t *)calloc(tx_buf_size, sizeof(u_int8_t));

    // Create reception buffer for communication with MAC layer
	u_int8_t *rx_buf = NULL;
    size_t rx_buf_size = 0;
    rx_buf_size = DEFAULT_BUF_SIZE;
    // Allocate dynamic memory for the buffer and Initialize the buffers for application MAC layer
	rx_buf = (u_int8_t *)calloc(rx_buf_size, sizeof(u_int8_t));
    
    // ------------------------- Initialize transmission shared memeory -------------------------
    shm_oper tx_shm_oper = {
        .shm_id = 0,
        .shm_addr = NULL
    };
	shm_struct *tx_shm = NULL;
    key_t tx_shm_key = 0 ;
    tx_shm_key = (key_t)KEY_OF_SHM(APP_MAC_KEY);
    int tx_sem_id = 0;
    key_t tx_sem_key = 0 ;
    tx_sem_key = (key_t)KEY_OF_SEM(APP_MAC_KEY);

    rc = ini_shm(tx_shm_key, &tx_shm_oper, tx_sem_key, &tx_sem_id);
    if (rc < 0){
        goto free_ptr;
    }
    tx_shm = tx_shm_oper.shm_addr;
    tx_shm->status = 0;
    memset(tx_shm->data, 0, sizeof(tx_shm->data));
    
    // ------------------------- Initialize reception shared memeory -------------------------
    shm_oper rx_shm_oper = {
        .shm_id = 0,
        .shm_addr = NULL
    };
	shm_struct *rx_shm = NULL;
    key_t rx_shm_key = 0 ;
    rx_shm_key = (key_t)KEY_OF_SHM(MAC_APP_KEY);
    int rx_sem_id = 0;
    key_t rx_sem_key = 0 ;
    rx_sem_key = (key_t)KEY_OF_SEM(MAC_APP_KEY);

    rc = ini_shm(rx_shm_key, &rx_shm_oper, rx_sem_key, &rx_sem_id);
    if (rc < 0){
        goto free_ptr;
    }
    rx_shm = rx_shm_oper.shm_addr;
    rx_shm->status = 0;
    memset(rx_shm->data, 0, sizeof(rx_shm->data));

    // ------------------------- Initialize successfully -------------------------
    get_tm_stamp(tm_stamp);
    fprintf(log_fp, "\n%s NODE[%02x] APP_LAYER: Application layer all set!\n", tm_stamp, LOCAL_ADDRESS);
    fprintf(stdout, "\n%s NODE[%02x] APP_LAYER: Application layer all set!\n", tm_stamp, LOCAL_ADDRESS);

    rc = ini_timer_manager();
    if(rc < 0){
       fprintf(stderr, "%s: Initialize tiemr manager failed!\n", ERROR_TITLE);
       goto free_ptr;
    }
    add_timer(100, rx_buf, DEFAULT_BUF_SIZE, rx_shm, rx_sem_id, rx_data_pkt);
    //add_timer((int)( (int)(1 / OFFERED_LOAD) * 10), tx_buf, data_pkt_size, tx_shm, tx_sem_id, tx_data_pkt);
	tx_data_pkt(tx_buf, data_pkt_size, tx_shm, tx_sem_id);
    while(1)
	{
        if(exit_stat){
            
            //node_sleep(500);

            // End receiving data from mac layer
            end_rx_data = 1;

            memcpy(exit_p, "end", 3);
            rc = shm_operation(exit_p, 3, tx_shm, tx_sem_id, APP_LAYER, SHM_WRITE);
            if (rc < 0){
                get_tm_stamp(tm_stamp);
                fprintf(log_fp, "%s APP_LAYER ERROR: Write data to MAC layer failed!\n", tm_stamp);
                fprintf(stderr, "APP_LAYER ERROR: Write data to MAC layer failed!\n");
                goto free_ptr;
            }
            get_tm_stamp(tm_stamp);
            fprintf(log_fp, "\n%s APP_LAYER: Now start exiting the program...\n", tm_stamp);
            fprintf(stdout, "\n%s APP_LAYER: Now start exiting the program...\n", tm_stamp);
            
            // Wait until MAC layer exit successfully
            node_sleep(200);
            get_tm_stamp(tm_stamp);
            fprintf(log_fp, "%s APP_LYAER: Wait for the safe exit of MAC layer...\n", tm_stamp);
            fprintf(stdout, "%s APP_LYAER: Wait for the safe exit of MAC layer...\n", tm_stamp);

            if(rx_shm->status == MAC_LAYER){
                // When MAC layer sent successful exit signal
                rc = shm_operation(rx_buf, DEFAULT_BUF_SIZE, rx_shm, rx_sem_id, APP_LAYER, SHM_READ);
                if (rc < 0) {
                    get_tm_stamp(tm_stamp);
                    fprintf(log_fp, "%s %s: Read data from MAC layer failed!\n", tm_stamp, ERROR_TITLE);
                    fprintf(stderr, "%s %s: Read data from MAC layer failed!\n", tm_stamp, ERROR_TITLE);
                    goto free_ptr;
                }
                if(memcmp(rx_buf, "emac", 4) == 0){
                    get_tm_stamp(tm_stamp);
                    fprintf(log_fp, "%s APP_LAYER: MAC layer EXIT successfully!\n", tm_stamp);
                    fprintf(stdout, "%s APP_LAYER: MAC layer EXIT successfully!\n", tm_stamp);
                    goto free_ptr;
                }
            }
            // We should never arrive here
            get_tm_stamp(tm_stamp);
            fprintf(log_fp, "%s %s: MAC layer exit failed!\n", tm_stamp, ERROR_TITLE);
            fprintf(stderr, "%s: MAC layer exit failed!\n", ERROR_TITLE);
            goto free_ptr;
        }
        /*
        // Get operation from the user
        memset(exit_p, 0, sizeof(exit_p));
        fprintf(exit_p, "\nPlease enter data (%d bytes): ", DEFAULT_DATA_PKT_SIZE);
        fgets(tx_buf, data_pkt_size + 1, stdin);
        while ((ch = getchar()) != EOF && ch != '\n');    // clean the stdin buffer, indispensable!
        
        if(memcmp(tx_buf, "rec", 3) == 0 && !re_enter_data){
            re_enter_data = 1;
        }

        if(memcmp(tx_buf, "end", 3) == 0 && !re_enter_data){
            // Clean the reception buffer
            if (rx_shm->status == MAC_LAYER)
            {
                get_tm_stamp(tm_stamp);
                fprintf(log_fp, "\n%s FIND DATA in the rx buffer! Now start saving it...\n", tm_stamp);
                fprintf(stdout, "\n%s FIND DATA in the rx buffer! Now start saving it...\n", tm_stamp);

                rc = shm_operation(rx_buf, DEFAULT_BUF_SIZE, rx_shm, rx_sem_id, APP_LAYER, SHM_READ);
                if (rc < 0) {
                    get_tm_stamp(tm_stamp);
                    fprintf(log_fp, "%s %s: Read data from MAC layer failed!\n", tm_stamp, ERROR_TITLE);
                    fprintf(stderr, "%s %s: Read data from MAC layer failed!\n", tm_stamp, ERROR_TITLE);
                    goto free_ptr;
                }
                
                // operation for data received in the application layer
                get_tm_stamp(tm_stamp);
                fprintf(rx_data_fp, "%s RECEIVE DATA from MAC layer: [%s]\n", tm_stamp, rx_buf);
                fprintf(stdout, "%s RECEIVE DATA from MAC layer: [%s]\n", tm_stamp, rx_buf);

                fprintf(log_fp, "%s RECEIVE DATA from MAC layer successfully!\n", tm_stamp);
                fprintf(stdout, "%s RECEIVE DATA from MAC layer successfully!\n", tm_stamp);
            }

            rc = shm_operation(tx_buf, 3, tx_shm, tx_sem_id, APP_LAYER, SHM_WRITE);
            if (rc < 0){
                get_tm_stamp(tm_stamp);
                fprintf(log_fp, "%s APP_LAYER ERROR: Write data to MAC layer failed!\n", tm_stamp);
                fprintf(stderr, "APP_LAYER ERROR: Write data to MAC layer failed!\n");
                goto free_ptr;
            }
            get_tm_stamp(tm_stamp);
            fprintf(log_fp, "\n%s APP_LAYER: Now start exiting the program...\n", tm_stamp);
            fprintf(stdout, "\n%s APP_LAYER: Now start exiting the program...\n", tm_stamp);
            // Wait until MAC layer exit successfully
            while(rx_shm->status != MAC_LAYER){
                cnt++;
                sleep(1);
                get_tm_stamp(tm_stamp);
                fprintf(log_fp, "%s APP_LYAER: Wait for the successful exit of MAC layer...\n", tm_stamp);
                fprintf(stdout, "%s APP_LYAER: Wait for the successful exit of MAC layer...\n", tm_stamp);

                if (cnt > 2){
                    get_tm_stamp(tm_stamp);
                    fprintf(log_fp, "%s APP_LAYER ERROR: MAC layer not respond!\n", tm_stamp);
                    fprintf(stdout, "%s APP_LAYER ERROR: MAC layer not respond!\n", tm_stamp);
                    goto free_ptr;
                }
            }
            // When MAC layer sent successful exit signal
            rc = shm_operation(rx_buf, DEFAULT_BUF_SIZE, rx_shm, rx_sem_id, APP_LAYER, SHM_READ);
            if (rc < 0) {
                get_tm_stamp(tm_stamp);
                fprintf(log_fp, "%s %s: Read data from MAC layer failed!\n", tm_stamp, ERROR_TITLE);
                fprintf(stderr, "%s %s: Read data from MAC layer failed!\n", tm_stamp, ERROR_TITLE);
                goto free_ptr;
            }
            if(memcmp(rx_buf, "emac", 4) == 0){
                get_tm_stamp(tm_stamp);
                fprintf(log_fp, "%s APP_LAYER: MAC layer EXIT successfully!\n", tm_stamp);
                fprintf(stdout, "%s APP_LAYER: MAC layer EXIT successfully!\n", tm_stamp);
                goto free_ptr;
            }

            // We should never arrive here
            get_tm_stamp(tm_stamp);
            fprintf(log_fp, "%s %s: MAC layer exit failed!\n", tm_stamp, ERROR_TITLE);
            fprintf(stderr, "%s: MAC layer exit failed!\n", ERROR_TITLE);
            goto free_ptr;

        }
        */

	}

free_ptr:

    rc = del_timer_manager();
    if(rc < 0){
       fprintf(stderr, "%s: Delete timer manager failed!\n", ERROR_TITLE);
    }

   // Release the link between program and shared memory
	if(shmdt((void*)tx_shm) == -1)
	{
		fprintf(stderr, "%s: tx_shm shmdt failed\n", ERROR_TITLE);
	}
	// Remove shared memory
	if(shmctl(tx_shm_oper.shm_id, IPC_RMID, 0) == -1)
	{
		fprintf(stderr, "%s: tx shmctl(IPC_RMID) failed\n", ERROR_TITLE);
	}
	// Delete the semaphore when exitting the program
	if(del_semvalue(tx_sem_id) == -1)
	{
		fprintf(stderr, "%s: Delete tx semaphore value failed!\n", ERROR_TITLE);
	}

   // Release the link between program and shared memory
	if(shmdt((void*)rx_shm) == -1)
	{
		fprintf(stderr, "%s: rx_shm shmdt failed\n", ERROR_TITLE);
	}
	// Remove shared memory
	if(shmctl(rx_shm_oper.shm_id, IPC_RMID, 0) == -1)
	{
		fprintf(stderr, "%s: rx shmctl(IPC_RMID) failed\n", ERROR_TITLE);
	}
	// Delete the semaphore when exitting the program
	if(del_semvalue(rx_sem_id) == -1)
	{
		fprintf(stderr, "%s: Delete rx semaphore value failed!\n", ERROR_TITLE);
	}

    free(tx_buf);
	tx_buf = NULL;
    free(rx_buf);
	rx_buf = NULL;

    get_tm_stamp(tm_stamp);
    fprintf(log_fp, "%s APP_LAYER: Exit program successfully!\n", tm_stamp); 
    fprintf(stdout, "%s APP_LAYER: Exit program successfully!\n", tm_stamp); 

    fprintf(log_fp, "\n----------------------- End of log file -----------------------\n\n");

    fclose(log_fp);
    fclose(rx_data_fp);
    fclose(tx_data_fp);
    

    return 0;
}