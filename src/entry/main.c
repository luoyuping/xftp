
#include "xftp.h"

int main(int argc, char *argv[])
{
	// 初始化服务器资源
	if(!xftp_init()) {
		exit(-1);
	}

	// 进入网络连接处理的主循环
	if (!xftp_accept_client()) {
		exit(-1);
	}

	// 释放资源并退出
	xftp_destroy();

	return EXIT_SUCCESS;
}

