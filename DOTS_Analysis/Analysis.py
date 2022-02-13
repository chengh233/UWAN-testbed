# -*- coding: utf-8 -*-
"""
Created on Mon Sep  7 17:25:39 2020

@author: Cheng
"""

import matplotlib.pyplot as plt

import numpy.matlib 
import numpy as np 
import math
import os


#=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-==-=-=-=-=-=-=-=-=-=-=-

fname1="./Data/cpu_rx_tx.txt"
#fname2="./Data/throughput_line_a.txt"

#throughput_line_v = numpy.loadtxt(fname1)
#throughput_line_a = numpy.loadtxt(fname2)
cpu_opr = numpy.loadtxt(fname1)

plt.rcParams['font.sans-serif'] = ['SimHei'] 
#plt.rcParams['axes.unicode_minus'] = False  

# Paper approach result display
# scatter t_plus and t_minus
plt.figure()
plt.grid(linestyle="--")  
ax = plt.gca()
ax.spines['top'].set_visible(False) 
ax.spines['right'].set_visible(False)  

##label
plt.xticks(fontsize=12, fontweight='bold') 
plt.yticks(fontsize=12, fontweight='bold')
plt.ylabel("CPU使用率 (%)", fontsize=13, fontweight='bold')
plt.xlabel("运行时间 (s)", fontsize=13, fontweight='bold')
plt.xlim(0, 500) 
plt.ylim(0, 100)

#plt.plot(throughput_line_v[:,0], throughput_line_v[:,1],color='b',linestyle='-', marker='^',markeredgecolor='b',markerfacecolor=(1, 1, 1, 0.2),markersize=8,label='基于N进制转换的哈希函数', linewidth = 1.2)
plt.plot(cpu_opr[:,0], cpu_opr[:,1],color='b',linestyle='-', marker='^',markeredgecolor='b',markerfacecolor=(1, 1, 1, 0.2),markersize=8,label='发送节点使用率', linewidth = 1.2)
plt.plot(cpu_opr[:,0], cpu_opr[:,2],color='r',linestyle='-', marker='o',markeredgecolor='r',markerfacecolor=(1, 1, 1, 0.2),markersize=8,label='接收节点使用率', linewidth = 1.2)
plt.plot(cpu_opr[:,0], cpu_opr[:,3],color='g',linestyle='-', marker='s',markeredgecolor='g',markerfacecolor=(1, 1, 1, 0.2),markersize=8,label='总使用率', linewidth = 1.2)

#plt.plot(throughput_line_a[:,0], throughput_line_a[:,1],color='r',linestyle='-', marker='o',markeredgecolor='r',markerfacecolor=(1, 1, 1, 0.2),markersize=8,label='Actual', linewidth = 1.2)

#legend
plt.legend(loc='upper right', numpoints=1)
leg = plt.gca().get_legend()
ltext = leg.get_texts()
plt.setp(ltext, fontsize=12, fontweight='bold') 

#plt.show()
plt.savefig("./Display/cpu_rx_tx.png",format="png",dpi=500,bbox_inches = 'tight')
#plt.savefig("D:/Applicaiton/NCSU_Summer_Intern/MASS communication for Constrained Devices/Code/MASS_v1/Display_2/overhead_"+"_"+str(M)+"_"+str(D)+str(m)+"_compare_n.png",format="png",dpi=500,bbox_inches = 'tight')

#=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-==-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-==-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-==-=-=-=-=-=-=-=-=-=-=-
