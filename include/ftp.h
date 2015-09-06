#ifndef FTP_H_
#define FTP_H_
#include "config.h" 
// 用户名长度
#define USER_NAME_LEN 20

// 用户登录服务器的环境变量
typedef struct user_env
{
	int is_login_in;			// 是否已经登录
	char user_name[USER_NAME_LEN];		// 登录的用户名
	int passive_on;			         // 数据传输的模式
	int conn_fd;				//服务端的套接字
	int data_fd;				//数据传输的套接字
    int data_listen_fd;          // 数据传输的监听套接字
    char filetype;
    char user_path[MAX_PATH];
}user_env_t;


// 定义单用户连接过程中可能的各个状态
typedef enum client_state {
	state_conn      = 0, 	// 刚连接
	state_login     = 1, 	// 命令处理
	state_quit      = 2, 	// 用户退出
	state_close     = 3 	// 连接关闭
}client_state_t;

// FTP 命令最长也就 4 个字母
#define MAX_CMD 5

// 最长的参数是路径
#define MAX_ARG MAX_PATH

// 定义FTP命令结构体
typedef struct ftp_cmd{
	char	cmd[MAX_CMD];
	char	arg[MAX_ARG];
}ftp_cmd_t;

// 客户处理线程的入口函数
void *xftp_thread_job_entry(void *arg);

// 向客户端发送一条消息
bool xftp_send_client_msg(int conn_fd, char *msg);

// 内存屏障
//#if __GNUC__ < 4
//static inline void _barrier(void)
//{ 
	//__asm__ volatile("":::"memory"); 
//}
//#else
//static inline void _barrier(void) 
//{ 
	//__sync_synchronize(); 
//}
//#endif

#endif 	// FTP_H_
