#include "xftp.h"

size_t rio_readn(int fd, void *usrbuf, size_t n)  // 循环读取数据到usrbuf 中， usrbuf的长度为 n
{
	size_t nleft = n;
	size_t nread = 0;
	char   *bufp = usrbuf;

	while (nleft > 0) {
		if ((nread = read(fd, bufp, nleft)) == -1) {
			if (errno == EINTR) { 	// Interrupted by sig handler return
				nread = 0; 	// and call read() again
			} else {
				return -1; 	// errno set by read()
			}
		} else if (nread == 0) {
			break; 		// EOF
		}
		nleft -= nread;
		bufp += nread;
	}

	return (n - nleft); 	// return >= 0
}

size_t rio_writen(int fd, void *usrbuf, size_t n) // 循环写取数据到usrbuf 中， usrbuf的长度为 n
{
	size_t nleft = n;
	size_t nwritten = 0;
	char   *bufp = usrbuf;
	
	while (nleft > 0) {
		if ((nwritten = write(fd, bufp, nleft)) <= 0) {
			if (errno == EINTR) { 	// Interrupted by sig handler return
				nwritten = 0; 	// and call write() again
			} else {
				return -1; 	// errno set by write()
			}
		}
		nleft -= nwritten;
		bufp += nwritten;
	}
	return n;
}

