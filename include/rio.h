#ifndef RIO_H_
#define RIO_H_

/*
 * Using __builtin_constant_p(x) to ignore cases where the return
 * value is always the same.  This idea is taken from a similar patch
 * written by Daniel Walker.
 */
# ifndef likely
#  define likely(x)	(__builtin_constant_p(x) ? !!(x) : __branch_check__(x, 1))
# endif
# ifndef unlikely
#  define unlikely(x)	(__builtin_constant_p(x) ? !!(x) : __branch_check__(x, 0))
# endif

size_t io_readn(int fd, void *usrbuf, size_t n);
size_t io_writen(int fd, void *usrbuf, size_t n);

#endif // RIO_H_
