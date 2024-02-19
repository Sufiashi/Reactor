#pragma once
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "InetAddress.h"
//创建非阻塞的socket
int createnonblocking();



//socket类
class Socket{
private:
    const int fd_;      //Socket持有的fd，在构造函数中传进来
public:
    Socket(int fd);     //构造函数
    ~Socket();          //析构函数

    int fd() const;     //返回fd_成员的函数

    void setreuseaddr(bool on);     //设置SO_REUSEADDR选项，true打开，false关闭
    void setreuseport(bool on);     //设置SO_REUSEPORT选项
    void settcpnodelay(bool on);    //设置TCP_NODELAY
    void setkeepalive(bool on);     //设置SO_KEEPALIVE选项

    void bind(const InetAddress& servaddr);
    void listen(int nn=128);            
    int accept(InetAddress& clientaddr);         

};