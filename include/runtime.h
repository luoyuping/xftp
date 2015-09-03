#ifndef RUNTIME_H_
#define RUNTIME_H_

// 声明客户连接的数据缓冲区指针
//extern xftp_buffer_t *conn_buff_global;

// 声明线程池指针
extern thread_pool_t *thread_pool_global;

// 初始化运行状态
bool xftp_init();

// 清理服务器占据资源
void xftp_destroy();

// 重置一个数据缓冲区
//bool xftp_reset_one_buff(xftp_buffer_t *buff);

// 增大一个数据缓冲区
bool xftp_resize_one_buff(xftp_buffer_t *buff);

// 修改程序当前工作目录为设置的目录
bool xftp_chdir();

// 当前系统时间打印函数
void xftp_print_time();

xftp_buffer_t* get_buff_for_client();
int free_buff_for_client(xftp_buffer_t* tcp_buff);
#endif 	// RUNTIME_H_
