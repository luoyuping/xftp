#ifndef PARSE_CMD_H_
#define PARSE_CMD_H_

// 最多计划支持 23 个命令
#define MAX_FTP_COMMAND 33

// 解析读取到的内容
client_state_t xftp_parse_cmd(user_env_t *user_env, xftp_buffer_t *conn_buff);

// 从缓冲区中解析出命令
int xftp_anaylse_buff(ftp_cmd_t *recv_cmd, xftp_buffer_t *conn_buff);

#endif 	// PARSE_CMD_H_
