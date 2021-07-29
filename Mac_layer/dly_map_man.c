/**
 * @file dly_map_man.c
 * @date Saturday, Mar 20, 2021 at 21:03:42 UTC+8
 * @author Cheng Huang (cheng_huang@zju.edu.cn)
 * 
 * This file contains functions for creating, processing, deleting
 * the delay map used in the DOTS protocol
 * 
 * @version 1.0
 * 
 **/

#include "dly_map_man.h"

static FILE *dly_map_fp;
static char tm_stamp[28];

//delay_map dly_map[DELAY_MAP_SIZE];
delay_map *dly_map = NULL;

int ini_dly_map()
{
    dly_map_fp = fopen("dly_map_man.log", "w");
    memset(tm_stamp, 0, sizeof(tm_stamp));
    dly_map = (delay_map *)calloc(DELAY_MAP_SIZE, sizeof(delay_map));
    
    //memset(dly_map, 0, sizeof(dly_map));
    return 0;
}
// ------------------------- Delay map manager -------------------------
int dly_harsh(u_int8_t src_dst_addr)
{
	int harsh_key = 0;
	int src_addr = 0;
	int dst_addr = 0;
	src_addr = (int)((src_dst_addr & 0xf0) >> 4);
	dst_addr = (int)(src_dst_addr & 0x0f);
	// harsh function H(key) = (i - 1) * N + j - 2, N is maximum number of nodes
	// in the network, i is source address and j is destination address
	harsh_key = ( (src_addr - 1) * MAX_NODE_NUM) + dst_addr - 2;
	return harsh_key;
}

int add_coll_node(u_int8_t src_dst_addr, u_int8_t cfrm_type, u_int16_t tmv_stmp)
{
    // Allocate space for new collision node on the heap
	collision_node *new_coll_node = NULL;
	new_coll_node = (collision_node *)calloc(1, sizeof(collision_node));
	if(new_coll_node == NULL){
		fprintf(stderr, "%s: Allocate space for new collision node failed!\n", ERROR_TITLE);
		return -1;
	}

    // Initialize collision node
    new_coll_node->src_dst_addr = src_dst_addr;
    new_coll_node->cfrm_type = cfrm_type;
    new_coll_node->tmv_stmp = tmv_stmp;
    new_coll_node->prior = NULL;
    new_coll_node->next = NULL;

    // Compute harsh address for this collision node
    int node_addr = 0;
    node_addr = dly_harsh(src_dst_addr);

    get_tm_stamp(tm_stamp);
    fprintf(dly_map_fp, "\n%s ADD Collision node: [%d]->{%02x:%02x:%d} \n", tm_stamp, node_addr, new_coll_node->src_dst_addr, new_coll_node->cfrm_type, new_coll_node->tmv_stmp);
    fprintf(stdout, "\n%s ADD Collision node: [%d]->{%02x:%02x:%d} \n", tm_stamp, node_addr, new_coll_node->src_dst_addr, new_coll_node->cfrm_type, new_coll_node->tmv_stmp);

    // if queue is empty, we let front ptr point to the new packet node
    if(dly_map[node_addr].front == NULL){
        dly_map[node_addr].front = new_coll_node;
        dly_map[node_addr].rear = dly_map[node_addr].front;
        //printf("MAC_LAYER: Add a new collision node to the empty delay map node successfully!\n");
        return 0;
    }
    // if queue is not empty, we add new packet node to the end of the queue
    else{
        new_coll_node->prior = dly_map[node_addr].rear;
        dly_map[node_addr].rear->next = new_coll_node;
        dly_map[node_addr].rear = new_coll_node;
        //printf("MAC_LAYER: Add a new collision node to the delay map node successfully!\n");
        return 0;
    }
    // We should never arrive here
	return -1;
}

// lcl == local node, adj == adjacent node
int ck_collision(u_int8_t src_dst_addr, collision_node *coll_node)
{
    int src_dst_index = 0;
    src_dst_index = dly_harsh(src_dst_addr);

    int oth_det_index = 0;
    oth_det_index = dly_harsh(coll_node->src_dst_addr);

    u_int8_t src_det_addr;
    src_det_addr = (u_int8_t)( (LOCAL_ADDRESS & 0x0f) << 4) + (u_int8_t)((coll_node->src_dst_addr & 0xf0) >> 4);
    int src_det_index = 0;
    src_det_index = dly_harsh(src_det_addr);


    // local node to detected node, not destination node
	switch (coll_node->cfrm_type)
	{
	case CFRM_RTS:
    {
        // Neighboring non-interference (no collision in the adjacent node)
        u_int16_t la_RTS_rx_tmv = 0;        // local node to adjacent node
        u_int16_t la_DATA_rx_tmv = 0;
        la_RTS_rx_tmv =  get_cur_tmv_stmp() + (u_int16_t)(dly_map[src_det_index].delay);
        la_DATA_rx_tmv = get_cur_tmv_stmp() + (2 * MAX_PROP_DELAY) + (u_int16_t)(dly_map[src_det_index].delay);
        u_int16_t adj_CTS_rx_tmv = 0;       // other nodes to adjacent node
        u_int16_t adj_ACK_rx_tmv = 0;
        adj_CTS_rx_tmv = coll_node->tmv_stmp + MAX_PROP_DELAY + (u_int16_t)dly_map[oth_det_index].delay;
        adj_ACK_rx_tmv = coll_node->tmv_stmp + (3 * MAX_PROP_DELAY) + (u_int16_t)dly_map[oth_det_index].delay;

        if(la_RTS_rx_tmv > adj_ACK_rx_tmv + CFRM_GUARD_TIME){
            if( del_coll_node(coll_node) < 0){
                fprintf(stderr, "\n\n%s: UPDATE collision node failed!\n\n", ERROR_TITLE);
            }
            get_tm_stamp(tm_stamp);
            fprintf(dly_map_fp, "\n%s UPDATE Collision list: [%d] successfully! (Remove RTS)\n", tm_stamp, oth_det_index);
            fprintf(stdout, "\n%s UPDATE Collision list: [%d]successfully! (Remove RTS)\n", tm_stamp, oth_det_index);
            return 0;
        }

        if((la_RTS_rx_tmv >= adj_CTS_rx_tmv - CFRM_GUARD_TIME) && (la_RTS_rx_tmv <= adj_CTS_rx_tmv + CFRM_GUARD_TIME)){
            return (int)(adj_CTS_rx_tmv + CFRM_GUARD_TIME - la_RTS_rx_tmv + 5);
        }
        if((la_RTS_rx_tmv >= adj_ACK_rx_tmv - CFRM_GUARD_TIME) && (la_RTS_rx_tmv <= adj_ACK_rx_tmv + CFRM_GUARD_TIME)){
            return (int)(adj_ACK_rx_tmv + CFRM_GUARD_TIME - la_RTS_rx_tmv + 5);
        }
        if((la_DATA_rx_tmv >= adj_CTS_rx_tmv - CFRM_GUARD_TIME) && (la_DATA_rx_tmv <= adj_CTS_rx_tmv + CFRM_GUARD_TIME)){
            return (int)(adj_CTS_rx_tmv + CFRM_GUARD_TIME - la_DATA_rx_tmv + 5);
        }
        if((la_DATA_rx_tmv >= adj_ACK_rx_tmv - CFRM_GUARD_TIME) && (la_DATA_rx_tmv <= adj_ACK_rx_tmv + CFRM_GUARD_TIME)){
            return (int)(adj_ACK_rx_tmv + CFRM_GUARD_TIME - la_DATA_rx_tmv + 5);
        }

        // Prospective non-interference (no collision in the local node)
        u_int16_t al_CTS_rx_tmv = 0;        // adjacent node to local node
        u_int16_t al_ACK_rx_tmv = 0;
        al_CTS_rx_tmv = get_cur_tmv_stmp() + MAX_PROP_DELAY + (u_int16_t)(dly_map[src_dst_index].delay);
        al_ACK_rx_tmv = get_cur_tmv_stmp() + (3 * MAX_PROP_DELAY) + (u_int16_t)(dly_map[src_dst_index].delay);
        u_int16_t lcl_DATA_rx_tmv = 0;      // adjacent node to local node
        lcl_DATA_rx_tmv = get_cur_tmv_stmp() + (2 * MAX_PROP_DELAY) + (u_int16_t)dly_map[src_det_index].delay;
        
        if((lcl_DATA_rx_tmv >= al_CTS_rx_tmv - CFRM_GUARD_TIME) && (lcl_DATA_rx_tmv <= al_CTS_rx_tmv + CFRM_GUARD_TIME)){
            return (int)(al_CTS_rx_tmv + CFRM_GUARD_TIME - lcl_DATA_rx_tmv + 5);
        }
        if((lcl_DATA_rx_tmv >= al_ACK_rx_tmv - CFRM_GUARD_TIME) && (lcl_DATA_rx_tmv <= al_ACK_rx_tmv + CFRM_GUARD_TIME)){
            return (int)(al_ACK_rx_tmv + CFRM_GUARD_TIME - lcl_DATA_rx_tmv + 5);
        }

        return 0;

		break;
    }

	case CFRM_CTS:
    {
        // Neighboring non-interference (no collision in the adjacent node)
        u_int16_t la_RTS_rx_tmv = 0;        // local node to adjacent node
        u_int16_t la_DATA_rx_tmv = 0;
        la_RTS_rx_tmv =  get_cur_tmv_stmp() + (u_int16_t)(dly_map[src_det_index].delay);
        la_DATA_rx_tmv = get_cur_tmv_stmp() + (2 * MAX_PROP_DELAY) + (u_int16_t)(dly_map[src_det_index].delay);
        u_int16_t adj_DATA_rx_tmv = 0;      // other nodes to adjacent node
        adj_DATA_rx_tmv = coll_node->tmv_stmp + MAX_PROP_DELAY + (u_int16_t)dly_map[oth_det_index].delay;
        
        if((la_RTS_rx_tmv >= adj_DATA_rx_tmv - (u_int16_t)(DATA_GUARD_TIME / 2) ) && (la_RTS_rx_tmv <= adj_DATA_rx_tmv + DATA_GUARD_TIME)){
            return (int)(adj_DATA_rx_tmv + DATA_GUARD_TIME - la_RTS_rx_tmv + 5);
        }
        if((la_DATA_rx_tmv >= adj_DATA_rx_tmv - (u_int16_t)(DATA_GUARD_TIME / 2)) && (la_DATA_rx_tmv <= adj_DATA_rx_tmv + DATA_GUARD_TIME)){
            return (int)(adj_DATA_rx_tmv + DATA_GUARD_TIME - la_DATA_rx_tmv + 5);
        }

        // Prospective non-interference (no collision in the local node)
        u_int16_t al_CTS_rx_tmv = 0;        // adjacent node to local node
        u_int16_t al_ACK_rx_tmv = 0;
        al_CTS_rx_tmv = get_cur_tmv_stmp() + MAX_PROP_DELAY + (u_int16_t)(dly_map[src_dst_index].delay);
        al_ACK_rx_tmv = get_cur_tmv_stmp() + (3 * MAX_PROP_DELAY) + (u_int16_t)(dly_map[src_dst_index].delay);
        u_int16_t lcl_ACK_rx_tmv = 0;      // adjacent node to other node
        lcl_ACK_rx_tmv = coll_node->tmv_stmp + (2 * MAX_PROP_DELAY) + (u_int16_t)dly_map[src_det_index].delay;
        
        if((lcl_ACK_rx_tmv >= al_CTS_rx_tmv - CFRM_GUARD_TIME) && (lcl_ACK_rx_tmv <= al_CTS_rx_tmv + CFRM_GUARD_TIME)){
            return (int)(al_CTS_rx_tmv + CFRM_GUARD_TIME - lcl_ACK_rx_tmv + 5);
        }
        if((lcl_ACK_rx_tmv >= al_ACK_rx_tmv - CFRM_GUARD_TIME) && (lcl_ACK_rx_tmv <= al_ACK_rx_tmv + CFRM_GUARD_TIME)){
            return (int)(al_ACK_rx_tmv + CFRM_GUARD_TIME - lcl_ACK_rx_tmv + 5);
        }

        if(la_RTS_rx_tmv > lcl_ACK_rx_tmv + CFRM_GUARD_TIME){
            if( del_coll_node(coll_node) < 0){
                fprintf(stderr, "\n\n%s: UPDATE collision node failed!\n\n", ERROR_TITLE);
            }
            get_tm_stamp(tm_stamp);
            fprintf(dly_map_fp, "\n%s UPDATE Collision list: [%d] successfully! (Remove CTS)\n", tm_stamp, oth_det_index);
            fprintf(stdout, "\n%s UPDATE Collision list: [%d]successfully! (Remove CTS)\n", tm_stamp, oth_det_index);
            return 0;
        }

        return 0;
		break;
    }

	case CFRM_DATA:
    {
        // Neighboring non-interference (no collision in the adjacent node)
        u_int16_t la_RTS_rx_tmv = 0;        // local node to adjacent node
        u_int16_t la_DATA_rx_tmv = 0;
        la_RTS_rx_tmv =  get_cur_tmv_stmp() + (u_int16_t)(dly_map[src_det_index].delay);
        la_DATA_rx_tmv = get_cur_tmv_stmp() + (2 * MAX_PROP_DELAY) + (u_int16_t)(dly_map[src_det_index].delay);
        u_int16_t adj_ACK_rx_tmv = 0;       // other nodes to adjacent node
        adj_ACK_rx_tmv = coll_node->tmv_stmp + MAX_PROP_DELAY + (u_int16_t)dly_map[oth_det_index].delay;

        if((la_RTS_rx_tmv >= adj_ACK_rx_tmv - CFRM_GUARD_TIME) && (la_RTS_rx_tmv <= adj_ACK_rx_tmv + CFRM_GUARD_TIME)){
            return (int)(adj_ACK_rx_tmv + CFRM_GUARD_TIME - la_RTS_rx_tmv + 5);
        }
        if((la_DATA_rx_tmv >= adj_ACK_rx_tmv - CFRM_GUARD_TIME) && (la_DATA_rx_tmv <= adj_ACK_rx_tmv + CFRM_GUARD_TIME)){
            return (int)(adj_ACK_rx_tmv + CFRM_GUARD_TIME - la_DATA_rx_tmv + 5);
        }

        if(la_RTS_rx_tmv > adj_ACK_rx_tmv + CFRM_GUARD_TIME){
            if( del_coll_node(coll_node) < 0){
                fprintf(stderr, "\n\n%s: UPDATE collision node failed!\n\n", ERROR_TITLE);
            }
            get_tm_stamp(tm_stamp);
            fprintf(dly_map_fp, "\n%s UPDATE Collision list: [%d] successfully! (Remove DATA)\n", tm_stamp, oth_det_index);
            fprintf(stdout, "\n%s UPDATE Collision list: [%d]successfully! (Remove DATA)\n", tm_stamp, oth_det_index);
            return 0;
        }

        return 0;
		break;	
    }
    case CFRM_ACK:
    {
        if( del_coll_node(coll_node) < 0){
            fprintf(stderr, "\n\n%s: UPDATE collision node failed!\n\n", ERROR_TITLE);
        }
        get_tm_stamp(tm_stamp);
        fprintf(dly_map_fp, "\n%s UPDATE Collision list: [%d] successfully! (Remove ACK)\n", tm_stamp, oth_det_index);
        fprintf(stdout, "\n%s UPDATE Collision list: [%d]successfully! (Remove ACK)\n", tm_stamp, oth_det_index);
        return 0;
    }
	default:
    {
        fprintf(stderr, "%s: Unknown control frame type in the collision node!\n", ERROR_TITLE);
        return -1;
		break;
    }

	}

    // We should never arrive here
    return -1;
}

int ck_coll_map(u_int8_t src_dst_addr)
{
    int back_off_tm = 0;
    int i = 0;
    collision_node *temp = NULL;
    for(i = 0; i < DELAY_MAP_SIZE; i++){
        temp = dly_map[i].front;
        while(temp != NULL){
            back_off_tm = ck_collision(src_dst_addr, temp);
            if(back_off_tm < 0){
                fprintf(stderr, "%s: Check collision node failed!\n", ERROR_TITLE);
                return -1;
            }
            if(back_off_tm > 0) return back_off_tm;
            temp = temp->next;
        }
    }
    return 0;
}

int del_coll_node(collision_node *coll_node)
{
    if(coll_node == NULL){
        fprintf(stderr, "%s: Delete collision node failed! Collision node is empty!\n", ERROR_TITLE);
        return -1;
    }
    int node_addr = 0;
    node_addr = dly_harsh(coll_node->src_dst_addr);
    // Delete only node in the list
    if(coll_node->prior == NULL && coll_node->next == NULL){
        dly_map[node_addr].front = NULL;
        dly_map[node_addr].rear = NULL;
        get_tm_stamp(tm_stamp);
        fprintf(dly_map_fp, "\n%s REMOVE Collision node:[%d]->{%02x:%02x:%d} \n", tm_stamp, node_addr, coll_node->src_dst_addr, coll_node->cfrm_type, coll_node->tmv_stmp);
        fprintf(stdout, "\n%s REMOVE Collision node: [%d]->{%02x:%02x:%d} \n", tm_stamp, node_addr, coll_node->src_dst_addr, coll_node->cfrm_type, coll_node->tmv_stmp);
        free(coll_node);

        return 0;
    }
    // Delete node in the end of the list
    else if(coll_node->prior != NULL && coll_node->next == NULL){
        dly_map[node_addr].rear = coll_node->prior;
        dly_map[node_addr].rear->next = NULL;
        get_tm_stamp(tm_stamp);
        fprintf(dly_map_fp, "\n%s REMOVE Collision node:[%d]->{%02x:%02x:%d} \n", tm_stamp, node_addr, coll_node->src_dst_addr, coll_node->cfrm_type, coll_node->tmv_stmp);
        fprintf(stdout, "\n%s REMOVE Collision node: [%d]->{%02x:%02x:%d} \n", tm_stamp, node_addr, coll_node->src_dst_addr, coll_node->cfrm_type, coll_node->tmv_stmp);
        free(coll_node);
        //printf("MAC_LAYER: Delete a collision node successfully!\n");
        return 0;
    }
    // Delete node in the head of the list
    else if (coll_node->prior == NULL && coll_node->next != NULL){
        dly_map[node_addr].front = coll_node->next;
        dly_map[node_addr].front->prior = NULL;
        get_tm_stamp(tm_stamp);
        fprintf(dly_map_fp, "\n%s REMOVE Collision node: [%d]->{%02x:%02x:%d} \n", tm_stamp, node_addr, coll_node->src_dst_addr, coll_node->cfrm_type, coll_node->tmv_stmp);
        fprintf(stdout, "\n%s REMOVE Collision node: [%d]->{%02x:%02x:%d} \n", tm_stamp, node_addr, coll_node->src_dst_addr, coll_node->cfrm_type, coll_node->tmv_stmp);
        free(coll_node);
        //printf("MAC_LAYER: Delete a collision node successfully!\n");
        return 0;
    }
    // Delete node in the middle of the list
    else{

        coll_node->prior->next = coll_node->next;
        coll_node->next->prior = coll_node->prior;
        get_tm_stamp(tm_stamp);
        fprintf(dly_map_fp, "\n%s REMOVE Collision node: [%d]->{%02x:%02x:%d} \n", tm_stamp, node_addr, coll_node->src_dst_addr, coll_node->cfrm_type, coll_node->tmv_stmp);
        fprintf(stdout, "\n%s REMOVE Collision node: [%d]->{%02x:%02x:%d} \n", tm_stamp, node_addr, coll_node->src_dst_addr, coll_node->cfrm_type, coll_node->tmv_stmp);
        free(coll_node);
        //printf("MAC_LAYER: Delete a collision node successfully!\n");
        return 0;
    }

    // We should never arrive here
    return -1;
}

int del_dly_map()
{
    int rc = 0;
    int node_id = 0;
    collision_node *temp = NULL;
    get_tm_stamp(tm_stamp);
    fprintf(dly_map_fp, "\n%s Start deleting delay map:\n", tm_stamp);
    fprintf(stdout, "\n%s Start deleting delay map:\n", tm_stamp);
    for(node_id = 0; node_id < DELAY_MAP_SIZE; node_id++){
        fprintf(dly_map_fp, "\n[%d]->{%02x:%p:%p}\n", node_id, dly_map[node_id].delay, dly_map[node_id].front, dly_map[node_id].rear);
        fprintf(stdout, "\n[%d]->{%02x:%p:%p}\n", node_id, dly_map[node_id].delay, dly_map[node_id].front, dly_map[node_id].rear);
        while(dly_map[node_id].front != NULL){
            temp = dly_map[node_id].front->next;
            rc = del_coll_node(dly_map[node_id].front);
            if(rc < 0){
                fprintf(stderr, "%s: Delete collision node in the collision list at [%d] failed!\n", ERROR_TITLE, node_id);
                fprintf(stderr, "%s: Delete delay map failed!\n", ERROR_TITLE);
                return -1;
            }
            dly_map[node_id].front = temp;
        }
        dly_map[node_id].front = NULL;
        dly_map[node_id].rear = NULL;
    }
    free(dly_map);
    dly_map = NULL;

    get_tm_stamp(tm_stamp);
    fprintf(dly_map_fp, "\n%s Delete delay map successfully!\n", tm_stamp);
    fprintf(stdout, "\n%s Delete delay map successfully!\n", tm_stamp);


    fclose(dly_map_fp);
    return 0;
}

