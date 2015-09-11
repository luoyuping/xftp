
#include "xftp.h"

// 标准 FTP 服务器 支持的命令(按照本程序支持的先后顺序排序)
// 总共33 条命令
static char *ftp_command[] = {
/*    0   */		"USER", 	// 登录的用户名
/*    1   */		"PASS", 	// 密码
/*    2   */		"QUIT", 	// 退出
/*    3   */		"REIN", 	// 重新登录
/*    4   */		"CWD", 		// 改变当前目录
/*    5   */		"CDUP", 	// 在当前程序或目录上返回信息
/*    6   */		"RNFR", 	// 指定需要改名的原文件
/*    7   */		"RNTO", 	// 指定需要改名的新文件名
/*    8   */		"ABOR", 	// 取消前一指令
/*    9   */		"DELE", 	// 在服务器上删除指定文件
/*   10   */		"RMD", 		// 在服务器上删除指定目录
/*   11   */		"MKD", 		// 在服务器上创建目录       
/*   12   */		"PWD", 		// 显示当前目录
/*   13   */		"LIST", 	// 列出当前目录下所有文件和子目录
/*   14   */		"NLST", 	// 列出当前目录下所有文件和子目录的名字
/*   15   */		"PORT", 	// IP 地址和两字节的端口
/*   16   */		"TYPE", 	// 设置数据交换的类型{A(SCII),E(BCDIC),I(mage),L(ocal byte size)}
/*   17   */		"PASV", 	// 获取数据传输端口
/*   18   */		"RETR", 	// 从服务器上找回（复制）文件
/*   19   */		"STOR", 	// 用覆盖方式上传一个文件到服务器
/*   20   */		"APPE", 	// 用追加方式上传一个文件到服务器
/*   21   */		"HELP", 	// 获取帮助
/*   22   */		"NOOP", 	// 无动作，除了来自服务器上的承认
/*   23   */		"SMNT", 	// 挂载指定文件结构
/*   24   */		"ACCT", 	// 系统特权帐号
/*   25   */		"STRU", 	// 数据结构（F=文件，R=记录，P=页面）
/*   26   */		"MODE", 	// 传输模式（S=流模式，B=块模式，C=压缩模式）
/*   27   */		"STOU", 	// 储存文件到服务器名称上
/*   28   */		"ALLO", 	// 为服务器上的文件存储器分配字节
/*   29   */		"REST", 	// 指定重新下载的字节数
/*   30   */		"SITE", 	// 由服务器提供的站点特殊参数
/*   31   */		"SYST", 	// 返回服务器使用的操作系统
};

client_state_t xftp_parse_cmd(user_env_t *user_env, xftp_buffer_t *tcp_buf)
{
	ftp_cmd_t recv_cmd;  // 解析后的结果放在这～ 
	int cmd_index = xftp_anaylse_buff(&recv_cmd, tcp_buf);

    // 未登录的时候拒绝一切非登录指令  
    // cmd_index = 0 用户验证
    // cmd_index = 2 用户退出
    if ((user_env->is_login_in == false) && (cmd_index != 0) ) 
    {  
        if (!xftp_send_client_msg(user_env->conn_fd, ftp_send_msg[FTP_E_NO_USER_PASS])) {
            xftp_print_info(LOG_INFO, "Write Data To Client Error!");
        }
        return state_close;
    }
    // 已登录遭遇位置指令回复无法识别
    else if ((user_env->is_login_in == true) && (cmd_index == -1) ) 
    {
        if (!xftp_send_client_msg(user_env->conn_fd, ftp_send_msg[FTP_E_UNKNOW_CMD])) {
            xftp_print_info(LOG_INFO, "Write Data To Client Error!");
        }
        return state_close; // 可以考虑return state_login
    }
    // 请求登录或者已登录发送命令
    else
    {
        // 通过索引调用指令处理函数,指令处理函数设置为三种返回值,cmd_state 
        client_state_t cmd_state = do_cmd_func[cmd_index](user_env, &recv_cmd,(void*)tcp_buf);  // 通过函数指针数组索引到函数，并传入两个参数
        return cmd_state;
    }
}


// 从缓冲区中解析出命令
/*
 *下面说明函数的返回值
  + 执行正确返回 ftp_command[] 数组中的命令的下标 index ，数组的定义在该文件顶部;
  + 解析命令出错 ，返回 -1;
  + 解析得到的命令 和参数（如果有的话） 存放在ftp_cmd_t 结构体中
 */

/*
 *测试的客户端是linux的ftp客户端，除了换行符的问题之外，应该能满足win平台的ftp客户端
 */


int xftp_anaylse_buff(ftp_cmd_t *recv_cmd, xftp_buffer_t *tcp_buf)
{
    // 命令行中空格的位置
#ifdef FTP_DEBUG
    bzero(recv_cmd->cmd,strlen(recv_cmd->cmd));
    bzero(recv_cmd->arg,strlen(recv_cmd->arg));
#endif

    int blank_index = 0;
    // 最短的命令 + '\n' 也有 4 字节
    if (tcp_buf->len < 4) {
        return -1;
    }
    bool has_arg = true;

    // 格式不正确不考虑粘包处理,直接反馈无法识别
    // 命令 3-4 个字节
    if (tcp_buf->buff[3] == ' ') {
        blank_index = 3;
    } else if (tcp_buf->buff[4] == ' ') {
        blank_index = 4;
    } else { // 下面处理没有参数的情况
        if (tcp_buf->buff[3] == '\r' || tcp_buf->buff[3] == '\n') {
            blank_index = 3;  // 这里不要将has_arg 置为false吗？
            has_arg = false ;// 我的想法是这需要
        } else if (tcp_buf->buff[4] == '\r' || tcp_buf->buff[4] == '\n') {
            blank_index = 4;
            has_arg = false;
        } else {
            return -1;
        }
    }

    // 拷贝命令
    strncpy(recv_cmd->cmd, tcp_buf->buff, blank_index);
    recv_cmd->cmd[blank_index] = '\0';

    // 处理参数 
    if (has_arg) {
        // 拷贝参数，按照惯例支持 \r\n 和 \n
        // 2015 09 09 20:54 这里的获取函数的参数，有问题，没有把不可见的字符清理干净,自能骂娘
        if (tcp_buf->buff[tcp_buf->len-2] == '\r' && tcp_buf->buff[tcp_buf->len-1] == '\n') {
            strncpy(recv_cmd->arg, tcp_buf->buff+blank_index+1, tcp_buf->len-blank_index-3);
            recv_cmd->arg[tcp_buf->len-blank_index-2] = '\0';
        } else if (tcp_buf->buff[tcp_buf->len-1] == '\n') {
            strncpy(recv_cmd->arg, tcp_buf->buff+blank_index+1, tcp_buf->len-blank_index-2);
            recv_cmd->arg[tcp_buf->len-blank_index-1] = '\0';
        } else {
            // 命令没读取完，此时最好应该判断是否断包
            return -1;
        }
    }

#ifdef FTP_DEBUG
    printf("Debug Info : A Client Send A Command. cmd : %s  arg : %s\n", recv_cmd->cmd, recv_cmd->arg);
#endif
    /*
    // 命令字母转换到大写
    int i;
    for (i = 0; i < MAX_CMD; i++) {
    recv_cmd->cmd[i] &= 0x5F;
    }
    */
    // 比较参数并返回索引
    int index;
    for (index = 0; index < MAX_FTP_COMMAND; index++) {
        if (strcmp(ftp_command[index], recv_cmd->cmd) == 0) {
            return index;
        }		
    }

    return -1;
}

