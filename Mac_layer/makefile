#!/bin/bash
# -----------------------------------------------------------------
# FileName: Makefile.sh
# Date: 2020-01-13
# Author: jiftle
# Description:
#   构建配置文件
# 注意事项:
#       TAB作为缩进，不能是空格
# -----------------------------------------------------------------
# 编译器
CC = gcc
 
# 源代码目录
SRCDIR := $(PWD)
 
# 输出目标程序
TARGET = Mac_layer
 
# 默认
#   -fPIC 产生位置无关代码
all:
	$(CC) $(SRCDIR)/*.c -ldl -o $(TARGET)

# 清理
clean:
	rm $(TARGET) 
