CC = gcc
CFLAGS =  -g -O0
SRC = ${wildcard *.c}  #将当前目录下所有的.c文件名赋给SRC 
OBJS = ${patsubst %.c,%,$(SRC)}

#建立依赖关系
all:$(OBJS)

%:%.c
	$(CC) $(CFLAGS) -o $@ $^ -pthread

clean:
	$(RM) $(OBJS)
