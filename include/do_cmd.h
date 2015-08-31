#ifndef DO_CMD_H_
#define DO_CMD_H_

// 命令处理的函数指针
typedef int (*p_do_cmd_func)(user_env_t *, ftp_cmd_t *);

// 命令解析函数指针的数组
extern p_do_cmd_func do_cmd_func[];

#endif 	// DO_CMD_H_
