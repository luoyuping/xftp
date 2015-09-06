#include "xftp.h"
#define MAXLINE 1024 

int get_file_info(const char *filename, char buff[], size_t len) {
    char    mode[] = "----------";
    char    timebuf[MAXLINE];  
    int     timelen, off = 0;  
    struct passwd *pwd; 
    struct group *grp;
    struct tm *ptm;
    struct stat st;
    if (-1 == stat(filename, &st)) 
    {       
        /*FTPD_DEBUG("stat error: %s\n", strerror(errno));        */
        xftp_print_info(LOG_ERR,"get file information err");
        return -1;    
    } 
    if (S_ISDIR(st.st_mode))    
        mode[0] = 'd';
    if (st.st_mode & S_IRUSR)  
        mode[1] = 'r'; 
    if (st.st_mode & S_IWUSR)  
        mode[2] = 'w';
    if (st.st_mode & S_IXUSR)    
        mode[3] = 'x';  
    if (st.st_mode & S_IRGRP) 
        mode[4] = 'r'; 
    if (st.st_mode & S_IWGRP)   
        mode[5] = 'w'; 
    if (st.st_mode & S_IXGRP)
        mode[6] = 'x';  
    if (st.st_mode & S_IROTH)    
        mode[7] = 'r';  
    if (st.st_mode & S_IWOTH) 
        mode[8] = 'w';  
    if (st.st_mode & S_IXOTH)   
        mode[9] = 'x';
    mode[10] = '\0';  
    off += snprintf(buff + off, len - off, "%s", mode);
    off += snprintf(buff + off, len - off, "%2d", 1);
    if (NULL == (pwd = getpwuid(st.st_uid))) 
    {
        /*FTPD_DEBUG("getpwuid error: %s\n", strerror(errno));      */
        xftp_print_info(LOG_ERR,"get uid error in get_file_info");
        return  -1;   
    } 
    off += snprintf(buff + off, len - off, " %4s", pwd->pw_name); 
    if (NULL == (grp = getgrgid(st.st_gid))) 
    {  
        xftp_print_info(LOG_ERR,"get gid error in get_file_info");
        return -1;   
    } 
    off += snprintf(buff + off, len - off, " %4s", (char *) grp->gr_name);
    off += snprintf(buff + off, len - off, " %*d", 8,(int) st.st_size);
    ptm = localtime(&st.st_mtime); 
    if (ptm != NULL && (timelen = strftime(timebuf, sizeof(timebuf), " %b %d %H:%S", ptm)) > 0) 
    {    
        timebuf[timelen] = '\0';   
        off += snprintf(buff + off, len - off, "%s", timebuf); 
    }
    else
    {   
        xftp_print_info(LOG_ERR,"error in get_file_info");
        return -1; 
    }  
    off += snprintf(buff + off, len - off, " %s\r\n", filename);   
    return off;
}

int ls(user_env_t* user_env,char* buff,int length)
{
    DIR* dir;
    struct dirent* dent;
    char* filename;
    int off = 0;
    int tmp;
    bzero(buff,length);

    dir = opendir(user_env->user_path);
    /*dir = opendir("./share");*/
    
    if(dir == NULL)
    { 
       return -1; 
    } 

    while((dent= readdir(dir)) != NULL)
    {
        filename = dent->d_name;
        if(filename[0] == '.')
            continue;
        /*off += get_file_info(filename,buff +off,length - off); */
        if(tmp= get_file_info(filename,buff+off,length-off),tmp < 0)
        {
#ifdef FTP_DEBUG
            printf("get file infomation error\n");
#endif
            return -1;
        }
        off += tmp;
    }
    
#ifdef FTP_DEBUG
    printf("the result of ls:\n%s",buff);
#endif
    return off;
}


