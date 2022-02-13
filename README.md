# Implementation of Underwater DOTS MAC Protocol based on Embedded Linux System


<!-- vim-markdown-toc GFM -->

* [Introduction](#introduction)
* [System Architecture](#system-architecture)
* [Implementation Highlight](#implementation-highlight)
    * [User Space](#user-space)
    * [Kernel Space](#kernel-space)
    * [Communication Module](#communication-module)
    * [Network Protocol](#network-protocol)
* [Recourses](#recourses)

<!-- vim-markdown-toc -->

## Introduction

Underwater acoustic communication networks (UACNs) play an important role in the Internet of Underwater Things (IoUT) and have broad application prospects in marine environment monitoring, underwater vehicle communication, underwater natural resource detection, etc. However, due to the complex underwater environment and the high deployment cost of underwater acoustic communication nodes, the construction of underwater acoustic communication networks faces many challenges. At present, most of researches on UACNs stay at the stage of theoretical analysis and software simulation, and only a few UACNs are implemented based on hardware. To build an UACNs experimental platform, in this article, we first built a lightweight communication network implementation framework based on the embedded Linux system and realized the hierarchical architecture of UACNs. After that, based on this framework, we implemented the DOTS protocol in the MAC layer of the UACNs. For this protocol, we applied the clock wheel algorithm to meet the protocol's requirements for multiple timer settings in a single process; combined the hash table, the doubly linked list, and a hash function based on N-array conversion to realize the efficient and dynamic resource management of the delay-conflict table; constructed a state vector to manage transmission states of data packets, which successfully realized the switching of data packets’ transmission states and the execution of data packets’ operations such as retransmission and back off. Finally, we built a 4-node linear topology network in the Linux system to test and analyze the implementation performances of the DOTS protocol. After that, we successfully ran the DOTS protocol on an embedded Linux system based on the ARM-FPGA platform and analyzed its operating performances. Our work reveals the feasibility of implementing the DOTS protocol using the embedded Linux operating system with a reasonable cost while maintaining the hierarchical characteristics of the network, which prospects the possibility of building more complex UACNs in the future.

## System Architecture

The system architecture is shown in the following graph:


```

   =-=-=-=-=-=-=-=-=- System Architecture -=-=-=-=-=-=-=-=- 
  |                                                        |
  |  __________        ___________________                 | 
  | |          |      |                   |                |
  | |          | <==> | Application Layer |                |    
  | |          |      |___________________|                |
  | |          |               |                           |
  | |          |       ________|__________                 |
  | |          |      |                   |                |
  | |          | <==> |  Transport Layer  |                |    
  | |          |      |___________________|                |
  | |   Log    |               |                           |
  | |          |       ________|__________       Linux     |         
  | |  System  |      |                   |                |
  | |          | <==> |    Networ Layer   |                |
  | |          |      |___________________|                |
  | |          |               |                           |
  | |          |       ________|__________                 |
  | |          |      |                   |                |
  | |          | <==> |  Data Link Layer  |                |
  | |__________|      |___________________|                |
  |                            |                           |
  |                    ________|__________                 |
  |                   |   AXI-DMA Driver  |                |
  |                    -------------------                 |
  |                            |                           |
  | - - - - - - - - - - - - - - - - - - - - - - - - - - -  | 
  |  __________________________|____________               |
  | |                                       |              |
  | | Communication Module (Physical Lyaer) |    FPGA      |
  | |_______________________________________|              |
  |                                                        |
  |=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-|                                     


```

## Implementation Highlight

### User Space

This project designed an network protocol implementation framework in the user space of Embedded Linux system, comparing with common way to 
create communication interface between **communication module** such as Telesonar Benthos and **Network Implementation Framework** such as ***NS3***, since
NS3 needs high computation and storage resources, it is not pratical in a resources-limited embedded system. To this end, we introduced out system,
which is a lightweight network implementation framework with better computation and storage efficiency.

the main highlights of this framwork is:
- Used **shared memory** to implement the Inter-Process Communication (IPS) between different network layer user spcace applications. 
- Innovated a storage and search algorithm for protocol-related data by using an **optimized hash table**.
- Implemented multiple timer mechanism by using the **time-wheel** algorithm and function pointer.

### Kernel Space

By using with an open-source [AXI-DMA driver](https://github.com/bperez77/xilinx_axidma), the project implemented the inboard communication between the **Programmable System (PS)** ARM component and 
**Programmable Logic (PL)** FPGA component on the [Xilinx ZYNQ-7000](https://www.xilinx.com/products/silicon-devices/soc/zynq-7000.html) embedded development board.

### Communication Module

In this project, we leveraged the communication module based on FPGA which is designed by the Underwater Sensor Network Lab at Zhejiang University
led by [Professor Fengzhong Qu](https://scholar.google.com/citations?user=LtsoDBQAAAAJ).

### Network Protocol

Based on this network implementation framwork, this project implemented the 
[DOTS: A Propagation Delay-Aware Opportunistic MAC Protocol for Mobile Underwater Network](https://ieeexplore.ieee.org/document/6708483)

## Recourses

The slice can be found [here](DOTS.pptx)*

 **Chinese version* **中文版本*
