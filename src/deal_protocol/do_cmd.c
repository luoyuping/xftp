#include "xftp.h"
#define LS_BUFF_SIZE    9216
//所有的命令执行模块



// 用户验证
client_state_t do_user(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    xftp_buffer_t *conn_buff = (xftp_buffer_t*)third_par; 
    ftp_cmd_t recv_cmd;

    if (user_env->is_login_in == true) {
        if (!xftp_send_client_msg(user_env->conn_fd, ftp_send_msg[FTP_S_ANONYMOUS])) {
            xftp_print_info(LOG_INFO, "Write Data To Client Error!");
            return state_close;
        }
        return state_login;
    } 
    else if (config_global.anonymous_enable == false) { // 不支持匿名登陆
        // 开始匹配合法用户列表 
        // mark 
        if (!xftp_send_client_msg(user_env->conn_fd, ftp_send_msg[FTP_E_LOGIN])) {
            xftp_print_info(LOG_INFO, "Write Data To Client Error!");
        }
        return state_login ; // 用户重新登陆
    }
    //做匿名的登录的验证
    else if (strncmp(cmd->arg, "anonymous",strlen("anonymous") )== 0) {
        if (!xftp_send_client_msg(user_env->conn_fd, ftp_send_msg[FTP_NEED_PASS])) {
            xftp_print_info(LOG_INFO, "Write Data To Client Error!");
            return state_close;
        }
        strcpy(user_env->user_name, "anonymous");
        assert(conn_buff->len >= 0 && conn_buff->len < conn_buff->size); // why i need to do so?
        // 密码验证
        bzero(conn_buff->buff, conn_buff->len);
        conn_buff->len = read(user_env->conn_fd, conn_buff->buff, conn_buff->size);
        if (conn_buff->len <= 0) {
            xftp_print_info(LOG_INFO, "Read Data From Client Error!");
            return state_close;
        }

        // 本来这里要做密码的正则匹配，简化，随便就行
        user_env->is_login_in = true;
        if (!xftp_send_client_msg(user_env->conn_fd, ftp_send_msg[FTP_S_ANONYMOUS])) {
            xftp_print_info(LOG_INFO, "Write Data To Client Error!");
            return state_close;
        }
        return state_login;
    }
}

// 验证密码(程序内部不会显示调用)
client_state_t do_pass(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    if (user_env->is_login_in == true) {
        if (!xftp_send_client_msg(user_env->conn_fd, ftp_send_msg[FTP_S_ANONYMOUS])) {
            xftp_print_info(LOG_INFO, "Write Data To Client Error!");
            return state_close;
        }
        return state_login;
    } 
    else {
        if (!xftp_send_client_msg(user_env->conn_fd, ftp_send_msg[FTP_E_NO_USER_PASS])) {
            xftp_print_info(LOG_INFO, "Write Data To Client Error!");
            return state_close;
        }
        return state_login;
    }
}

// 用户退出
client_state_t do_quit(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    if(!xftp_send_client_msg(user_env->conn_fd,ftp_send_msg[FTP_BYE]))
    { 
        xftp_print_info(LOG_INFO,"write to clent date error in do_quit");
    } 
    return state_close;
}

// 重新初始化登录状态连接
client_state_t do_rein(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    user_env->is_login_in = 0;
    return state_login; // 这个不需要返回信息码吗？
}


client_state_t do_syst(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{

    if (!xftp_send_client_msg(user_env->conn_fd, ftp_send_msg[FTP_SYSTEM_TYPE])) {
        xftp_print_info(LOG_INFO, "Write Data To Client Error!");
        return state_close;
    }
    return state_login;
}

// 设置 被动模式
client_state_t do_pasv(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    int server_fd;
    user_env->passive_on = 1;
    // 简历套接字，等待客户的链接，在f非20的端口
    if(user_env->data_listen_fd  < 0)
    {
        server_fd= data_socket_init();  // 这种效率有点低。。。。
        user_env->data_listen_fd = server_fd;
    }
    else 
    {
        server_fd = user_env->data_listen_fd;
    }
    // 通过这个socket 读取他的本地端绑定的ip+ port
    struct sockaddr_in datatarans_server_addr;
    socklen_t len = sizeof(datatarans_server_addr); 
    getsockname(server_fd,&datatarans_server_addr,&len);
    int  port; 
    in_addr_t   ip;
    ip = ntohl(datatarans_server_addr.sin_addr.s_addr);
    port = ntohs(datatarans_server_addr.sin_port);

#ifdef FTP_DEBUG
    printf("the data transfer listen socket's ip:%d,port:%d\n",ip,port);
#endif
    char data_buff[100]={0};
    sprintf(data_buff,"%d %s (%d,%d,%d,%d,%d,%d)\r\n",227,"Entering Passive Mode",(ip>>24)&0xff,\
            (ip>>16)&0xff,(ip>>8)&0xff,ip & 0xff,(port>>8),port & 0xff);
#ifdef FTP_DEBUG
    printf("the data buff is:%s",data_buff);
#endif
    if(io_writen(user_env->conn_fd,data_buff,strlen(data_buff))!= strlen(data_buff))
    { 
        xftp_print_info(LOG_ERR,"send data to client error while feedback to pasv commmand\n");
        return state_close;
    }     
    return state_login;
}


// 列出当前目录和子目录
client_state_t do_list(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    if(user_env->data_listen_fd == -1)
    {
        if(!xftp_send_client_msg(user_env->conn_fd,ftp_send_msg[FTP_E_OPEN_CONN]))
      { 
            xftp_print_info(LOG_ERR,"send data to client error while feedback to do_list\n");
            return state_close;
        } 
        return state_login;
    }

    int data_fd = get_data_sockfd(user_env->data_listen_fd);
#if 0// 输出套接字的信息
    struct sockaddr_in temp;
    int len_temp  =  sizeof(temp);
    getsockname(data_fd,&temp,&len_temp);
    int  port; 
    in_addr_t   ip;
    ip = ntohl(temp.sin_addr.s_addr);
    port = ntohs(temp.sin_port);
    printf("ip :(%d,%d,%d,%d),port: %d\r\n",(ip>>24)&0xff,\
            (ip>>16)&0xff,(ip>>8)&0xff,ip & 0xff,port);
#endif
    if(data_fd < 0)
    {
        if(!xftp_send_client_msg(user_env->conn_fd,ftp_send_msg[FTP_E_OPEN_CONN]))
        { 
            xftp_print_info(LOG_ERR,"send data to client error while feedback to do_list\n");
            return state_close;
        } 
        return state_login;
    }
    //禁用nodelay;满负荷在发送数据
    int one = 1;
    setsockopt(data_fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
    // 进行数据的传输
    user_env->data_fd = data_fd;
    char send_data[LS_BUFF_SIZE];        
    int len = 0;
    if(strlen(cmd->arg) == 0)
    {
    //没有带参数
        len = ls(user_env,send_data,LS_BUFF_SIZE);
        if(len < 0)
        {
            xftp_print_info(LOG_ERR,"error while call ls in do_list");
            close(user_env->data_fd);
            return state_close;
        }
    } 

    else   // 查看单个文件的状态的信息
    {
        struct stat st;
        if(stat(cmd->arg,&st),errno = ENOENT)
        {
            sprintf(send_data,"%s\r\n","file not exist");
            len = strlen("file no exist");
            close(user_env->data_fd);
            return state_close;
        }
        else
        {
            int len = get_file_info(cmd->arg,send_data,LS_BUFF_SIZE);            
            if (len < 0)
            {
                xftp_print_info(LOG_ERR,"error while call ls in do_list");
                close(user_env->data_fd);
                return state_close;
            }
        }
    }

    if(!xftp_send_client_msg(user_env->conn_fd,ftp_send_msg[FTP_S_FSO]))
    { 
        xftp_print_info(LOG_ERR,"send data to client error while feedback to do_list\n");
        return state_close;
    } 
    if((io_writen(user_env->data_fd,send_data,len)) != len)
    {
        xftp_print_info(LOG_ERR,"send list data to client error");
        close(user_env->data_fd);
        return state_close;
    } 
    
    if (!xftp_send_client_msg(user_env->conn_fd, ftp_send_msg[FTP_S_SENDDATA_OVER])) {
        xftp_print_info(LOG_INFO, "Write Data To Client Error!");
        return state_close;
    }
    close(user_env->data_fd);  // 这就比较差劲了，客户端每次都是先调用do_pasv,然后调用do_list
    // 发送 226 代码  
    return state_login;
}

// 设置数据交换的类型 {A(SCII),E(BCDIC),I(mage),L(ocal byte size)}
client_state_t do_type(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    char filetype = *(cmd->arg);
#ifdef FTP_DEBUG
    printf("the file type  is:%s",cmd->arg);
#endif
    if(filetype == 'A')   // ASCII 编码数据传输
    { 
        user_env->filetype = 'A'; 
    } 
    else if(filetype == 'I')  // 二进制传输
    { 
        user_env->filetype = 'I'; 
    } 
    else if(filetype == 'E')
    { 
        user_env->filetype = 'E'; 
    } 
    else
    {
        if (!xftp_send_client_msg(user_env->conn_fd, ftp_send_msg[FTP_E_ARGS])) {
            xftp_print_info(LOG_ERR, "error while send message to client in command do_type()\n");
            return state_close;
        }
    }
    user_env->filetype = filetype;
    // 回复client 
    if (!xftp_send_client_msg(user_env->conn_fd, ftp_send_msg[FTP_S_COMMAND])) {
        xftp_print_info(LOG_ERR, "error while send message to client in command do_type()\n");
        return state_close;
    }
    return state_login;
}

// 显示当前目录
client_state_t do_pwd(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    char curr_dir[MAX_PATH] = {0};
    char* pwd   = getcwd(curr_dir,MAX_PATH);
    if(pwd == NULL)
    { 
        xftp_print_info(LOG_ERR,"getcwd error");
        return state_close;
    }  
    // 手动拼接字符串
    char send_buff[150] = {0};
    sprintf(send_buff,"%d %s\r\n",250,curr_dir); 
    if((io_writen(user_env->conn_fd,send_buff,strlen(send_buff))) != strlen(send_buff))
    { 
        xftp_print_info(LOG_ERR,"send data to client error");
        return state_close;
    } 
    return state_login;
}

// 修改当前目录
client_state_t do_cwd(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    if(cmd->arg[0] == 0)
    {
        if (!xftp_send_client_msg(user_env->conn_fd, ftp_send_msg[FTP_E_CWD])) {
            xftp_print_info(LOG_INFO, "Write Data To Client Error!");
            return state_close;
        }
    }
    int ret ;
#if 0
    printf("the path is :%s",cmd->arg);
#endif 
    if((ret=chdir(cmd->arg)),ret ==-1)
    {
        if (!xftp_send_client_msg(user_env->conn_fd, ftp_send_msg[FTP_E_CWD])) {
            xftp_print_info(LOG_INFO, "Write Data To Client Error!");
            return state_close;
        }
        return state_login;
    }
    if (!xftp_send_client_msg(user_env->conn_fd, ftp_send_msg[FTP_S_COMMAND])) {
        xftp_print_info(LOG_INFO, "Write Data To Client Error!");
        return state_close;
    }
     
    char curr_dir[MAX_PATH] = {0};
    char* pwd   = getcwd(curr_dir,MAX_PATH);
    if(pwd == NULL)
    { 
        xftp_print_info(LOG_ERR,"getcwd error");
        return state_close;
    }  
    strncpy(user_env->user_path,curr_dir,strlen(curr_dir)+1);
    return state_login;
}

// 返回上级目录
client_state_t do_cdup(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    if (!xftp_send_client_msg(user_env->conn_fd, ftp_send_msg[FTP_E_UNKNOW_CMD])) {
        xftp_print_info(LOG_INFO, "Write Data To Client Error!");
        return state_close;
    }
         
    return state_login;
}

// 下载,暂时不考虑文本模式，全部是二进制模式下载
client_state_t do_retr(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    if(user_env->data_listen_fd == -1)
    {
        if(!xftp_send_client_msg(user_env->conn_fd,ftp_send_msg[FTP_E_OPEN_CONN]))
        { 
            xftp_print_info(LOG_ERR,"send data to client error while feedback to do_retr\n");
            return state_close;
        } 
        return state_login;
    }
    int data_fd = get_data_sockfd(user_env->data_listen_fd);
    if(data_fd < 0)
    {
        if(!xftp_send_client_msg(user_env->conn_fd,ftp_send_msg[FTP_E_OPEN_CONN]))
        { 
            xftp_print_info(LOG_ERR,"send data to client error while feedback to do_list\n");
            return state_close;
        } 
        return state_login;
    }
    //禁用nodelay;满负荷在发送数据
    int one = 1;
    setsockopt(data_fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
    // 做参数的验证
    int filefd = open(cmd->arg,O_RDONLY);
    if(filefd < 0)
    {
#ifdef FTP_DEBUF
        perror("line 348:");
#endif
        xftp_send_client_msg(user_env->data_fd,"550 Can not open the file.\r\n");
    }

    if(!xftp_send_client_msg(user_env->conn_fd,ftp_send_msg[FTP_S_FSO]))
    { 
        xftp_print_info(LOG_ERR,"send data to client error while feedback to do_recv\n");
        close(data_fd);
        return state_close;
    } 

    // 开始发送
    // 使用sendfile发送
    struct stat stat_buf;
    fstat(filefd,&stat_buf);
    /*if((sendfile(data_fd,filefd,NULL,stat_buf.st_size)) != stat_buf.st_size)*/
    /*{*/
        /*[>xftp_send_client_msg(user_env->conn_fd,"")   <]*/
        /*xftp_print_info(LOG_ERR,"the data is  not send comoletely  in do_recv\n");*/
        /*close(data_fd);*/
        /*return state_close;*/
    /*}*/
    sendfile(data_fd,filefd,NULL,stat_buf.st_size);
    if(!xftp_send_client_msg(user_env->conn_fd,ftp_send_msg[FTP_S_SENDDATA_OVER]))
    { 
        xftp_print_info(LOG_ERR,"send data to client error while feedback to do_recv\n");
        close(data_fd);
        return state_close;
    } 
    close(data_fd);
    return state_login;
}

/*
下面的功能没有实现
*/
// 指定要改名的原文件
client_state_t do_rnfr(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    if (!xftp_send_client_msg(user_env->conn_fd, ftp_send_msg[FTP_E_UNKNOW_CMD])) {
        xftp_print_info(LOG_INFO, "Write Data To Client Error!");
        return state_close;
    }
         
    return state_login;
}

// 指定要改名的新文件
client_state_t do_rnto(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    if (!xftp_send_client_msg(user_env->conn_fd, ftp_send_msg[FTP_E_UNKNOW_CMD])) {
        xftp_print_info(LOG_INFO, "Write Data To Client Error!");
        return state_close;
    }
         
    return state_login;
}

// 中断上一个命令
client_state_t do_abor(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    if (!xftp_send_client_msg(user_env->conn_fd, ftp_send_msg[FTP_E_UNKNOW_CMD])) {
        xftp_print_info(LOG_INFO, "Write Data To Client Error!");
        return state_close;
    }
         
    return state_login;
}

// 删除指定文件
client_state_t do_dele(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    if (!xftp_send_client_msg(user_env->conn_fd, ftp_send_msg[FTP_E_UNKNOW_CMD])) {
        xftp_print_info(LOG_INFO, "Write Data To Client Error!");
        return state_close;
    }
         
    return state_login;
}

// 删除指定目录
client_state_t do_rmd(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    if (!xftp_send_client_msg(user_env->conn_fd, ftp_send_msg[FTP_E_UNKNOW_CMD])) {
        xftp_print_info(LOG_INFO, "Write Data To Client Error!");
        return state_close;
    }
         
    return state_login;
}

// 创建指定目录
client_state_t do_mkd(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    if (!xftp_send_client_msg(user_env->conn_fd, ftp_send_msg[FTP_E_UNKNOW_CMD])) {
        xftp_print_info(LOG_INFO, "Write Data To Client Error!");
        return state_close;
    }
         
    return state_login;
}



// 列出当前目录和子目录的名字
client_state_t do_nlst(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    if (!xftp_send_client_msg(user_env->conn_fd, ftp_send_msg[FTP_E_UNKNOW_CMD])) {
        xftp_print_info(LOG_INFO, "Write Data To Client Error!");
        return state_close;
    }
         
    return state_login;
}

// 不支持该方式，仅支持 PASV
client_state_t do_port(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    /*if (!xftp_send_client_msg(user_env->conn_fd, ftp_send_msg[FTP_E_UNKNOW_CMD])) {*/
    if (!xftp_send_client_msg(user_env->conn_fd, "500 please change to passive mode,now the server only support passive mode\r\n")) {
        xftp_print_info(LOG_INFO, "Write Data To Client Error!");
        return state_close;
    }
         
    return state_login;
}




// 覆盖式上传
client_state_t do_stor(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    if (!xftp_send_client_msg(user_env->conn_fd, ftp_send_msg[FTP_E_UNKNOW_CMD])) {
        xftp_print_info(LOG_INFO, "Write Data To Client Error!");
        return state_close;
    }
         
    return state_login;
}

// 追加式上传(不准备支持)
client_state_t do_appe(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    if (!xftp_send_client_msg(user_env->conn_fd, ftp_send_msg[FTP_E_UNKNOW_CMD])) {
        xftp_print_info(LOG_INFO, "Write Data To Client Error!");
        return state_close;
    }
         
    return state_login;
}

// 显示帮助(不准备实现)
client_state_t do_help(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    if (!xftp_send_client_msg(user_env->conn_fd, ftp_send_msg[FTP_E_UNKNOW_CMD])) {
        xftp_print_info(LOG_INFO, "Write Data To Client Error!");
        return state_close;
    }
         
    return state_login;
}

// 服务器反馈收到即可
client_state_t do_noop(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    if (!xftp_send_client_msg(user_env->conn_fd, ftp_send_msg[FTP_E_UNKNOW_CMD])) {
        xftp_print_info(LOG_INFO, "Write Data To Client Error!");
        return state_close;
    }
         
    return state_login;
}

client_state_t do_smnt(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    if (!xftp_send_client_msg(user_env->conn_fd, ftp_send_msg[FTP_E_UNKNOW_CMD])) {
        xftp_print_info(LOG_INFO, "Write Data To Client Error!");
        return state_close;
    }
         
    return state_login;
}

client_state_t do_acct(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    if (!xftp_send_client_msg(user_env->conn_fd, ftp_send_msg[FTP_E_UNKNOW_CMD])) {
        xftp_print_info(LOG_INFO, "Write Data To Client Error!");
        return state_close;
    }
         
    return state_login;
}
client_state_t do_stru(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    if (!xftp_send_client_msg(user_env->conn_fd, ftp_send_msg[FTP_E_UNKNOW_CMD])) {
        xftp_print_info(LOG_INFO, "Write Data To Client Error!");
        return state_close;
    }
         
    return state_login;
}

client_state_t do_mode(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    if (!xftp_send_client_msg(user_env->conn_fd, ftp_send_msg[FTP_E_UNKNOW_CMD])) {
        xftp_print_info(LOG_INFO, "Write Data To Client Error!");
        return state_close;
    }
         
    return state_login;
}
client_state_t do_stou(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    if (!xftp_send_client_msg(user_env->conn_fd, ftp_send_msg[FTP_E_UNKNOW_CMD])) {
        xftp_print_info(LOG_INFO, "Write Data To Client Error!");
        return state_close;
    }
         
    return state_login;
}
client_state_t do_allo(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    if (!xftp_send_client_msg(user_env->conn_fd, ftp_send_msg[FTP_E_UNKNOW_CMD])) {
        xftp_print_info(LOG_INFO, "Write Data To Client Error!");
        return state_close;
    }
         
    return state_login;
}
client_state_t do_rest(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    if (!xftp_send_client_msg(user_env->conn_fd, ftp_send_msg[FTP_E_UNKNOW_CMD])) {
        xftp_print_info(LOG_INFO, "Write Data To Client Error!");
        return state_close;
    }
         
    return state_login;
}
client_state_t do_site(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    if (!xftp_send_client_msg(user_env->conn_fd, ftp_send_msg[FTP_E_UNKNOW_CMD])) {
        xftp_print_info(LOG_INFO, "Write Data To Client Error!");
        return state_close;
    }
         
    return state_login;
}
client_state_t do_stat(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    if (!xftp_send_client_msg(user_env->conn_fd, ftp_send_msg[FTP_E_UNKNOW_CMD])) {
        xftp_print_info(LOG_INFO, "Write Data To Client Error!");
        return state_close;
    }
         
    return state_login;
}


// 命令解析函数指针的数组
p_do_cmd_func do_cmd_func[MAX_FTP_COMMAND] = {
    do_user, do_pass, do_quit, do_rein,
    do_cwd, do_cdup, do_rnfr, do_rnto,
    do_abor, do_dele, do_rmd, do_mkd,
    do_pwd, do_list, do_nlst, do_port,
    do_type, do_pasv, do_retr, do_stor,
    do_appe, do_help, do_noop,do_smnt,
    do_acct,do_stru,do_mode,do_stou,
    do_allo,do_rest,do_site,do_syst,
    do_stat};
