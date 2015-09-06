#include "xftp.h"
#include "reply_message.h"
/*
 *定义全局变量的资源文件
 * 避免链接时，出现二义性
 */

// 发送给客户端的消息
// 响应代码
char *ftp_send_msg[] = {
/*   0  */	"150 File status okay; about to open data connection.\r\n",
/*   1  */	"200 Succeed to create data connection.\r\n",
/*   2  */	"200 Welcome to xftp server.\r\n",
/*   3  */	"220 Only one user is anonymous.\r\n",
/*   4  */	"221 Goodbye.\r\n",
/*   5  */	"230 User anonymous logged in.\r\n",
/*   6  */	"250 RMD command successful.\r\n",
/*   7  */	"250 Command succeed.\r\n",
/*   8  */	"250 File sucessfully deleted.\r\n",
/*   9  */	"257 Directory successfully created:\r\n",
/*  10  */	"331 Please specify the password.\r\n",
/*  11  */	"421 Failed to create data connection.\r\n",
/*  12  */	"450 No such file or directory.\r\n",
/*  13  */	"500 Unknown command.\r\n",
/*  14  */	"501 Arguments wrong, the file or directory does not exists!\r\n",
/*  15  */	"501 Diretory or file name is too long.\r\n",
/*  16  */	"501 Can't change directory.\r\n",
/*  17  */	"501 Can't rename this file.\r\n",
/*  18  */	"503 You have already logged in.\r\n",
/*  19  */	"530 Login incorrect.\r\n",
/*  20  */	"530 Please login with USER and PASS.\r\n",
/*  21  */	"550 Permission denied.\r\n",
/*  22  */	"215 linux.\r\n",
/*  23  */  "504 useless argument.\r\n",
/*  24  */  "202 you hava already set  to pasv mode.\r\n ",
/*  25  */  "425 can not open the connnect.\r\n",
/*  26  */  "226 data transfer ends.\r\n"
};

