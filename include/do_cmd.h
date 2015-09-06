#ifndef DO_CMD_H_
#define DO_CMD_H_

// 命令处理的函数指针
// 第三个参数，是我后期自己新增的，如果具体的函数功能需要的话，那就使用啊

typedef  client_state_t(* p_do_cmd_func)(user_env_t *, ftp_cmd_t *,void* third_par);

// 命令解析函数指针的数组
extern p_do_cmd_func do_cmd_func[];

#endif 	// DO_CMD_H_
