#ifndef STATUS_H_
#define STATUS_H_

extern char* ftp_send_msg[];
// 定义字符串所在编号
#define   FTP_S_FSO       	 0  	// 150 File status okay; about to open data connection.
#define   FTP_S_CONN      	 1  	// 200 Succeed to create data connection.
#define   FTP_WELCOME     	 2  	// 200 Welcome to xftp server.
#define   FTP_E_OTHER_USER   	 3  	// 220 Only one user is anonymous.
#define   FTP_BYE   	 	 4  	// 221 Goodbye.
#define   FTP_S_ANONYMOUS   	 5  	// 230 User anonymous logged in.
#define   FTP_S_RMD   		 6  	// 250 RMD command successful.
#define   FTP_S_COMMAND 	 7  	// 250 Command succeed.
#define   FTP_S_FILE_DEL   	 8  	// 250 File sucessfully deleted.  #define   FTP_S_CREAT_DIR   	 9  	// 257 Directory successfully created:
#define   FTP_NEED_PASS  	10  	// 331 Please specify the password.
#define   FTP_E_CONN  		11  	// 421 Failed to create data connection.
#define   FTP_E_NO_FILE  	12  	// 450 No such file or directory.
#define   FTP_E_UNKNOW_CMD 	13 	// 500 Unknown command.
#define   FTP_E_ARGS_WRONG  	14  	// 501 Arguments wrong, the file or directory does not exists!
#define   FTP_E_ARGS_LONG  	15  	// 501 Diretory or file name is too long.
#define   FTP_E_CHG_DIR  	16  	// 501 Can't change directory.
#define   FTP_E_RENAME  	17  	// 501 Can't rename this file.
#define   FTP_E_NO_LOGIN  	18  	// 503 You have already logged in.
#define   FTP_E_LOGIN		19  	// 530 Login incorrect.
#define   FTP_E_NO_USER_PASS 	20  	// 530 Please login with USER and PASS.
#define   FTP_E_PERMISSION  	21  	// 550 Permission denied.

// 用户名长度
#define USER_NAME_LEN 20

// 用户登录服务器的环境变量
typedef struct user_env
{
	int is_login_in;			// 是否已经登录
	char user_name[USER_NAME_LEN];		// 登录的用户名
	_Bool passive_on;			// 是否为被动模式
	int conn_fd;				//服务端的套接字
	int data_fd;				// 数据的位置
}user_env_t;
#endif 	// STATUS_H_
