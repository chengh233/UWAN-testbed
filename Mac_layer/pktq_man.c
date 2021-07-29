/**
 * @file pktq_man.c
 * @date Thursday, Mar 18, 2021 at 22:57:44 UTC+8
 * @author Cheng Huang (cheng_huang@zju.edu.cn)
 * 
 * This file is used for implementing DATA packet manager for
 * each adjacent node. For each adjacent node, we create a data
 * packet queue. By using the following function, we can add new
 * data packet to the queue, delete data packets after sending them,
 * modify the status of each data packet to realize re-transmission caused
 * by the failure of control frame reception at the receiving nodes or 
 * collision detection in the delay map.
 * 
 * @version 1.0
 * 
 **/

#include "pktq_man.h"
static FILE *pkt_q_fp = NULL;
static char tm_stamp[28];
//node_manage node_man[MAX_NODE_NUM];
node_manage *node_man = NULL;

// Initialize node manager
int ini_node_manage()
{
    // Initialize node manager
    pkt_q_fp = fopen("pktq_man.log", "w");
    memset(tm_stamp, 0, sizeof(tm_stamp));

    node_man = (node_manage *)calloc(MAX_NODE_NUM, sizeof(node_manage));

    //memset(node_man, 0, sizeof(node_man));
    return 0;
}

/* 
 *-----------------------------------------------------------------------------
 * Function: add_pkt
 * Description: Add new packet nodes to the packet queue from buffer in which 
 * we contained the packets from network layer.
 * Calls: calloc, memcpy, memset
 * Called By: DOTS.c
 * Table Accessed: none
 * Table Updated: none
 * Input: buffer ptr, data size(int), destination address(int)
 * Output: log file
 * Return: function status
 * Others: none
 *-----------------------------------------------------------------------------
 */
int add_pkt(u_int8_t *buf, size_t data_size)
{
    // Create a new packet node on heap
    pkt_queue_node *pkt_q_node = NULL;
    pkt_q_node = (pkt_queue_node *)calloc(1, sizeof(pkt_queue_node));

    if(pkt_q_node == NULL){
        fprintf(stderr, "%s: Create new packet nodes failed!\n", ERROR_TITLE);
        return -1;
    }

    // Initialize data in the new packet node
    pkt_q_node->dst_addr = (u_int8_t)(0x0f & buf[data_size - 1]);                // the last byte in the network packet is the address of next hop
    memcpy(pkt_q_node->DATA, buf, sizeof(pkt_q_node->DATA));                         // Copy data from buffer to packet node
    pkt_q_node->send_stat = 0;
    pkt_q_node->backoff_stat = 0;
    memset(pkt_q_node->cfrm_stat, 0, sizeof(pkt_q_node->cfrm_stat));        // Initialize control frame matrix status
    pkt_q_node->prior = NULL;
    pkt_q_node->next = NULL;


    int pkt_index = 0;
    pkt_index = (int)(pkt_q_node->dst_addr) - 1;

    get_tm_stamp(tm_stamp);
    fprintf(pkt_q_fp, "\n%s ADD DATA Packet: [%d]->{%02x:%s}\n", tm_stamp, pkt_index, pkt_q_node->dst_addr, pkt_q_node->DATA);
    fprintf(stdout, "\n%s ADD DATA Packet: [%d]->{%02x:%s}\n", tm_stamp, pkt_index, pkt_q_node->dst_addr, pkt_q_node->DATA);

    // if queue is empty, we let front ptr point to the new packet node
    if(node_man[pkt_index].front == NULL){
        node_man[pkt_index].front = pkt_q_node;
        node_man[pkt_index].rear = node_man[pkt_index].front;
        node_man[pkt_index].update_stat++;
        //printf("MAC_LAYER: Add a new packet node to the empty queue successfully!\n");
        return 0;
    }
    // if queue is not empty, we add new packet node to the end of the queue
    else{
        pkt_q_node->prior = node_man[pkt_index].rear;
        node_man[pkt_index].rear->next = pkt_q_node;
        node_man[pkt_index].rear = pkt_q_node;
        node_man[pkt_index].update_stat++;
        //printf("MAC_LAYER: Add a new packet node to the queue successfully!\n");
        return 0;
    }

    // we should never arrive here!
    return -1;
}

int del_pkt(pkt_queue_node *pkt_node)
{
    if(pkt_node == NULL){
        fprintf(stderr, "%s: Delete packet node failed! Packet node is empty!\n", ERROR_TITLE);
        return -1;
    }
    int pkt_index = 0;
    pkt_index = (int)(pkt_node->dst_addr) - 1;
    // Delete only node in the list
    if(pkt_node->prior == NULL && pkt_node->next == NULL){
        node_man[pkt_index].front = NULL;
        node_man[pkt_index].rear = NULL;
        get_tm_stamp(tm_stamp);
        fprintf(pkt_q_fp, "\n%s REMOVE DATA Packet: [%d]->{%02x:%s}\n", tm_stamp, pkt_index, pkt_node->dst_addr, pkt_node->DATA);
        fprintf(stdout, "\n%s REMOVE DATA Packet: [%d]->{%02x:%s}\n", tm_stamp, pkt_index, pkt_node->dst_addr, pkt_node->DATA);

        free(pkt_node);

        return 0;
    }
    // Delete node in the end of the list
    else if(pkt_node->prior != NULL && pkt_node->next == NULL){
        node_man[pkt_index].rear = pkt_node->prior;
        node_man[pkt_index].rear->next = NULL;
        get_tm_stamp(tm_stamp);
        fprintf(pkt_q_fp, "\n%s REMOVE DATA Packet: [%d]->{%02x:%s}\n", tm_stamp, pkt_index, pkt_node->dst_addr, pkt_node->DATA);
        fprintf(stdout, "\n%s REMOVE DATA Packet: [%d]->{%02x:%s}\n", tm_stamp, pkt_index, pkt_node->dst_addr, pkt_node->DATA);

        free(pkt_node);

        //printf("MAC_LAYER: Delete a pakcet node successfully!\n");

        return 0;
    }
    // Delete node in the head of the list
    else if (pkt_node->prior == NULL && pkt_node->next != NULL){
        node_man[pkt_index].front = pkt_node->next;
        node_man[pkt_index].front->prior = NULL;
        get_tm_stamp(tm_stamp);
        fprintf(pkt_q_fp, "\n%s REMOVE DATA Packet: [%d]->{%02x:%s}\n", tm_stamp, pkt_index, pkt_node->dst_addr, pkt_node->DATA);
        fprintf(stdout, "\n%s REMOVE DATA Packet: [%d]->{%02x:%s}\n", tm_stamp, pkt_index, pkt_node->dst_addr, pkt_node->DATA);

        free(pkt_node);
        //printf("MAC_LAYER: Delete a pakcet node successfully!\n");
        return 0;
    }
    // Delete node in the middle of the list
    else{
        pkt_node->prior->next = pkt_node->next;
        pkt_node->next->prior = pkt_node->prior;
        get_tm_stamp(tm_stamp);
        fprintf(pkt_q_fp, "\n%s REMOVE DATA Packet: [%d]->{%02x:%s}\n", tm_stamp, pkt_index, pkt_node->dst_addr, pkt_node->DATA);
        fprintf(stdout, "\n%s REMOVE DATA Packet: [%d]->{%02x:%s}\n", tm_stamp, pkt_index, pkt_node->dst_addr, pkt_node->DATA);

        free(pkt_node);
        //printf("MAC_LAYER: Delete a pakcet node successfully!\n");
        return 0;

    }


    // We should never arrive here
    return -1;
}




/* 
 *-----------------------------------------------------------------------------
 * Function: delete node manager
 * Description: Delete all packet nodes in the node manager
 * Calls: del_pkt
 * Called By: Main
 * Table Accessed: none
 * Table Updated: none
 * Input: none
 * Output: log file
 * Return: function status
 * Others: none
 *-----------------------------------------------------------------------------
 */

int del_node_manager()
{
    int i = 0;
    // traverse all nodes in the node manager
    pkt_queue_node *temp = NULL;
    get_tm_stamp(tm_stamp);
    fprintf(pkt_q_fp, "\n%s Start deleting node manager:\n", tm_stamp);
    fprintf(stdout, "\n%s Start deleting node manager:\n", tm_stamp);

    for(i = 0; i < MAX_NODE_NUM; i++){
        fprintf(pkt_q_fp, "\n[%d]->{%d:%p:%p}\n", i, node_man[i].update_stat, node_man[i].front, node_man[i].rear);
        fprintf(stdout, "\n[%d]->{%d:%p:%p}\n", i, node_man[i].update_stat, node_man[i].front, node_man[i].rear);
        while(node_man[i].front != NULL){
            //printf("Node [%d] has packets!\n", i);

            fprintf(pkt_q_fp, "\n\t{%d:%d}-{%d:%d:%d:%d}\n\t{%02x:%s}\n",\
             node_man[i].front->send_stat, node_man[i].front->backoff_stat,\
             node_man[i].front->cfrm_stat[RTS_TX_NUM], node_man[i].front->cfrm_stat[CTS_RX_STAT],\
             node_man[i].front->cfrm_stat[CTS_RX_CK_STAT], node_man[i].front->cfrm_stat[ACK_RX_STAT],\
             node_man[i].front->dst_addr, node_man[i].front->DATA);

            fprintf(stdout, "\n\t{%d:%d}-{%d:%d:%d:%d}\n\t{%02x:%s}\n",\
             node_man[i].front->send_stat, node_man[i].front->backoff_stat,\
             node_man[i].front->cfrm_stat[RTS_TX_NUM], node_man[i].front->cfrm_stat[CTS_RX_STAT],\
             node_man[i].front->cfrm_stat[CTS_RX_CK_STAT], node_man[i].front->cfrm_stat[ACK_RX_STAT],\
             node_man[i].front->dst_addr, node_man[i].front->DATA);

            temp = node_man[i].front->next;
            //printf("temp status is '%02x'\n", temp);
            free(node_man[i].front);
            node_man[i].front = temp;
            //printf("new front status is '%p'\n", node_man[i].front);
        }
        node_man[i].front = NULL;
        node_man[i].rear = NULL;
    }

    free(node_man);
    node_man = NULL;
    
    get_tm_stamp(tm_stamp);
    fprintf(pkt_q_fp, "\n%s Delete node manager successfully!\n", tm_stamp);
    fprintf(stdout, "\n%s Delete node manager successfully!\n", tm_stamp);

    fclose(pkt_q_fp);
    return 0;
}
