#ifndef TYPES_H_
#define TYPES_H_

#ifndef __cplusplus  // 编译宏
typedef enum _bool {
	false = 0,
	true = 1
}bool;
#endif

// 定义服务器数据缓冲区类型
typedef struct  xftp_buffer {
	char *buff;
	int  size;
	int  len;
}xftp_buffer_t;

#endif 	// TYPES_H_
