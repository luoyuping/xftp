#include "xftp.h"
// 客户处理线程的入口函数
void *xftp_thread_job_entry(void *arg)
{ 	
    client_state_t client_state = state_conn;  
    // 维护状态机的变量 ,这个变量很关键啊,对于这种多次交互的网络协议
    // 使用状态机来维护逻辑，处理异常，也是非常的方便，逻辑清晰啊
    
    user_env_t user_env;
    user_env.conn_fd =((int)arg); // 这里传过来的是int类型的fd，而不是fd的地址，是很有讲究的，看看主线程的函数栈就会明白,黑科技，@_@
    user_env.is_login_in = 0;  // false
    user_env.passive_on = 1;  //数据传输的模式:主动模式，被动模式，单端口模式,默认开启被动模式
    user_env.data_fd = -1;  // 进行数据传输的套接字
    user_env.data_listen_fd = -1;
    bzero(user_env.user_path,MAX_PATH);
    // 先读取config_global 中默认的目录
    strncpy(user_env.user_path,config_global.ftp_path,strlen(config_global.ftp_path));   

#ifdef FTP_DEBUG
    printf("the user default working path is:%s",user_env.user_path);
#endif


    xftp_buffer_t* tcp_buff = get_buff_for_client();
    if(tcp_buff == NULL)
    {
        xftp_print_info(LOG_ERR, "Malloc memory for the client err while call get_buff_for_client()");
        client_state = state_close; 
    }
#ifdef FTP_DEBUG
    xftp_print_info(LOG_INFO, "A Job Create!");
#endif
    // 客户端连接刚建立状态
    /*client_state_t client_state = state_conn;*/

    // 只要用户不处于已退出状态就保持状态机循环
    while (client_state != state_close) {
        switch (client_state) {
            case state_conn:   // 这个状态好像就是为了发欢迎登录的信息
                if (!xftp_send_client_msg(user_env.conn_fd, ftp_send_msg[FTP_WELCOME])) {
                    xftp_print_info(LOG_INFO, "Write Data To Client Error!");
                    client_state = state_close;
                    break;
                }
                client_state = state_login;
                break;
            case state_login:
                assert(tcp_buff->len >= 0 && tcp_buff->len < tcp_buff->size);
                bzero(tcp_buff->buff, tcp_buff->len);
                // 好，这里开始读取用户的数据 ,关键
                //
                /*conn_buff->len = read(user_env.conn_fd, conn_buff->buff, conn_buff->size);*/
                tcp_buff->len = io_read(user_env.conn_fd,tcp_buff->buff,tcp_buff->size);
                
                if (tcp_buff->len <= 0) {
                    xftp_print_info(LOG_INFO, "Read Data From Client Error!");
                    client_state = state_close;
                    break;
                }
                // 解析读取的内容
                client_state = xftp_parse_cmd(&user_env, tcp_buff);
                break;
            /*case state_quit:  // 这个状态是死状态，并没有鸟用  ? */
                /*if (!xftp_send_client_msg(user_env.conn_fd, ftp_send_msg[FTP_BYE])) {*/
                    /*xftp_print_info(LOG_INFO, "Write Data To Client Error!");*/
                    /*return NULL;*/
                /*}*/
                /*client_state = state_close;*/
                /*break;*/
            default:
                break;
        }
    }

    // 关闭前一定重置缓冲区,
    /*xftp_reset_one_buff(tcp_buff);*/
    
    // 插入内存屏障，严格控制时序
    /*_barrier();  // 目前没有搞懂 这是要干啥*/

    // 所有的套接字关闭全部由状态机轮转到此处
    if(user_env.conn_fd > 0)
    {   
        close(user_env.conn_fd);
    }
    if(user_env.data_listen_fd > 0)
    {
       close(user_env.data_listen_fd);
    }
    free_buff_for_client(tcp_buff); // 释放内存

#ifdef FTP_DEBUG
    xftp_print_info(LOG_INFO, "A Job Exit!");
#endif

    return NULL;
}

// 向客户端发送一条消息
bool xftp_send_client_msg(int conn_fd, char *msg)
{
    if (io_writen(conn_fd, msg, strlen(msg))== strlen(msg)) {
        return true;
    }
    return false;
}

