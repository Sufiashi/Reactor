#pragma once
#include <sys/epoll.h>
#include "Epoll.h"
#include "InetAddress.h"
#include "Socket.h"

class Epoll;

class Channel{
private:
    int fd_=-1;                 //channel与fd一一对应
    Epoll *ep_=nullptr;         //channel对Epoll是多对一的关系，一个channnel只有一个Epoll
    bool inepoll_=false;        //channel是否已经添加到epoll树上，若已添加，调用epoll_ctl时用EPOLL)CTL_ADD,否则用EPOLL_CTL_MOD
    uint32_t events_=0;         //fd需要监视的事件，listenfd和clientfd需要监视EPOLLIN，clientfd需要监视EPOLLOUT
    uint32_t revents_=0;        //fd已发生事件
    bool islisten_=false;        //如果是listenfd取值为true，客户端的为false
public:
    Channel(Epoll *ep,int fd,bool islisten);  //构造函数
    ~Channel();                 //析构函数

    int fd();                   //返回fd成员
    void useet();               //采用边缘触发

    void enablereading();               //让epoll_wait监视fd的读事件
    void setinepoll();                  //把inepoll成员变量设置为true
    void setrevents(uint32_t ev);       //设置revents成员的值为参数ev
    bool inpoll();                      //返回inpoll成员的值
    uint32_t events();                  //返回events成员
    uint32_t revents();                 //返回revents成员
    void handleevent(Socket *servsock);                 //处理返回事件
};