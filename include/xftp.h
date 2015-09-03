#ifndef XFTP_H_
#define XFTP_H_

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <syslog.h>
#include <fcntl.h>

#include "types.h"
#include "connect.h"
#include "thread_pool.h"
#include "runtime.h"
#include "config.h"
#include "rio.h"
#include "log.h"
#include "ftp.h"
#include "reply_message.h"
#include "parse_cmd.h"
#include "do_cmd.h"

// 服务器名称
#define SERVER_NAME "xftp Server"

// 服务器端口
#ifdef FTP_DEBUG
	#define SERV_PORT 9527
#else
	#define SERV_PORT 21
#endif

// 服务器最大连接数
#define MAX_CONNECT_SIZE 1024

// 服务端最大支持用户数
#define MAX_CONNECT_USER 100

// 默认线程池大小
#ifdef  FTP_DEBUG
    #define THREAD_POOL_SIZE 3
#else 
    #define THREAD_POOL_SIZE 10
#endif

// 线程池每次增加粒度
#define THREAD_POOL_ADD_SIZE 10

// listen() 的 backlog 队列长度
#define LISTEN_SIZE 20

// 套接字缓冲区默认尺寸
#define BUFF_LENGTH 4096

// 套接字缓冲区增加粒度(经验值)
#define BUFF_ADD_LENGTH 128

#endif 	// XFTP_H_

