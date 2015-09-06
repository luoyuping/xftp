#include "xftp.h"

int data_socket_init()
{ 
     
    int listen_fd;
    /*int accept_fd;*/
    struct sockaddr_in server_addr;
    /*struct sockaddr_in client_addr;*/

    if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        xftp_print_info(LOG_ERR, "Create Socket Error");
        return -1;
    }

    int optval = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (void *)&optval, sizeof(int)) < 0) {
        xftp_print_info(LOG_ERR, "Set Socket SO_REUSEADDR Error!");
        return -1;
    }

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(0); // 让系统绑定任何能用的端口
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        xftp_print_info(LOG_ERR, "Bind Scoket Error!");
        return -1;
    }

    if (listen(listen_fd, LISTEN_SIZE) < 0) {
        xftp_print_info(LOG_ERR, "Listen Socket Error!");
        return -1;
    }
    return listen_fd;
} 

