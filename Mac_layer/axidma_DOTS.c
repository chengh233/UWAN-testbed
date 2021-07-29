/**
 * @file axidma_DOTS.c
 * @date Thursday, Mar 04, 2021 at 02:03:16 UTC+8
 * @author Cheng Huang (cheng_huang@zju.edu.cn)
 * 
 * This file is used for implementing DOTS mac protocol
 * in the mac layer.
 * 
 * @version 1.0
 * 
 **/

#include "axidma_DOTS.h"

static u_int8_t *g_phy_tx_buf = NULL;
static shm_struct *g_phy_tx_shm = NULL;
static int g_phy_tx_sem_id = 0;

static u_int8_t *g_net_tx_buf = NULL;
static shm_struct *g_net_tx_shm = NULL;
static int g_net_tx_sem_id = 0;

static FILE *DOTS_fp = NULL;
static char *log_path = NULL;
static char tm_stamp[28];
int tx_sleep_stat = 0;

static FILE *throughput_fp = NULL;

//extern delay_map dly_map[DELAY_MAP_SIZE];
//extern node_manage node_man[MAX_NODE_NUM];

extern delay_map *dly_map;
extern node_manage *node_man;

int tx_v_sleep(u_int8_t src_dst_addr, u_int8_t cfrm_type)
{
    tx_sleep_stat = 0;
    return 0;
}


// Initialize DOTS protocol
int ini_DOTS(u_int8_t *net_tx_buf, shm_struct *net_tx_shm, int net_tx_sem_id, u_int8_t *phy_tx_buf, shm_struct *phy_tx_shm, int phy_tx_sem_id)
{
	int rc = 0;
	rc = ini_timer_manager();
	if(rc < 0){
		fprintf(stderr, "%s: Initialize TIMER MANAGER failed!\n", ERROR_TITLE);
		return -1;
	}
	rc = ini_node_manage();
	if(rc < 0){
		fprintf(stderr, "%s: Initialize NODE MANAGER failed!\n", ERROR_TITLE);
		return -1;
	}
	rc = ini_dly_map();
	if(rc < 0){
		fprintf(stderr, "%s: Initialize DELAY MAP failed!\n", ERROR_TITLE);
		return -1;
	}
	g_net_tx_buf = net_tx_buf;
	g_net_tx_shm = net_tx_shm;
	g_net_tx_sem_id = net_tx_sem_id;

	g_phy_tx_buf = phy_tx_buf;
	g_phy_tx_shm = phy_tx_shm;
	g_phy_tx_sem_id = phy_tx_sem_id;

	//sprintf(log_path, "%s%s", directory);
    DOTS_fp = fopen("axidma_DOTS.log", "w");
	throughput_fp = fopen("./Throughput.txt", "w");
	//fprintf(throughput_fp, "\n\n------------------- Log file for Throughput -------------------\n\n");
    memset(tm_stamp, 0, sizeof(tm_stamp));

	return 0;
}

/* 
 *-----------------------------------------------------------------------------
 * Function: parse packet or mac control frame function group
 * Description: This part contains the functions that parse the packet 
 * from network layer and the control frames or data packet from other mac layers.
 * Calls: none
 * Called By: none
 * Table Accessed: none
 * Table Updated: none
 * Input: buf
 * Output: none
 * Return: packet
 * Others: none
 *-----------------------------------------------------------------------------
 */

// parse function for the packet transmitted from other mac layers
void parse_cfrm(u_int8_t *buf, mac_frame *cfrm)
{
    // ------------ Get source and destination address of control frame ------------------
	// NOTE: we set local address as 0x31 (ASCII: 1) due to we enter
	// ASCII char in the application. Therefore, we need to add 0x30 to
	// src_addr and dst_addr!
	/*
    frm->src_addr = (buf[0] & 0xf0);
	frm->src_addr = (frm->src_addr >> 4) + 0x30;    // 0x30 is important, the reason is on above
    frm->dst_addr = (buf[0] & 0x0f) + 0x30;
	*/
	// Receive a control frame
	if(cfrm == NULL){
		fprintf(stderr, "%s: Create mac control frame struct failed!\n", ERROR_TITLE);
	}

    cfrm->src_dst_addr = buf[0];
    // ------------------------- Get time stamp of control frame -------------------------
    cfrm->tmv_stmp = (u_int16_t)buf[1];
	cfrm->tmv_stmp = (u_int16_t)((cfrm->tmv_stmp << 8) + (u_int16_t)buf[2]);
    // ------------------------- Get delay and type of control frame -------------------------
	cfrm->delay = buf[3];
	cfrm->type = buf[4];
}



/* 
 *-----------------------------------------------------------------------------
 * Function: map_update_part
 * Description: This part contains the function that help to update two kinds of
 * table that are used in the DOTS protocol, the delay_map table and collision_map
 * table. We transmit the first address of each table into the function and return
 * the status of running.
 * Calls: noen
 * Called By: none
 * Table Accessed: delay_map and collision_map
 * Table Updated: delay_map and collision_map
 * Input: none
 * Output: none
 * Return: running status
 * Others: none
 *-----------------------------------------------------------------------------
 */

int proc_cfrm(mac_frame *cfrm, u_int8_t *rx_buf)
{
	int rc = 0;
	int interval = 0;
	
	switch (cfrm->type)
	{
	case CFRM_RTS:
	{
		get_tm_stamp(tm_stamp);
		fprintf(DOTS_fp, "\n%s Start PROCESS RTS control frame: [%02x:%d:%02x:%02x]\n", tm_stamp, cfrm->src_dst_addr, cfrm->tmv_stmp, cfrm->delay, cfrm->type);
		fprintf(stdout, "\n%s Start PROCESS RTS control frame: [%02x:%d:%02x:%02x]\n", tm_stamp, cfrm->src_dst_addr, cfrm->tmv_stmp, cfrm->delay, cfrm->type);

		interval = cfrm->tmv_stmp + MAX_PROP_DELAY - get_cur_tmv_stmp();
		fprintf(DOTS_fp, "%s DONE! Set CTS sending timer: [%d]\n", tm_stamp, interval);
		fprintf(stdout, "%s DONE! Set CTS sending timer: [%d]\n", tm_stamp, interval);

		rc = add_timer(interval, swap_addr(cfrm->src_dst_addr), CFRM_CTS, send_frm);
		if(rc < 0){
			fprintf(stderr, "%s: Add timer for RTS sending failed!\n", ERROR_TITLE);
			return -1;
		}

		break;
	}

	case CFRM_CTS:
	{
		get_tm_stamp(tm_stamp);
		fprintf(DOTS_fp, "\n%s Start PROCESS CTS control frame: [%02x:%d:%02x:%02x]\n", tm_stamp, cfrm->src_dst_addr, cfrm->tmv_stmp, cfrm->delay, cfrm->type);
		fprintf(stdout, "\n%s Start PROCESS CTS control frame: [%02x:%d:%02x:%02x]\n", tm_stamp, cfrm->src_dst_addr, cfrm->tmv_stmp, cfrm->delay, cfrm->type);
		// Set DATA sending timer
		interval = cfrm->tmv_stmp + MAX_PROP_DELAY - get_cur_tmv_stmp();
		rc = add_timer(interval, swap_addr(cfrm->src_dst_addr), CFRM_DATA, send_frm);
		if(rc < 0){
			fprintf(stderr, "%s: Add timer for RTS sending failed!\n", ERROR_TITLE);
			return -1;
		}
		
		fprintf(DOTS_fp, "%s DONE! Set DATA sending timer: [%d]\n", tm_stamp, interval);
		fprintf(stdout, "%s DONE! Set DATA sending timer: [%d]\n", tm_stamp, interval);
		// Update control frame waiting status
		pkt_queue_node *temp = NULL;
		//temp = node_man[(int)(cfrm->src_dst_addr & 0x0f)].front;
		int node_addr = 0;
		node_addr = (int)( (u_int8_t)((cfrm->src_dst_addr & 0xf0) >> 4) ) - 1;
		temp = node_man[node_addr].front;
		while (temp != NULL)
		{
			// Check packet status, we need send status == 1, and backoff status != 1
			if(temp->send_stat && !temp->backoff_stat){
				// Check control waiting status, we need [1][0][0][0]
				if(temp->cfrm_stat[RTS_TX_NUM] && !temp->cfrm_stat[CTS_RX_STAT] && !temp->cfrm_stat[CTS_RX_CK_STAT]){
					// Set CTS reception status as 1, 
					// which means control frame matrix is changed from [1][0][0][0] to [1][1][0][0]
					temp->cfrm_stat[CTS_RX_STAT] = 1;
					
					fprintf(DOTS_fp, "%s UPDATE control frame status: {%d:%d:%d:%d}\n", tm_stamp, temp->cfrm_stat[RTS_TX_NUM], temp->cfrm_stat[CTS_RX_STAT], temp->cfrm_stat[CTS_RX_CK_STAT], temp->cfrm_stat[ACK_RX_STAT]);
					fprintf(stdout, "%s UPDATE control frame status: {%d:%d:%d:%d}\n", tm_stamp, temp->cfrm_stat[RTS_TX_NUM], temp->cfrm_stat[CTS_RX_STAT], temp->cfrm_stat[CTS_RX_CK_STAT], temp->cfrm_stat[ACK_RX_STAT]);
					return 0;
				}
				else temp = temp->next;
			}
			else temp = temp->next;
		}
		fprintf(stderr, "%s: Cannot find the packet which is waiting for CTS control frame feed back!\n", ERROR_TITLE);
		return -1;
		break;
	}

	case CFRM_DATA:
	{
		get_tm_stamp(tm_stamp);
		fprintf(DOTS_fp, "\n%s Start PROCESS DATA control frame: [%02x:%d:%02x:%02x]\n", tm_stamp, cfrm->src_dst_addr, cfrm->tmv_stmp, cfrm->delay, cfrm->type);
		fprintf(stdout, "\n%s Start PROCESS DATA control frame: [%02x:%d:%02x:%02x]\n", tm_stamp, cfrm->src_dst_addr, cfrm->tmv_stmp, cfrm->delay, cfrm->type);
		// Pass Data to network layer
		memset(g_net_tx_buf, 0, DEFAULT_NET_BUF_SIZE);
		memcpy(g_net_tx_buf, (rx_buf + DEFAULT_CFRM_SIZE), DEFAULT_DATA_PKT_SIZE);
		if(g_net_tx_shm->status){
			get_tm_stamp(tm_stamp);
			fprintf(DOTS_fp, "\n%s ERROR! DATA passed to network layer has NOT been received!\n", tm_stamp);
			fprintf(stdout, "\n%s ERROR! DATA passed to network layer has NOT been received!\n", tm_stamp);
			fprintf(DOTS_fp, "%s Now overwrite the DATA in the shared memory!\n", tm_stamp);
			fprintf(stdout, "%s Now overwrite the DATA in the shared memory!\n", tm_stamp);
		}
		rc = shm_operation(g_net_tx_buf, DEFAULT_DATA_PKT_SIZE, g_net_tx_shm, g_net_tx_sem_id, MAC_LAYER, SHM_WRITE);
		if (rc < 0){
			fprintf(stderr, "%s: Pass data to network layer failed!\n", ERROR_TITLE);
			return -1;
		}
		fprintf(DOTS_fp, "%s PASS DATA to network layer successfully! DATA: [%s]\n", tm_stamp, rx_buf + DEFAULT_CFRM_SIZE);
		fprintf(stdout, "%s PASS DATA to network layer successfully! DATA: [%s]\n", tm_stamp, rx_buf + DEFAULT_CFRM_SIZE);
		interval = cfrm->tmv_stmp + MAX_PROP_DELAY - get_cur_tmv_stmp();
		rc = add_timer(interval, swap_addr(cfrm->src_dst_addr), CFRM_ACK, send_frm);
		if(rc < 0){
			fprintf(stderr, "%s: Add timer for RTS sending failed!\n", ERROR_TITLE);
			return -1;
		}
		fprintf(DOTS_fp, "%s DONE! Set ACK sending timer: [%d]\n", tm_stamp, interval);
		fprintf(stdout, "%s DONE! Set ACK sending timer: [%d]\n", tm_stamp, interval);
		fprintf(DOTS_fp, "%s DATA receive successfully!\n", tm_stamp);
		fprintf(stdout, "%s DATA receive successfully!\n", tm_stamp);
		break;
	}

	case CFRM_ACK:
	{
		get_tm_stamp(tm_stamp);
		fprintf(DOTS_fp, "\n%s Start PROCESS ACK control frame: [%02x:%d:%02x:%02x]\n", tm_stamp, cfrm->src_dst_addr, cfrm->tmv_stmp, cfrm->delay, cfrm->type);
		fprintf(stdout, "\n%s Start PROCESS ACK control frame: [%02x:%d:%02x:%02x]\n", tm_stamp, cfrm->src_dst_addr, cfrm->tmv_stmp, cfrm->delay, cfrm->type);

		fprintf(throughput_fp, "%s Node[%02x] ACK [%02x:%d:%02x:%02x]\n", tm_stamp, LOCAL_ADDRESS, cfrm->src_dst_addr, cfrm->tmv_stmp, cfrm->delay, cfrm->type);

		// Update control frame waiting status
		pkt_queue_node *temp = NULL;
		int node_addr = 0;
		node_addr = (int)( (u_int8_t)((cfrm->src_dst_addr & 0xf0) >> 4) ) - 1;
		temp = node_man[node_addr].front;
		while (temp != NULL)
		{
			// Check packet status, we need send status == 1, and backoff status != 1
			if(temp->send_stat && !temp->backoff_stat){
				// Check control waiting status, we need {1:1:1:0}
				if(temp->cfrm_stat[RTS_TX_NUM] && temp->cfrm_stat[CTS_RX_STAT] && temp->cfrm_stat[CTS_RX_CK_STAT] && !temp->cfrm_stat[ACK_RX_STAT]){
					// Set CTS reception status as 1, 
					// which means control frame matrix is changed from {1:1:1:0} to {1:1:1:1}
					temp->cfrm_stat[ACK_RX_STAT] = 1;
					fprintf(DOTS_fp, "%s UPDATE control frame status: {%d:%d:%d:%d}\n", tm_stamp, temp->cfrm_stat[RTS_TX_NUM], temp->cfrm_stat[CTS_RX_STAT], temp->cfrm_stat[CTS_RX_CK_STAT], temp->cfrm_stat[ACK_RX_STAT]);
					fprintf(stdout, "%s UPDATE control frame status: {%d:%d:%d:%d}\n", tm_stamp, temp->cfrm_stat[RTS_TX_NUM], temp->cfrm_stat[CTS_RX_STAT], temp->cfrm_stat[CTS_RX_CK_STAT], temp->cfrm_stat[ACK_RX_STAT]);
					return 0;
				}
				else temp = temp->next;
			}
			else temp = temp->next;
		}
		fprintf(stderr, "%s: Cannot find the packet which is waiting for CTS control frame feed back!\n", ERROR_TITLE);
		return -1;
		break;
	}

	default:
	{
		fprintf(stderr, "%s: Unable to identify frame type!\n", ERROR_TITLE);
		return -1;
		break;
	}

	}
	return 0;
}

// ------------------------- Call back function -------------------------

int send_frm(u_int8_t src_dst_addr, u_int8_t cfrm_type)
{
    int rc = 0;
	int back_off_tm = 0;
	
    switch (cfrm_type)
    {
    case CFRM_RTS:
	{
		back_off_tm = ck_coll_map(src_dst_addr);
		
		if(back_off_tm == 0){
			u_int8_t mac_cfrm[5];
			memset(mac_cfrm, 0, sizeof(mac_cfrm));

			mac_cfrm[0] = src_dst_addr;
			u_int8_t tmv_stmp[2];
			get_cfrm_tmv_stmp(tmv_stmp);
			mac_cfrm[1] = tmv_stmp[0];
			mac_cfrm[2] = tmv_stmp[1];
			mac_cfrm[3] = dly_map[dly_harsh(src_dst_addr)].delay;
			mac_cfrm[4] = CFRM_RTS;
			memset(g_phy_tx_buf, 0, DEFAULT_PHY_BUF_SIZE);
			memcpy(g_phy_tx_buf, mac_cfrm, sizeof(mac_cfrm));
			// Simulate control frame process time on FPGA and transmission delay
			/*
			printf("\nSimulate Actural SENDING process...\n");
			tx_sleep_stat = 1;
			add_timer((int)(CFRM_PROC_TIME + TRAN_DELAY_TIME), 0, 0, tx_v_sleep);
			while(tx_sleep_stat){
				
			}
			*/
			get_tm_stamp(tm_stamp);
			if(g_phy_tx_shm->status){
				get_tm_stamp(tm_stamp);
				fprintf(DOTS_fp, "\n%s ERROR! DATA passed to adjacent node has NOT been received!\n", tm_stamp);
				fprintf(stdout, "\n%s ERROR! DATA passed to adjacent node has NOT been received!\n", tm_stamp);
				fprintf(DOTS_fp, "%s Now overwrite the DATA in the shared memory!\n", tm_stamp);
				fprintf(stdout, "%s Now overwrite the DATA in the shared memory!\n", tm_stamp);
			}
			rc = shm_operation(g_phy_tx_buf, DEFAULT_CFRM_SIZE, g_phy_tx_shm, g_phy_tx_sem_id, LOCAL_ADDRESS, SHM_WRITE);
			if (rc < 0){
				fprintf(stderr, "%s: Write data to physical layer failed!\n", ERROR_TITLE);
				return -1;
			}
			fprintf(DOTS_fp, "\n%s SEND RTS: [%02x:%02x:%02x:%02x:%02x]\n", tm_stamp, mac_cfrm[0], mac_cfrm[1], mac_cfrm[2], mac_cfrm[3], mac_cfrm[4]);
			fprintf(stdout, "\n%s SEND RTS: [%02x:%02x:%02x:%02x:%02x]\n", tm_stamp, mac_cfrm[0], mac_cfrm[1], mac_cfrm[2], mac_cfrm[3], mac_cfrm[4]);
			//printf("MAC_LAYER: RTS send successfully!\n");
			// Set timer for CTS feed back
			rc = add_timer(WAIT_FB_TIME, src_dst_addr, CFRM_CTS, wait_cfrm_fdbk);
			if(rc < 0){
			   fprintf(stderr, "%s: Add timer for waiting CTS feedback failed!\n", ERROR_TITLE);
			   return -1;
			}
			fprintf(DOTS_fp, "%s Start CTS waiting! [%d]\n", tm_stamp, WAIT_FB_TIME);
			fprintf(stdout, "%s Start CTS waiting! [%d]\n", tm_stamp, WAIT_FB_TIME);
			return 0;
		}
		else if(back_off_tm > 0){
			fprintf(DOTS_fp, "\n%s Collision FOUND!\n", tm_stamp);
			fprintf(stdout, "\n%s Collision FOUND!\n", tm_stamp);
			add_timer(back_off_tm, src_dst_addr, CFRM_RTS, send_frm);
			fprintf(DOTS_fp, "%s RTS Dynamic backoff: [%d]\n", tm_stamp, back_off_tm);
			fprintf(stdout, "%s RTS Dynamic backoff: [%d]\n", tm_stamp, back_off_tm);

			return 0;
		}
		else if(back_off_tm < 0){
			fprintf(stderr, "%s: Check collision map failed!\n", ERROR_TITLE);
			return -1;
		}
        break;
	}

    case CFRM_CTS:
	{
		u_int8_t mac_cfrm[5];
		memset(mac_cfrm, 0, sizeof(mac_cfrm));

		mac_cfrm[0] = src_dst_addr;

		u_int8_t tmv_stmp[2];
		get_cfrm_tmv_stmp(tmv_stmp);
		mac_cfrm[1] = tmv_stmp[0];
		mac_cfrm[2] = tmv_stmp[1];

		mac_cfrm[3] = dly_map[dly_harsh(src_dst_addr)].delay;

		mac_cfrm[4] = CFRM_CTS;
		memset(g_phy_tx_buf, 0, DEFAULT_PHY_BUF_SIZE);
		memcpy(g_phy_tx_buf, mac_cfrm, sizeof(mac_cfrm));

		// Simulate control frame process time on FPGA and transmission delay
		/*
		printf("\nSimulate Actural SENDING process...\n");
		tx_sleep_stat = 1;
		add_timer((int)(CFRM_PROC_TIME + TRAN_DELAY_TIME), 0, 0, tx_v_sleep);
		while(tx_sleep_stat){
			
		}
		*/
		if(g_phy_tx_shm->status){
			get_tm_stamp(tm_stamp);
			fprintf(DOTS_fp, "\n%s ERROR! DATA passed to adjacent node has NOT been received!\n", tm_stamp);
			fprintf(stdout, "\n%s ERROR! DATA passed to adjacent node has NOT been received!\n", tm_stamp);
			fprintf(DOTS_fp, "%s Now overwrite the DATA in the shared memory!\n", tm_stamp);
			fprintf(stdout, "%s Now overwrite the DATA in the shared memory!\n", tm_stamp);
		}
		rc = shm_operation(g_phy_tx_buf, DEFAULT_CFRM_SIZE, g_phy_tx_shm, g_phy_tx_sem_id, LOCAL_ADDRESS, SHM_WRITE);
		if (rc < 0){
			fprintf(stderr, "%s: Write data to physical layer failed!\n", ERROR_TITLE);
			return -1;
		}
		get_tm_stamp(tm_stamp);
		fprintf(DOTS_fp, "\n%s SEND CTS: [%02x:%02x:%02x:%02x:%02x]\n", tm_stamp, mac_cfrm[0], mac_cfrm[1], mac_cfrm[2], mac_cfrm[3], mac_cfrm[4]);
		fprintf(stdout, "\n%s SEND CTS: [%02x:%02x:%02x:%02x:%02x]\n", tm_stamp, mac_cfrm[0], mac_cfrm[1], mac_cfrm[2], mac_cfrm[3], mac_cfrm[4]);
		//printf("MAC_LAYER: CTS send successfully!\n");
		return 0;
        break;
	}

    case CFRM_DATA:
	{
		
		u_int8_t mac_cfrm[5];
		memset(mac_cfrm, 0, sizeof(mac_cfrm));

		mac_cfrm[0] = src_dst_addr;

		u_int8_t tmv_stmp[2];
		get_cfrm_tmv_stmp(tmv_stmp);
		mac_cfrm[1] = tmv_stmp[0];
		mac_cfrm[2] = tmv_stmp[1];

		mac_cfrm[3] = dly_map[dly_harsh(src_dst_addr)].delay;

		mac_cfrm[4] = CFRM_DATA;
		// Add DATA control frame to tx buffer
		memset(g_phy_tx_buf, 0, DEFAULT_PHY_BUF_SIZE);
		memcpy(g_phy_tx_buf, mac_cfrm, sizeof(mac_cfrm));

		pkt_queue_node *temp = NULL;
		int node_addr = 0;
		node_addr = (int)( (u_int8_t)(src_dst_addr & 0x0f) ) - 1;
		temp = node_man[node_addr].front;
		while(temp != NULL){
			if(temp->send_stat && !temp->backoff_stat){
				if(temp->cfrm_stat[RTS_TX_NUM] && temp->cfrm_stat[CTS_RX_STAT] && temp->cfrm_stat[CTS_RX_CK_STAT]){
					// Add DATA packet to tx buffer
					memcpy( (g_phy_tx_buf + DEFAULT_CFRM_SIZE), temp->DATA, sizeof(temp->DATA));

					// Simulate control frame and DATA packet process time on FPGA and transmission delay
					/*
					printf("\nSimulate Actural SENDING process...\n");
					tx_sleep_stat = 1;
					add_timer((int)(CFRM_PROC_TIME + DATA_PKT_PROC_TIME + TRAN_DELAY_TIME), 0, 0, tx_v_sleep);
					while(tx_sleep_stat){
						
					}
					*/
					if(g_phy_tx_shm->status){
						get_tm_stamp(tm_stamp);
						fprintf(DOTS_fp, "\n%s ERROR! DATA passed to adjacent node has NOT been received!\n", tm_stamp);
						fprintf(stdout, "\n%s ERROR! DATA passed to adjacent node has NOT been received!\n", tm_stamp);
						fprintf(DOTS_fp, "%s Now overwrite the DATA in the shared memory!\n", tm_stamp);
						fprintf(stdout, "%s Now overwrite the DATA in the shared memory!\n", tm_stamp);
					}
					rc = shm_operation(g_phy_tx_buf, (DEFAULT_CFRM_SIZE + DEFAULT_DATA_PKT_SIZE), g_phy_tx_shm, g_phy_tx_sem_id, LOCAL_ADDRESS, SHM_WRITE);
					if (rc < 0){
						fprintf(stderr, "%s: Write data to physical layer failed!\n", ERROR_TITLE);
						return -1;
					}
					get_tm_stamp(tm_stamp);
					fprintf(DOTS_fp, "\n%s SEND DATA: [%02x:%02x:%02x:%02x:%02x:%s]\n", tm_stamp, mac_cfrm[0], mac_cfrm[1], mac_cfrm[2], mac_cfrm[3], mac_cfrm[4], temp->DATA);
					fprintf(stdout, "\n%s SEND DATA: [%02x:%02x:%02x:%02x:%02x:%s]\n", tm_stamp, mac_cfrm[0], mac_cfrm[1], mac_cfrm[2], mac_cfrm[3], mac_cfrm[4], temp->DATA);
					// Set timer for ACK feed back
					rc = add_timer(WAIT_FB_TIME, src_dst_addr, CFRM_ACK, wait_cfrm_fdbk);
					if(rc < 0){
						fprintf(stderr, "%s: Add timer for waiting CTS feedback failed!\n", ERROR_TITLE);
						return -1;
					}
					fprintf(DOTS_fp, "%s Start ACK waiting! [%d]\n", tm_stamp, WAIT_FB_TIME);
					fprintf(stdout, "%s Start ACK waiting! [%d]\n", tm_stamp, WAIT_FB_TIME);
					//printf("MAC_LAYER: DATA send successfully!\n");
					return 0;
				}
				else temp = temp->next;//!!!!!!!!!
			}
			else temp = temp->next;
		}
		// If we did not find the pakcet that meets the condisions
		fprintf(stderr, "%s: Cannot find the packet that meets the conditions!\n", ERROR_TITLE);
		return -1;
        break;
	}

    case CFRM_ACK:
	{
		u_int8_t mac_cfrm[5];
		memset(mac_cfrm, 0, sizeof(mac_cfrm));

		mac_cfrm[0] = src_dst_addr;

		u_int8_t tmv_stmp[2];
		get_cfrm_tmv_stmp(tmv_stmp);
		mac_cfrm[1] = tmv_stmp[0];
		mac_cfrm[2] = tmv_stmp[1];

		mac_cfrm[3] = dly_map[dly_harsh(src_dst_addr)].delay;

		mac_cfrm[4] = CFRM_ACK;
		memset(g_phy_tx_buf, 0, DEFAULT_PHY_BUF_SIZE);
		memcpy(g_phy_tx_buf, mac_cfrm, sizeof(mac_cfrm));

		// Simulate control frame process time on FPGA and transmission delay
		/*
		printf("\nSimulate Actural SENDING process...\n");
		tx_sleep_stat = 1;
		add_timer((int)(CFRM_PROC_TIME + TRAN_DELAY_TIME), 0, 0, tx_v_sleep);
		while(tx_sleep_stat){
			
		}
		*/
		if(g_phy_tx_shm->status){
			get_tm_stamp(tm_stamp);
			fprintf(DOTS_fp, "\n%s ERROR! DATA passed to adjacent node has NOT been received!\n", tm_stamp);
			fprintf(stdout, "\n%s ERROR! DATA passed to adjacent node has NOT been received!\n", tm_stamp);
			fprintf(DOTS_fp, "%s Now overwrite the DATA in the shared memory!\n", tm_stamp);
			fprintf(stdout, "%s Now overwrite the DATA in the shared memory!\n", tm_stamp);
		}
		rc = shm_operation(g_phy_tx_buf, DEFAULT_CFRM_SIZE, g_phy_tx_shm, g_phy_tx_sem_id, LOCAL_ADDRESS, SHM_WRITE);
		if (rc < 0){
			fprintf(stderr, "%s: Write data to physical layer failed!\n", ERROR_TITLE);
			return -1;
		}
		get_tm_stamp(tm_stamp);
		fprintf(DOTS_fp, "\n%s SEND ACK: [%02x:%02x:%02x:%02x:%02x]\n", tm_stamp, mac_cfrm[0], mac_cfrm[1], mac_cfrm[2], mac_cfrm[3], mac_cfrm[4]);
		fprintf(stdout, "\n%s SEND ACK: [%02x:%02x:%02x:%02x:%02x]\n", tm_stamp, mac_cfrm[0], mac_cfrm[1], mac_cfrm[2], mac_cfrm[3], mac_cfrm[4]);
		//printf("MAC_LAYER: ACK send successfully!\n");
		return 0;
        break;
	}

    default:
	{
		fprintf(stderr, "%s: The argument 'Control Frame' in the send frame function is wrong!\n", ERROR_TITLE);
        return -1;
		break;
	}

    }
    return -1;
}

int wait_cfrm_fdbk(u_int8_t src_dst_addr, u_int8_t cfrm_type)
{
    int rc = 0;
    switch (cfrm_type)
    {
	case CFRM_CTS:
	{
		get_tm_stamp(tm_stamp);
		fprintf(DOTS_fp, "\n%s Wait CTS TIME OUT!\n", tm_stamp);
		fprintf(stdout, "\n%s Wait CTS TIME OUT!\n", tm_stamp);
		pkt_queue_node *temp = NULL;
		int node_addr = 0;
		node_addr = (int)( (u_int8_t)(src_dst_addr & 0x0f) ) - 1;
		//printf("\nWait CTS: node address = %d\n", node_addr);
		temp = node_man[node_addr].front;
		while(temp != NULL){
			if(temp->send_stat && !temp->backoff_stat){
				if(temp->cfrm_stat[RTS_TX_NUM] && temp->cfrm_stat[CTS_RX_STAT] && !temp->cfrm_stat[CTS_RX_CK_STAT]){
					fprintf(DOTS_fp, "%s CTS RECEIVED!\n", tm_stamp);
					fprintf(stdout, "%s CTS RECEIVED!\n", tm_stamp);
					temp->cfrm_stat[CTS_RX_CK_STAT] = 1;
					fprintf(DOTS_fp, "%s UPDATE control frame status: {%d:%d:%d:%d}\n", tm_stamp, temp->cfrm_stat[RTS_TX_NUM], temp->cfrm_stat[CTS_RX_STAT], temp->cfrm_stat[CTS_RX_CK_STAT], temp->cfrm_stat[ACK_RX_STAT]);
					fprintf(stdout, "%s UPDATE control frame status: {%d:%d:%d:%d}\n", tm_stamp, temp->cfrm_stat[RTS_TX_NUM], temp->cfrm_stat[CTS_RX_STAT], temp->cfrm_stat[CTS_RX_CK_STAT], temp->cfrm_stat[ACK_RX_STAT]);
					return 0;
				}

				else temp = temp->next;
			}
			else temp = temp->next;
		}
		// If cannot find correspoind pkt, we backoff the latest pkt
		temp = node_man[node_addr].front;
		while(temp != NULL){
			if(temp->send_stat && !temp->backoff_stat){
				if(temp->cfrm_stat[RTS_TX_NUM] && !temp->cfrm_stat[CTS_RX_STAT] && !temp->cfrm_stat[CTS_RX_CK_STAT]){
					fprintf(DOTS_fp, "%s CTS NOT receive!\n", tm_stamp);
					fprintf(stdout, "%s CTS NOT receive!\n", tm_stamp);
					temp->backoff_stat = 1;
					rc = add_timer(BACK_OFF_TIME(temp->cfrm_stat[RTS_TX_NUM]), src_dst_addr, cfrm_type, back_off);
					if(rc < 0){
						fprintf(stderr, "%s: Add back off timer failed!\n", ERROR_TITLE);
						return -1;
					}
					fprintf(DOTS_fp, "%s Start back-off! [%d]\n", tm_stamp, BACK_OFF_TIME(temp->cfrm_stat[RTS_TX_NUM]));
					fprintf(stdout, "%s Start back-off! [%d]\n", tm_stamp, BACK_OFF_TIME(temp->cfrm_stat[RTS_TX_NUM]));
					return 0;
				}

				else temp = temp->next;
			}
			else temp = temp->next;
		}

		fprintf(stderr, "Cannot find the packet that meets the conditions!\n");
		fprintf(stderr, "Callback action in ACK feedback waiting funciton failed!\n");
		return -1;

		break;
	}

    case CFRM_ACK:
	{
		get_tm_stamp(tm_stamp);
		fprintf(DOTS_fp, "\n%s Wait ACK TIME OUT!\n", tm_stamp);
		fprintf(stdout, "\n%s Wait ACK TIME OUT!\n", tm_stamp);
		pkt_queue_node *temp = NULL;
		int node_addr = 0;
		node_addr = (int)( (u_int8_t)(src_dst_addr & 0x0f) ) - 1;
		temp = node_man[node_addr].front;
		while(temp != NULL){
			if(temp->send_stat && !temp->backoff_stat){
				if(temp->cfrm_stat[RTS_TX_NUM] && temp->cfrm_stat[CTS_RX_STAT] && temp->cfrm_stat[CTS_RX_CK_STAT] && temp->cfrm_stat[ACK_RX_STAT]){
					fprintf(DOTS_fp, "%s ACK RECEIVED!\n", tm_stamp);
					fprintf(stdout, "%s ACK RECEIVED!\n", tm_stamp);
					rc = del_pkt(temp);
					if(rc < 0){
					   fprintf(stderr, "%s: Delete packet in the packet queue failed!\n", ERROR_TITLE);
					   return -1;
					}
					fprintf(DOTS_fp, "%s DATA packet DELETE successfully!\n", tm_stamp);
					fprintf(stdout, "%s DATA packet DELETE successfully!\n", tm_stamp);
					return 0;
				}
				else temp = temp->next;
			}
			else temp = temp->next;
		}

		temp = node_man[node_addr].front;
		while(temp != NULL){
			if(temp->send_stat && !temp->backoff_stat){
				if(temp->cfrm_stat[RTS_TX_NUM] && temp->cfrm_stat[CTS_RX_STAT] && temp->cfrm_stat[CTS_RX_CK_STAT] && !temp->cfrm_stat[ACK_RX_STAT]){
					fprintf(DOTS_fp, "%s ACK NOT receive!\n", tm_stamp);
					fprintf(stdout, "%s ACK NOT receive!\n", tm_stamp);
					temp->backoff_stat = 1;
					rc = add_timer(BACK_OFF_TIME(temp->cfrm_stat[RTS_TX_NUM]), src_dst_addr, cfrm_type, back_off);
					if(rc < 0){
						fprintf(stderr, "%s: Add back off timer failed!\n", ERROR_TITLE);
						return -1;
					}
					
					fprintf(DOTS_fp, "%s Start back-off! [%d]\n", tm_stamp, BACK_OFF_TIME(temp->cfrm_stat[RTS_TX_NUM]));
					fprintf(stdout, "%s Start back-off! [%d]\n", tm_stamp, BACK_OFF_TIME(temp->cfrm_stat[RTS_TX_NUM]));
					return 0;
				}
				else temp = temp->next;
			}
			else temp = temp->next;
		}



		fprintf(stderr, "Cannot find the packet that meets the conditions!\n");
		fprintf(stderr, "Callback action in ACK feedback waiting funciton failed!\n");
		return -1;
		break;
	}

    default:
	{
        fprintf(stderr, "The arguments in control frame feedback waiting funciton is wrong!\n");
        return -1;
        break;
	}

    }
    return -1;
}

int back_off(u_int8_t src_dst_addr, u_int8_t cfrm_type)
{
    int rc = 0;
	get_tm_stamp(tm_stamp);
	fprintf(DOTS_fp, "\n%s Back-off TIME OUT!\n", tm_stamp);
	fprintf(stdout, "\n%s Back-off TIME OUT!\n", tm_stamp);
    // release backoff lock
	pkt_queue_node *temp = NULL;
	int node_addr = 0;
	node_addr = (int)( (u_int8_t)(src_dst_addr & 0x0f) ) - 1;
	temp = node_man[node_addr].front;
	while(temp != NULL){
		if(temp->backoff_stat){
			temp->cfrm_stat[CTS_RX_STAT] = 0;
			temp->cfrm_stat[CTS_RX_CK_STAT] = 0;
			temp->cfrm_stat[ACK_RX_STAT] = 0;
			temp->send_stat = 0;
			temp->backoff_stat = 0;
			node_man[node_addr].update_stat++;
			fprintf(DOTS_fp, "%s UPDATE control frame status: {%d:%d:%d:%d}\n", tm_stamp, temp->cfrm_stat[RTS_TX_NUM], temp->cfrm_stat[CTS_RX_STAT], temp->cfrm_stat[CTS_RX_CK_STAT], temp->cfrm_stat[ACK_RX_STAT]);
			fprintf(stdout, "%s UPDATE control frame status: {%d:%d:%d:%d}\n", tm_stamp, temp->cfrm_stat[RTS_TX_NUM], temp->cfrm_stat[CTS_RX_STAT], temp->cfrm_stat[CTS_RX_CK_STAT], temp->cfrm_stat[ACK_RX_STAT]);
			return 0;
		}
		else temp = temp->next;
	}
	fprintf(stderr, "Cannot find the packet that meets the conditions!\n");
	fprintf(stderr, "Callback action in BACK OFF funciton failed!\n");
    return -1;
}



// ------------------------- MAIN function -------------------------

int mac_DOTS(u_int8_t *rx_buf, size_t rx_data_size, int rx_layer_id)
{
	int rc = 0;
	
	// ------------------------- Data from network layer -------------------------
	if (rx_layer_id == APP_LAYER)
	{
		if( (int)rx_buf[rx_data_size - 1] <= 0 || (int)rx_buf[rx_data_size - 1] > MAX_NODE_NUM){
			fprintf(stderr, "\n%s: DATA packet format ERROR!\n", ERROR_TITLE);
			return -1;
		}
		get_tm_stamp(tm_stamp);
		fprintf(DOTS_fp, "\n%s RECEIVE DATA Packet: [%02x:%s]\n", tm_stamp, rx_buf[rx_data_size - 1], rx_buf);
		fprintf(stdout, "\n%s RECEIVE DATA Packet: [%02x:%s]\n", tm_stamp, rx_buf[rx_data_size - 1], rx_buf);
		// Add new packets to the packet queue
		rc = add_pkt(rx_buf, rx_data_size);
		if(rc < 0){
			fprintf(stderr, "%s: Add new packets to the packet queue failed!\n", ERROR_TITLE);
			return -1;
		}
		return 0;
	}

	// ------------------------- Data from mac layer -------------------------
	if (rx_layer_id == MAC_LAYER)
	{
		get_tm_stamp(tm_stamp);
		fprintf(DOTS_fp, "\n%s RECEIVE Control frame: [%02x:%02x:%02x:%02x:%02x]\n", tm_stamp, rx_buf[0], rx_buf[1], rx_buf[2], rx_buf[3], rx_buf[4]);
		fprintf(stdout, "\n%s RECEIVE Control frame: [%02x:%02x:%02x:%02x:%02x]\n", tm_stamp, rx_buf[0], rx_buf[1], rx_buf[2], rx_buf[3], rx_buf[4]);


		// Receive a control frame
		mac_frame rx_cfrm;
		// parse mac control frame
		parse_cfrm(rx_buf, &rx_cfrm);
		// update delay map
		int adj_node_index = 0;
		adj_node_index = dly_harsh( (u_int8_t)((u_int8_t)((LOCAL_ADDRESS & 0x0f) << 4) + (u_int8_t)((rx_cfrm.src_dst_addr & 0xf0) >> 4)) );
		if(adj_node_index < 0 || adj_node_index >= DELAY_MAP_SIZE){
			fprintf(stderr, "%s: Address of received control frame ERROR!\n", ERROR_TITLE);
			return -1;
		}

		dly_map[adj_node_index].delay = (u_int8_t)(get_cur_tmv_stmp() - rx_cfrm.tmv_stmp);

		// If we receive DATA control frame, we will simulate actual DATA packet reception followed by the 
		// DATA control frame
		if(rx_cfrm.type == CFRM_DATA){
			printf("\nSimulate Actural DATA packet process...\n");
			tx_sleep_stat = 1;
			add_timer((int)(DATA_PKT_PROC_TIME), 0, 0, tx_v_sleep);
			while(tx_sleep_stat){
					
			}
		}
		// Check if destination is local node
		if( (rx_cfrm.src_dst_addr & 0x0f) == (LOCAL_ADDRESS & 0x0f)){
			// update collision map
			rc = add_coll_node(rx_cfrm.src_dst_addr, rx_cfrm.type, rx_cfrm.tmv_stmp);
			if(rc < 0){
				fprintf(stderr, "%s: Add new collision nodes to delay map failed!\n", ERROR_TITLE);
				return -1;
			}
			// respond mac control frame
			//
			// receive what kind of control frame?
			// RTS: send back CTS when there is no collision or do nothing
			// CTS: send back DATA first and then send DATA pkt
			// DATA: change into receive DATA mode (cannot be interrupted by the network layer or sending interupt)
			// Wait until receive DATA pkt successfully and pass it to the network layer.
			// Release DATA mode, send back ACK control frame.
			// ACK: successful transmitted packets number ++
			// 
			// process the pkt from network layer(check collsion, clear then send RTS, collision found then backoff)
			// wait for new pkt or control frame...
			rc = proc_cfrm(&rx_cfrm, rx_buf);
			if(rc < 0){
				fprintf(stderr, "%s: Process control frame failed!\n", ERROR_TITLE);
				return -1;
			}
			return 0;

		}

		// Enter overhearing mode
		else{
			// update delay map
			dly_map[dly_harsh(rx_cfrm.src_dst_addr)].delay = rx_cfrm.delay;
			// update collision map
			rc = add_coll_node(rx_cfrm.src_dst_addr, rx_cfrm.type, rx_cfrm.tmv_stmp);
			if(rc < 0){
				fprintf(stderr, "%s: Add new collision nodes to delay map failed!\n", ERROR_TITLE);
				return -1;
			}
			return 0;
		}

	}
}

int Close_DOTS()
{
	int rc = 0;
	fclose(throughput_fp);
	fclose(DOTS_fp);
    // ------------------------- Delete delay map and all collision nodes -------------------------
    rc = del_dly_map();
	if(rc < 0){
	   fprintf(stderr, "%s: Delte delay map failed!\n", ERROR_TITLE);
	   return -1;
	}

    rc = del_timer_manager();
	if(rc < 0){
	   fprintf(stderr, "%s: Delte timing wheel failed!\n", ERROR_TITLE);
	   return -1;
	}


    // ------------------------- Delete node manager and all packets -------------------------
    rc = del_node_manager();
	if(rc < 0){
	   fprintf(stderr, "%s: Delte node manager failed!\n", ERROR_TITLE);
	   return -1;
	}
	return 0;
}