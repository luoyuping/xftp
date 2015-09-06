#ifndef SUPPORT_LS_
#define SUPPORT_LS_ 
#include "xftp.h" 
int ls(user_env_t* user_env,char* buff,int length);

int get_file_info(const char *filename, char buff[], size_t len) ;

#endif 
