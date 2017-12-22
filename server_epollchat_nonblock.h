#include "./head.h"

typedef struct User
{
    struct User *next;
    char name[32];
    int sock;

    char buf[8192];
    int already_read_len;
    int packet_len;
}User;

User *userhead = NULL;

void addUser(User *user)
{
    user->next = userhead;
    userhead = user;
}

void new_user(int sock)
{
    User *user = malloc(sizeof(User));
    user->name[0] = 0;
    user->sock = sock;
    user->next = NULL;
    user->already_read_len = 0;

    //头插
    addUser(user);
}

User *findUser(const char *name)
{
    User *user = userhead;
    while(user)
    {
        if(strcmp(user->name, name) == 0)   //找到了
            return user;    
        user = user->next;
    }
    return NULL;
}

User *findUserBySock(int sock)
{
    User *user = userhead;
    while(user)
    {
        if(user->sock == sock)
            return user;
        user = user->next;
    }
    return NULL;
}

void delUser(int sock)
{
    User *del = userhead;
    if(del->sock == sock)
    {   //头结点就是要删除的结点
        userhead = userhead->next;
        free(del);
        return;
    }
    while(del->next)
    {
        if(del->next->sock == sock)
        {
            User *tmp = del->next;
            del->next = del->next->next;
            free(tmp);
            return;
        }
        del = del->next;
    }
}

void send_data(int sock, const char *data, int len)
{
    char buflen[5];
    sprintf(buflen, "%04d", len);
    doWrite(sock, buflen, 4);

    doWrite(sock, data, len);
}

void response(int sock, const char *info)
{   //ack|content
    char buf[8192];
    sprintf(buf, "ack|%s", info);
    send_data(sock, buf, strlen(buf));    
}

void set_name(int sock, const char *name)

{
    User *user = findUser(name);    //通过名字在链表找 用户
    if(user)     //不为空，找到一个
    {   //重名了
        response(sock, "duplicate name");       //给原来sock回复一个 信息
        return;
    }

    //没有重名，再通过sock找到新建的用户，把名字写入
    user = findUserBySock(sock);
    strcpy(user->name, name);   //把用户名写入
    printf("someone changed name %s\n", name);
    response(sock, "set name ok");
}

void transMsg(int toSock, const char *fromName, const char *content)
{
    char buf[8192];
    sprintf(buf, "msg|%s|%s", fromName, content);
    send_data(toSock, buf, strlen(buf));
}

void send_msg(int sock, const char *toName, const char *content)
{
    User *toUser = findUser(toName);     //找到目的用户
    if(toUser == NULL)
    {
        response(sock, "user offline");
        return;
    }
    //找到了要发送的 名字，再找 发送者
    User *user = findUserBySock(sock);   //这个是 通过sock找到 源的用户

    transMsg(toUser->sock, user->name, content);
    response(sock, "msg transferred");   
}

