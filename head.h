#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/epoll.h>
#include <dirent.h>
#include <errno.h>

//doRead    一定读完整的len 字节数再返回
static inline int doRead(int sockfd, void *buf, int len)    //同read函数模型
{
    int already_read = 0;   //已经读到的字节数
    while(already_read != len) 
    {
        int ret = read(sockfd, buf+already_read, len-already_read);
        if(ret > 0)
        {
            already_read += ret;
        }
        else    //出错
        {
            return -1;
        }
    }
    return 0; 
}

static inline int doWrite(int sockfd, const void *buf, int len)
{
    int already_write = 0;
    while(already_write != len)
    {
        int ret = write(sockfd, buf+already_write, len-already_write);
        if(ret > 0)
        {
            already_write += ret;
        }
        else
        {
            return -1;
        }
    }
    return 0;
}

static inline int connectServer(unsigned short port, const char *ip)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);

    int ret = connect(sockfd, (struct sockaddr *)&addr, sizeof(addr));
    if(ret == 0)
        return sockfd;
    close(sockfd);
    return -1;
}

static inline int startTcpServer(unsigned short port, const char * ip, int backlog)
{
    int server = socket(AF_INET, SOCK_STREAM, 0);
    //创建 结构体并赋值
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);
    //绑定
    int ret = bind(server, (struct sockaddr *)&addr, sizeof(addr));
    if(ret < 0)
    {
        close(server);
        exit(1);
    }
    //监听
    listen(server, backlog);
    return server;
}

static inline int getFilelen(const char *filename)
{
    struct stat stat_buf;
    int ret = stat(filename, &stat_buf);
    if(ret < 0)
    {
        return -1;
    }

    return stat_buf.st_size;
}

static inline void set_nonblock(int fd)
{
    int flag = fcntl(fd, F_GETFL);
    flag |= O_NONBLOCK;
    fcntl(fd, F_SETFL, flag);   //先获取，再或赋值，再设置，不然会把所有属性都覆盖成nonblock;
}

static inline void addEpoll(int epollfd, int fd, int events)
{
    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;

    if(epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev) < 0)
    {
        epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev);
    }
}

