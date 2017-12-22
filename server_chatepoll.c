#include "./server_chatepoll.h"

int read_data(int sock)
{   //9999name|wang  或者  9999msg|li|nihao
    char buflen[5];
    if(doRead(sock, buflen, 4) < 0) //该socket出错了
    {       
        delUser(sock);
        return -1;
    }
    buflen[4] = 0;
    int len = atoi(buflen);

    char buf[8192];
    doRead(sock, buf, len);
    buf[len] = 0;
    //buf中是 name|wang  或者 msg|li|nihao
    char *cmd = strtok(buf, "|");
    if(strcmp(cmd, "name") == 0)
    {   //需要获取名字，并未用户起名字
        char *name = strtok(NULL, "\0");
        set_name(sock, name);
    }
    else if(strcmp(cmd, "msg") == 0)
    {
        char *toName = strtok(NULL, "|");
        char *msg = strtok(NULL, "\0");
        send_msg(sock, toName, msg);
    }
}

int main(int argc, char *argv[])
{
    int server = startTcpServer(9988, "0.0.0.0", 10);
    int epollfd = epoll_create(1024);
    addEpoll(epollfd, server, EPOLLIN);

    struct epoll_event ev[8];
    while(1)
    {
        int count = epoll_wait(epollfd, ev, 8, 5000);
        if(count < 0)
        {
            printf("errno is %d\n", errno);
            return -1;
        }
        if(count == 0)  //超时
            continue;
        //count > 0
        int i;
        for(i = 0; i < count; ++i)
        {
            int fd = ev[i].data.fd;
            if(fd == server)    //被读的是server
            {
                int sock = accept(fd, NULL, NULL);
                if(sock == -1)  //接收出了问题
                    break;
                //接收成功后，创建用户，将新句柄加入到epoll中
                new_user(sock);
                printf("someone has connected\n");
                addEpoll(epollfd, sock, EPOLLIN);
            }
            else    //fd == sock, 被读的是通信的sock,读取数据
            {
                if(read_data(fd) < 0)
                {
                    close(fd);  //close会自动将出问题的sock从epoll中取出
                }
            }
        }

    }
    
	return 0;
}
