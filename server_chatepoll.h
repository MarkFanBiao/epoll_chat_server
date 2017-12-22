#include "./head.h"

typedef struct User
{
    struct User *next;
    char name[32];
    int sock;
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
    user->next = NULL;
    user->name[0] = 0;  //先不取名字
    user->sock = sock;
    //头插
    addUser(user);
}

//通过sock找用户
User *findUserBySock(int sock)
{
    User *user = userhead;
    while(user)
    {
        if(user->sock == sock)
            return user;    //找到了，返回user
        user = user->next;
    }
    return NULL;
}

//通过 name 找用户  //主要是为了防止重名用
User *findUserByName(const char *name)
{
    User *user = userhead;
    while(user)
    {
        if(strcmp(user->name, name) == 0)
            return user;
        user = user->next;
    }
    return NULL;
}

void delUser(int sock)
{
    User *del = userhead;
    if(del->sock == sock)   //首元素就是要删除的结点
    {
        userhead = userhead->next;
        free(del);
        return;
    }
    //首元素不是要删除的，从第二个元素开始遍历链表
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

//回复 提示信息
void response(int sock, const char *info)
{   //ack|set name ok
    char buf[8192];
    sprintf(buf, "ack|%s", info);
    send_data(sock, buf, strlen(buf));
}

void set_name(int sock, const char *name)
{   //先通过名字，在用户链表中找
    User *user = findUserByName(name);
    if(user)    //重名
    {
        response(sock, "duplicated name");
        return;
    }
    //没有重名的，再通过sock找到new_user创建的用户
    user = findUserBySock(sock);
    strcpy(user->name, name);   //将名字赋给用户
    printf("someone change name %s\n", name);
    response(sock, "set name ok!");
}

void transMsg(int toSock, const char *fromName, const char *msg)
{
    char buf[8192];
    sprintf(buf, "msg|%s|%s", fromName, msg);
    send_data(toSock, buf, strlen(buf));
}

void send_msg(int sock, const char *toName, char *msg)
{   //通过toName查找 要发送名字的用户是否在线
    User *toUser = findUserByName(toName);  //找到目的用户
    if(toUser == NULL)
    {   //若该用户名不在用户链表中
        response(sock, "user offline");
        return;
    }
    //通过不会重复的名字找到了该用户 在线,即toUser存在于用户链表，
    //则server肯定有和toUser客户端通信的sock保存在toUser结点中
    
    //找到了要发送者，转发消息，发完给源发送者回个信息
    User *user = findUserBySock(sock);  //这个sock本来就是源发送者的sock值传递进来的
    
    transMsg(toUser->sock, user->name, msg);   //转发消息   //user->name说: msg
    response(sock, "massage transferred");
}
