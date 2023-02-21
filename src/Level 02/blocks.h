#include <stdio.h>      //printf(), fprintf(), stderr, stdout, stdin
#include <fcntl.h>      //O_WRONLY, O_CREAT, O_TRUNC
#include <sys/stat.h>   //S_IRUSR, S_IWUSR
#include <stdlib.h>     //exit(), EXIT_SUCCESS, EXIT_FAILURE, atoi()
#include <unistd.h>     // SEEK_SET, read(), write(), open(), close(), lseek()
#include <errno.h>      //errno
#include <string.h>     // strerror()

#define BLOCKSIZE 1024
#define RW_PERMS 0666

// Error management
#define SUCCESS 0
#define FAILURE -1

int bmount(const char *path);
int bumount();
int bwrite(unsigned int block, const void *buf);
int bread(unsigned int n_block, void *buf);