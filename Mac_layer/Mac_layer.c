/**
 * @file Mac_layer.c
 * @date Thursday, Mar 04, 2021 at 00:58:25 UTC+8
 * @author Cheng Huang (cheng_huang@zju.edu.cn)
 * 
 * This program is used for implement of the protocol
 * in the mac layer.
 * 
 * @version 1.0
 * 
 **/


#include "Mac_layer.h"

// ------------------------- Log file time stamp -------------------------
static char tm_stamp[28];
static FILE *log_fp;
extern delay_map *dly_map;
extern node_manage *node_man;
int rx_sleep_stat = 0;

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

int rx_v_sleep(u_int8_t src_dst_addr, u_int8_t cfrm_type)
{
    rx_sleep_stat = 0;
    return 0;
}



/* 
 *-----------------------------------------------------------------------------
 * Function: main
 * Description: This main funciton is designed for mac layer, we can run the
 * protocol of mac layer here.
 * Calls: shared memory create, semaphore create, shared memory operation, axi-dma
 * operation
 * Called By: none
 * Table Accessed: delay map, collision map
 * Table Updated: none
 * Input: none
 * Output: none
 * Return: -1 failure, 0 success
 * Others: none
 *-----------------------------------------------------------------------------
 */

int main(int argc, int *argv[])
{
	int rc = 0;
    int cnt = 0;
    int drop_fram = 0;
    // Set file ptr for log file
    log_fp = fopen("mac_layer.log", "w");
    fprintf(log_fp, "------------------- Log file for DATALINK layer -------------------\n\n");

    size_t data_pkt_size = 0;
    data_pkt_size = DEFAULT_DATA_PKT_SIZE;
	size_t net_pkt_size = 0;
	net_pkt_size = DEFAULT_APP_PKT_SIZE;
    size_t mac_cfrm_size = 0;
    mac_cfrm_size = DEFAULT_CFRM_SIZE;

    // ------------------------- Buffer and shared memory for NETWORK layer initialization -------------------------
    // Create buffer for communication with MAC layer
	u_int8_t *net_tx_buf = NULL;
    size_t net_tx_buf_size = 0;
    net_tx_buf_size = DEFAULT_NET_BUF_SIZE;
    // Allocate dynamic memory for the buffer and Initialize the buffers for netlication MAC layer
	net_tx_buf = (u_int8_t *)calloc(net_tx_buf_size, sizeof(u_int8_t));

    // Create buffer for communication with MAC layer
	u_int8_t *net_rx_buf = NULL;
    size_t net_rx_buf_size = 0;
    net_rx_buf_size = DEFAULT_NET_BUF_SIZE;
    // Allocate dynamic memory for the buffer and Initialize the buffers for netlication MAC layer
	net_rx_buf = (u_int8_t *)calloc(net_rx_buf_size, sizeof(u_int8_t));

    // Create struct for saving arguments of shared memory
    shm_oper net_tx_shm_oper = {
        .shm_id = 0,
        .shm_addr = NULL
    };
	shm_struct *net_tx_shm = NULL;
    key_t net_tx_shm_key = 0 ;
    net_tx_shm_key = (key_t)KEY_OF_SHM(MAC_APP_KEY);
    int net_tx_sem_id = 0;
    key_t net_tx_sem_key = 0 ;
    net_tx_sem_key = (key_t)KEY_OF_SEM(MAC_APP_KEY);

    rc = ini_shm(net_tx_shm_key, &net_tx_shm_oper, net_tx_sem_key, &net_tx_sem_id);
    if (rc < 0){
        goto free_ptr;
    }
    net_tx_shm = net_tx_shm_oper.shm_addr;

    // Create struct for saving arguments of shared memory
    shm_oper net_rx_shm_oper = {
        .shm_id = 0,
        .shm_addr = NULL
    };
	shm_struct *net_rx_shm = NULL;
    key_t net_rx_shm_key = 0 ;
    net_rx_shm_key = (key_t)KEY_OF_SHM(APP_MAC_KEY);
    int net_rx_sem_id = 0;
    key_t net_rx_sem_key = 0 ;
    net_rx_sem_key = (key_t)KEY_OF_SEM(APP_MAC_KEY);

    rc = ini_shm(net_rx_shm_key, &net_rx_shm_oper, net_rx_sem_key, &net_rx_sem_id);
    if (rc < 0){
        goto free_ptr;
    }
    net_rx_shm = net_rx_shm_oper.shm_addr;

    // ------------------------- Buffer and shared memory for PHYSICAL layer initialization -------------------------
    // Create buffer for communication with MAC layer
	u_int8_t *phy_tx_buf = NULL;
    size_t phy_tx_buf_size = 0;
    phy_tx_buf_size = DEFAULT_PHY_BUF_SIZE;
    // Allocate dynamic memory for the buffer and Initialize the buffers for phylication MAC layer
	phy_tx_buf = (u_int8_t *)calloc(phy_tx_buf_size, sizeof(u_int8_t));

    // Create buffer for communication with MAC layer
	u_int8_t *phy_rx_buf = NULL;
    size_t phy_rx_buf_size = 0;
    phy_rx_buf_size = DEFAULT_PHY_BUF_SIZE;
    // Allocate dynamic memory for the buffer and Initialize the buffers for phylication MAC layer
	phy_rx_buf = (u_int8_t *)calloc(phy_rx_buf_size, sizeof(u_int8_t));

    // Create struct for saving arguments of shared memory
    shm_oper phy_tx_shm_oper = {
        .shm_id = 0,
        .shm_addr = NULL
    };
	shm_struct *phy_tx_shm = NULL;
    key_t phy_tx_shm_key = 0 ;
    phy_tx_shm_key = (key_t)KEY_OF_SHM_N(SEA_AREA_A, ADJACENT_ADDRESS_2);
    int phy_tx_sem_id = 0;
    key_t phy_tx_sem_key = 0 ;
    phy_tx_sem_key = (key_t)KEY_OF_SEM_N(SEA_AREA_A, ADJACENT_ADDRESS_2);

    rc = ini_shm(phy_tx_shm_key, &phy_tx_shm_oper, phy_tx_sem_key, &phy_tx_sem_id);
    if (rc < 0){
        goto free_ptr;
    }
    phy_tx_shm = phy_tx_shm_oper.shm_addr;
    phy_tx_shm->status = 0;
    memset(phy_tx_shm->data, 0, sizeof(phy_tx_shm->data));

    // Create struct for saving arguments of shared memory
    shm_oper phy_rx_shm_oper = {
        .shm_id = 0,
        .shm_addr = NULL
    };
	shm_struct *phy_rx_shm = NULL;
    key_t phy_rx_shm_key = 0 ;
    phy_rx_shm_key = (key_t)KEY_OF_SHM_N(SEA_AREA_A, LOCAL_ADDRESS);
    int phy_rx_sem_id = 0;
    key_t phy_rx_sem_key = 0 ;
    phy_rx_sem_key = (key_t)KEY_OF_SEM_N(SEA_AREA_A, LOCAL_ADDRESS);

    rc = ini_shm(phy_rx_shm_key, &phy_rx_shm_oper, phy_rx_sem_key, &phy_rx_sem_id);
    if (rc < 0){
        goto free_ptr;
    }
    phy_rx_shm = phy_rx_shm_oper.shm_addr;
    phy_rx_shm->status = 0;
    memset(phy_rx_shm->data, 0, sizeof(phy_rx_shm->data));

    // ------------------------- Initialize successfully -------------------------
    rc = ini_DOTS(net_tx_buf, net_tx_shm, net_tx_sem_id, phy_tx_buf, phy_tx_shm, phy_tx_sem_id);
    if(rc < 0){
       fprintf(stderr, "%s: Initialize DOTS protocol failed!", ERROR_TITLE);
       return -1;
    }
    get_tm_stamp(tm_stamp);
    fprintf(log_fp, "\n%s NODE[%02x] MAC_LAYER: Datalink layer all set!\n", tm_stamp, LOCAL_ADDRESS);
    fprintf(stdout, "\n%s NODE[%02x] MAC_LAYER: Datalink layer all set!\n", tm_stamp, LOCAL_ADDRESS);

    int i = 0;
    pkt_queue_node *pkt_temp = NULL;
    u_int8_t src_dst_addr_temp = 0;

	while(1)
	{
        
        for(i = 0; i < MAX_NODE_NUM; i++){
            if(node_man[i].update_stat){
                get_tm_stamp(tm_stamp);
                printf("\n%s Node [%d] has new packets!\n", tm_stamp, i);
                pkt_temp = node_man[i].front;
                while(pkt_temp != NULL){
                    if(!pkt_temp->send_stat && !pkt_temp->backoff_stat){
                        if(pkt_temp->cfrm_stat[RTS_TX_NUM] >= MAX_RETX_NUM){
                            pkt_queue_node *temp = NULL;
                            temp = pkt_temp->next;
                            get_tm_stamp(tm_stamp);
                            fprintf(log_fp, "\n%s Node [%d] has a packet meet its MAX re-transmission number!\n", tm_stamp, i);
                            fprintf(stdout, "\n%s Node [%d] has a packet meet its MAX re-transmission number!\n", tm_stamp, i);
                            del_pkt(pkt_temp);
                            node_man[i].update_stat--;
                            pkt_temp = temp;
                        }
                        else{
                            src_dst_addr_temp = (u_int8_t)( ((LOCAL_ADDRESS & 0x0f) << 4) + pkt_temp->dst_addr );
                            send_frm(src_dst_addr_temp, CFRM_RTS);
                            pkt_temp->send_stat = 1;
                            pkt_temp->cfrm_stat[RTS_TX_NUM]++;
                            node_man[i].update_stat--;
                            pkt_temp = pkt_temp->next;
                        }
                    }
                    else pkt_temp = pkt_temp->next;
                }
            }
        }

        if (phy_rx_shm->status == ADJACENT_ADDRESS_2)
        {
            drop_fram = 0;
            rc = shm_operation(phy_rx_buf, DEFAULT_PHY_BUF_SIZE, phy_rx_shm, phy_rx_sem_id, MAC_LAYER, SHM_READ);
			if (rc < 0) {
				fprintf(stderr, "MAC_LAYER ERROR: Read data from MAC layer failed!\n");
				goto free_ptr;
			}
            

            // Simulate reception process time
            printf("\nSimulate Actural RECEIVING process...\n");
            rx_sleep_stat = 1;
            add_timer((int)(CFRM_PROC_TIME), 0, 0, rx_v_sleep);
            while(rx_sleep_stat){
                
            }

            if(phy_rx_shm->status == ADJACENT_ADDRESS_2){
				get_tm_stamp(tm_stamp);
    			fprintf(log_fp, "\n%s MAC_LAYER ERROR: Reception Collsion OCCUR! Drop all frames...\n", tm_stamp);
    			fprintf(stdout, "\n%s MAC_LAYER ERROR: Reception Collsion OCCUR! Drop all frames...\n", tm_stamp);
                drop_fram = 1;
            }
            


            //sleep(3);
            if(!drop_fram){
                // Run the protocol in the mac layer
                rc = mac_DOTS(phy_rx_buf, DEFAULT_PHY_BUF_SIZE, MAC_LAYER);

                if (rc < 0) {
                    get_tm_stamp(tm_stamp);
                    fprintf(log_fp, "%s MAC_LAYER ERROR: Running protocol failed!\n", tm_stamp);
                    fprintf(stderr, "MAC_LAYER ERROR: Running protocol failed!\n");
                    goto free_ptr;
                }
            }
            if(drop_fram){
                rc = shm_operation(phy_rx_buf, DEFAULT_PHY_BUF_SIZE, phy_rx_shm, phy_rx_sem_id, MAC_LAYER, SHM_READ);
                if (rc < 0) {
                    fprintf(stderr, "MAC_LAYER ERROR: Read data from MAC layer failed!\n");
                    goto free_ptr;
                }
                memset(phy_rx_buf, 0, phy_rx_buf_size);
                get_tm_stamp(tm_stamp);
                fprintf(log_fp, "%s Drop all frames successfully!\n", tm_stamp);
                fprintf(stdout, "%s Drop all frames successfully!\n", tm_stamp);
            }
        }

		if (net_rx_shm->status == APP_LAYER)
		{
			rc = shm_operation(net_rx_buf, DEFAULT_NET_BUF_SIZE, net_rx_shm, net_rx_sem_id, MAC_LAYER, SHM_READ);
			if (rc < 0) {
				fprintf(stderr, "MAC_LAYER ERROR: Read data from MAC layer failed!\n");
				goto free_ptr;
			}

			// Robust exit
			if (memcmp(net_rx_buf, "end", 3) == 0){
                get_tm_stamp(tm_stamp);
                fprintf(log_fp, "\n%s MAC_LAYER: Now start exiting the program...\n", tm_stamp);
                fprintf(stdout, "\n%s MAC_LAYER: Now start exiting the program...\n", tm_stamp);
                memset(net_tx_buf, 0, sizeof(net_tx_buf));
                memcpy(net_tx_buf, "emac", 4);
                rc = shm_operation(net_tx_buf, net_tx_buf_size, net_tx_shm, net_tx_sem_id, MAC_LAYER, SHM_WRITE);
                if (rc < 0){
                    get_tm_stamp(tm_stamp);
                    fprintf(log_fp, "%s MAC_LAYER ERROR: Write data to MAC layer failed!\n", tm_stamp);
                    fprintf(stderr, "MAC_LAYER ERROR: Write data to MAC layer failed!\n");
                    goto free_ptr;
                }
                // Exit program
                goto free_ptr;
            }
            
            // Run the protocol in the mac layer
            rc = mac_DOTS(net_rx_buf, net_pkt_size, APP_LAYER);

			if (rc < 0) {
				get_tm_stamp(tm_stamp);
    			fprintf(log_fp, "%s MAC_LAYER ERROR: Running protocol failed!\n", tm_stamp);
				fprintf(stderr, "MAC_LAYER ERROR: Running protocol failed!\n");
				goto free_ptr;
			}

        }
	}
    
free_ptr:

	// ------------------------- Delete shared memory and semaphore with mac layer -------------------------
    // Release the link between program and shared memory
	if(shmdt((void*)phy_rx_shm) == -1)
	{
		fprintf(stderr, "%s: phy shmdt failed\n", ERROR_TITLE);
	}
    

    // Release the link between program and shared memory
	if(shmdt((void*)phy_tx_shm) == -1)
	{
		fprintf(stderr, "%s: phy shmdt failed\n", ERROR_TITLE);
	}
    
    
	// ------------------------- Unattach shared memory with netlication layer -------------------------
    // Release the link between program and shared memory
	if(shmdt((void*)net_tx_shm) == -1)
	{
		fprintf(stderr, "%s: net shmdt failed\n", ERROR_TITLE);
	}

    // Release the link between program and shared memory
	if(shmdt((void*)net_rx_shm) == -1)
	{
		fprintf(stderr, "%s: net shmdt failed\n", ERROR_TITLE);
	}
    
    // ------------------------- Delete buffer in the heap -------------------------
    // Free buffer of MAC layer and physical layer
	free(net_tx_buf);
	net_tx_buf = NULL;
	free(net_rx_buf);
	net_rx_buf = NULL;
    
	free(phy_tx_buf);
	phy_tx_buf = NULL;
	free(phy_rx_buf);
	phy_rx_buf = NULL;

    rc = Close_DOTS();


	get_tm_stamp(tm_stamp);
    fprintf(log_fp, "\n%s MAC_LAYER: Exit program successfully!\n", tm_stamp);
    fprintf(stdout, "\n%s MAC_LAYER: Exit program successfully!\n", tm_stamp);
	fprintf(log_fp, "\n----------------------- End of log file -----------------------\n\n");
	fclose(log_fp);

    return 0;
}