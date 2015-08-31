#include "xftp.h"

// 全局配置定义
xftp_config_t config_global;

// 读取配置文件
bool xftp_read_config()
{
#ifndef FTP_DEBUG
	char *config_file = "/etc/xftp.conf";
#else
	char *config_file = "xftp.conf";
#endif
	FILE *config_stream;
	char read_buff[BUF_SIZE];
	char key[KEY_SIZE];
	char value[KEY_SIZE];

	// 简单处理下配置文件
	if ((config_stream = fopen(config_file, "r")) == NULL) {
		xftp_print_info(LOG_ERR, "Open The Config File Error!");
		return false;
	}
	
	while (fscanf(config_stream, "%s\n", read_buff) > 0) {
		if (read_buff[0] == '#') {
			continue;
		} else if (read_buff[0] == '[') {
			while (fscanf(config_stream, "%s = %s\n", key, value) == 2) {
				if (strcmp("ftp_path", key) == 0) {
					strcpy(config_global.ftp_path, value);
				} else if (strcmp("anonymous_enable", key) == 0) {
					if (strcmp("YES", value) == 0) {
						config_global.anonymous_enable = true;
					} else {
						config_global.anonymous_enable = false;
					}
				} else if (strcmp("upload_enable", key) == 0) {
					if (strcmp("YES", value) == 0) {
						config_global.upload_enable = true;
					} else {
						config_global.upload_enable = false;
					}
				} else if (strcmp("download_enable", key) == 0) {
					if (strcmp("YES", value) == 0) {
						config_global.download_enable = true;
					} else {
						config_global.download_enable = false;
					}
				} else if (strcmp("log_enable", key) == 0) {
					if (strcmp("YES", value) == 0) {
						config_global.log_enable = true;
					} else {
						config_global.log_enable = false;
					}
				}
			}
		}
	}
	
	fclose(config_stream);

	if (config_global.ftp_path[0] == '\0') {
		xftp_print_info(LOG_ERR, "The Config File Format Error!");
		return false;
	}

#if 	0
	printf("Config Info:\n");
	printf("ftp_path:%s\n", config_global.ftp_path);
	printf("anonymous_enable:%d\n", config_global.anonymous_enable);
	printf("upload_enable:%d\n", config_global.upload_enable);
	printf("download_enable:%d\n", config_global.download_enable);
	printf("log_enable:%d\n", config_global.log_enable);
#endif
	config_global.max_clients = MAX_CONNECT_USER; 			// 允许的最大客户数目与线程池上限
	config_global.thread_pool_size = THREAD_POOL_SIZE; 		// 默认的线程池大小
	config_global.thread_pool_add_size = THREAD_POOL_ADD_SIZE; 	// 线程池每次增加大小
	config_global.ftp_port = SERV_PORT; 				// FTP使用的端口号
	config_global.local_umask = 22; 				// 上传文件权限

	return true;
}

