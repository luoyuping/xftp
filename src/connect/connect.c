#include "xftp.h"

// 服务器等待连接函数
bool xftp_accept_client()
{
	int listen_fd;
	int accept_fd;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;

	if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		xftp_print_info(LOG_ERR, "Create Socket Error, What The Fuck !?");
		return false;
	}
	
	int optval = 1;
	if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (void *)&optval, sizeof(int)) < 0) {
		xftp_print_info(LOG_ERR, "Set Socket SO_REUSEADDR Error!");
		return false;
	}

	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(config_global.ftp_port);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		xftp_print_info(LOG_ERR, "Bind Scoket Error!");
		return false;
	}

	if (listen(listen_fd, LISTEN_SIZE) < 0) {
		xftp_print_info(LOG_ERR, "Listen Socket Error!");
		return false;
	}

	socklen_t client_addr_len = sizeof(client_addr);

	while (true) {
		bzero(&client_addr, sizeof(client_addr));
		accept_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_addr_len);
		if (accept_fd < 0) {
			xftp_print_info(LOG_ERR, "Accept Socket Error!");
			return false;
		}

		char info_buf[100];
		sprintf(info_buf, "New Connection Create. Client IP : %s", inet_ntoa(client_addr.sin_addr));
		xftp_print_info(LOG_INFO, info_buf);

		// 传值，注意在线程函数内部读取参数值，而非解引用
		int tag, thread_pool_want;
		do {
			// 非阻塞模式调用线程池接口
			tag = thread_pool_add_job(thread_pool_global, xftp_thread_job_entry, (void *)accept_fd, 0);
			if (tag == 1) {
				thread_pool_want = thread_pool_global->thread_num + config_global.thread_pool_add_size;
				if (thread_pool_want <= config_global.max_clients) {
					if (thread_pool_resize(thread_pool_global, thread_pool_want, thread_pool_want) != 0) {
						xftp_print_info(LOG_ERR, "Thread Pool Resize Error!");
						break;
					}
					xftp_print_info(LOG_INFO, "Thread Pool Resized Succeed!");
				} else {
					xftp_print_info(LOG_ERR, "Too Many Client!");
					close(accept_fd);
					break;
				}
			} else if (tag == -1) {
				xftp_print_info(LOG_ERR, "Add Thread Pool Job Error!");
				return false;
			}
		} while (tag != 0);
	}
}
