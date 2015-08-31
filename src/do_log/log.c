#include "xftp.h"

// 定义错误日志缓冲区大小
#define ERR_BUFF 1024

// 处理输出信息
void xftp_print_info(int level, char *output_info)
{
	char err_buff[ERR_BUFF];

#ifndef FTP_DEBUG  // 非调试模式，编译时指定参数 
	if (level == LOG_DEBUG) {
		return;
	}
	if (errno == 0) {
		syslog(level, "%s\n", output_info);
	} else {
		strerror_r(errno, err_buff, ERR_BUFF);
		syslog(level, "%s - %s", output_info, err_buff);
	}
#else       // 调试运行模式
	time_t 	  timep;
	struct	  tm *p;
	
	time(&timep);
	p = localtime(&timep);

	if (errno == 0) {
		fprintf(stderr, "%02d:%02d:%02d Debug Info: %s\n",
				p->tm_hour, p->tm_min, p->tm_sec, output_info);
	} else {
		strerror_r(errno, err_buff, ERR_BUFF);
		fprintf(stderr, "%02d:%02d:%02d Debug Info: %s - %s\n",
				p->tm_hour, p->tm_min, p->tm_sec, output_info, err_buff);
	}
#endif	
}

