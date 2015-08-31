#include "xftp.h"
// 客户处理线程的入口函数
void *xftp_thread_job_entry(void *arg)
{ 	
    user_env_t user_env;
    user_env.conn_fd = (int)arg;
    user_env.is_login_in = 0;  // false
    user_env.passive_on = 0;
    user_env.data_fd = 0;

    xftp_buffer_t *conn_buff = &conn_buff_global[user_env.conn_fd];  //获取存储数据的位置

#ifdef FTP_DEBUG
    xftp_print_info(LOG_INFO, "A Job Create!");
#endif
    // 客户端连接刚建立状态
    client_state_t client_state = state_conn;

    // 只要用户不处于已退出状态就保持状态机循环
    while (client_state != state_close) {
        switch (client_state) {
            case state_conn:
                if (!xftp_send_client_msg(user_env.conn_fd, ftp_send_msg[FTP_WELCOME])) {
                    xftp_print_info(LOG_INFO, "Write Data To Client Error!");
                    client_state = state_close;
                    break;
                }
                client_state = state_login;
                break;
            case state_login:
                assert(conn_buff->len >= 0 && conn_buff->len < conn_buff->size);
                bzero(conn_buff->buff, conn_buff->len);
                // 此处读一次即可
                conn_buff->len = read(user_env.conn_fd, conn_buff->buff, conn_buff->size);
                if (conn_buff->len <= 0) {
                    xftp_print_info(LOG_INFO, "Read Data From Client Error!");
                    client_state = state_close;
                    break;
                }
                // 解析读取的内容
                client_state = xftp_parse_cmd(&user_env, conn_buff);
                break;
            case state_quit:
                if (!xftp_send_client_msg(user_env.conn_fd, ftp_send_msg[FTP_BYE])) {
                    xftp_print_info(LOG_INFO, "Write Data To Client Error!");
                    return NULL;
                }
                client_state = state_close;
                break;
            default:
                break;
        }
    }

    // 关闭前一定重置缓冲区
    xftp_reset_one_buff(conn_buff);

    // 插入内存屏障，严格控制时序
    _barrier();

    // 所有的套接字关闭全部由状态机轮转到此处
    close(user_env.conn_fd);

#ifdef FTP_DEBUG
    xftp_print_info(LOG_INFO, "A Job Exit!");
#endif

    return NULL;
}

// 向客户端发送一条消息
bool xftp_send_client_msg(int conn_fd, char *msg)
{
    if (rio_writen(conn_fd, msg, strlen(msg)) > 0) {
        return true;
    }

    return false;
}

