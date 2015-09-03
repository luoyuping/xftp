#include "xftp.h"

// 各个命令的解析函数
// 用户验证

int do_user(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    xftp_buffer_t *conn_buff = (xftp_buffer_t*)third_par; 
    ftp_cmd_t recv_cmd;

    // 暂时只有匿名用户，其他用户的登录暂不支持,所以判断匿名允许时，直接出错
    if (config_global.anonymous_enable == false) {
        if (!xftp_send_client_msg(user_env->conn_fd, ftp_send_msg[FTP_E_LOGIN])) {
            xftp_print_info(LOG_INFO, "Write Data To Client Error!");
        }
        return -1;
    } else if (user_env->is_login_in == true) {
        if (!xftp_send_client_msg(user_env->conn_fd, ftp_send_msg[FTP_S_ANONYMOUS])) {
            xftp_print_info(LOG_INFO, "Write Data To Client Error!");
            return -1;
        }
    } else if (strncmp(cmd->arg, "anonymous",strlen("anonymous") )== 0) {
        if (!xftp_send_client_msg(user_env->conn_fd, ftp_send_msg[FTP_NEED_PASS])) {
            xftp_print_info(LOG_INFO, "Write Data To Client Error!");
            return -1;
        }
        strcpy(user_env->user_name, "anonymous");
        /*assert(conn_buff->len >= 0 && conn_buff->len < conn_buff->size); // why i need to do so?*/

        // 密码验证
        bzero(conn_buff->buff, conn_buff->len);
        conn_buff->len = read(user_env->conn_fd, conn_buff->buff, conn_buff->size);
        if (conn_buff->len <= 0) {
            xftp_print_info(LOG_INFO, "Read Data From Client Error!");
            return -1;
        }
        int cmd_index = xftp_anaylse_buff(&recv_cmd, conn_buff);
        if (cmd_index == 1 && strncmp(recv_cmd.arg, "anonymous",strlen("anonymous")) == 0) {  // 密码验证
            user_env->is_login_in = true;
            if (!xftp_send_client_msg(user_env->conn_fd, ftp_send_msg[FTP_S_ANONYMOUS])) {
                xftp_print_info(LOG_INFO, "Write Data To Client Error!");
                return -1;
            }
            return 1;
        } else {
            if (!xftp_send_client_msg(user_env->conn_fd, ftp_send_msg[FTP_E_NO_USER_PASS])) {
                xftp_print_info(LOG_INFO, "Write Data To Client Error!");
                return -1;
            }
        }
    }
    else {  // 其他的用户
        if (!xftp_send_client_msg(user_env->conn_fd, ftp_send_msg[FTP_E_OTHER_USER])) {
            xftp_print_info(LOG_INFO, "Write Data To Client Error!");
            return -1;
        }
        return 2;  // 暂时不让其他的用户登陆
    }

    /*用ubuntu自带的ftp客户端测试，发现客户端会马上请求服务器的信息，所以，加上测试*/

        /*bzero(conn_buff->buff, conn_buff->len);*/
        /*conn_buff->len = read(user_env->conn_fd, conn_buff->buff, conn_buff->size);*/
        /*if (conn_buff->len <= 0) {*/
            /*xftp_print_info(LOG_INFO, "Read Data From Client Error!");*/
            /*return -1;*/
        /*}*/
        /*int cmd_index = xftp_anaylse_buff(&recv_cmd, conn_buff);*/
}

// 验证密码(程序内部不会显示调用)
int do_pass(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    if (user_env->is_login_in == true) {
        if (!xftp_send_client_msg(user_env->conn_fd, ftp_send_msg[FTP_S_ANONYMOUS])) {
            xftp_print_info(LOG_INFO, "Write Data To Client Error!");
            return -1;
        }
    return 1;
    } else {
        if (!xftp_send_client_msg(user_env->conn_fd, ftp_send_msg[FTP_E_NO_USER_PASS])) {
            xftp_print_info(LOG_INFO, "Write Data To Client Error!");
            return -1;
        }
    }
    return 2;

}

// 用户退出
int do_quit(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    return -1;
}

// 重新登录
int do_rein(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    user_env->is_login_in = 0;
    return 1;
}

int do_syst(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{

    if (!xftp_send_client_msg(user_env->conn_fd, ftp_send_msg[FTP_SYSTEM_TYPE])) {
        xftp_print_info(LOG_INFO, "Write Data To Client Error!");
        return -1;
    }
    return 1;
}

// 列出当前目录和子目录
int do_list(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
     
    return 2;
}

/*
下面的功能没有实现
*/





// 修改当前目录
int do_cwd(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    return 2;
}

// 返回上级目录
int do_cdup(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    return 2;
}

// 指定要改名的原文件
int do_rnfr(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    return 2;
}

// 指定要改名的新文件
int do_rnto(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    return 2;
}

// 中断上一个命令
int do_abor(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    return 2;
}

// 删除指定文件
int do_dele(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    return 2;
}

// 删除指定目录
int do_rmd(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    return 2;
}

// 创建指定目录
int do_mkd(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    return 2;
}

// 显示当前目录
int do_pwd(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    return 2;
}


// 列出当前目录和子目录的名字
int do_nlst(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    return 2;
}

// 不支持该方式，仅支持 PASV
int do_port(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    return 2;
}

// 设置数据交换的类型 {A(SCII),E(BCDIC),I(mage),L(ocal byte size)}
int do_type(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    return 2;
}

int do_pasv(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    return 0;
}

// 下载
int do_retr(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    return 0;
}

// 覆盖式上传
int do_stor(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    return 0;
}

// 追加式上传(不准备支持)
int do_appe(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    return 2;
}

// 显示帮助(不准备实现)
int do_help(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    return 2;
}

// 服务器反馈收到即可
int do_noop(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    return 1;
}

int do_smnt(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    return 1;
}

int do_acct(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    return 1;
}
int do_stru(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    return 1;
}

int do_mode(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    return 1;
}
int do_stou(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    return 1;
}
int do_allo(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    return 1;
}
int do_rest(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    return 1;
}
int do_site(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    return 1;
}
int do_stat(user_env_t *user_env, ftp_cmd_t *cmd,void* third_par)
{
    return 1;
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
