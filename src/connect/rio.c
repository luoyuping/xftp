#include "xftp.h"

/*
 *返回值的说明
 * 正常返回实际读取到的字符的长度
 *异常返回 -1
 *写时，能避免被信号打断
 */

/*
 * 该操作，自能适用于读取阻塞型的套接字，只做一次的读取操作，但读取发生错误时，也要进行错误处理 ,但本身内部不进行再次接收操作
 * 若发生错误，请再次进行读取数据
 */
ssize_t io_read(int fd, void *usrbuf, size_t bufsize)  // 循环读取数据到usrbuf 中， usrbuf的长度为 bufsize
{
    ssize_t recv_bytes;
    while(1)
    {
        recv_bytes  = recv(fd,usrbuf,bufsize,0);
        // 这里就不进行错误的处理，和标准客户端没有办法协商，标准客户端，不会进行信息的重发
        if(recv_bytes < 0)
        { 
            if(errno == EINTR)
            {
                continue;
            } 
        }
        break;
    } 
    return recv_bytes;
}


/*
 *返回值的说明
 * 正常返回要写入的字符串的长度
 *异常返回 -1
 *写时，能避免被信号打断
 */
size_t io_writen(int fd, void *usrbuf, size_t msg_len) // 循环写取数据到usrbuf 中， usrbuf的长度为 n
{
	size_t nleft = msg_len;
	size_t nwritten = 0;
	char   *bufp = usrbuf;
	
	while (nleft > 0) {
		if ((nwritten = send(fd, bufp, nleft,0)) <= 0) {
			if (errno == EINTR) { 	// Interrupted by sig handler return
				nwritten = 0; 	// and call write() again
			} else {
				return -1; 	// errno set by write()
			}
		}
		nleft -= nwritten;
		bufp += nwritten;
	}
	return msg_len;
}

