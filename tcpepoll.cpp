/*
*程序名：tcpepoll.cpp,此程序用于演示采用epoll模型实现网络通信的服务端
*作者：Sufia
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/fcntl.h>
#include <netinet/tcp.h>    //TCP_NODELAY需要包含这个头文件
#include "InetAddress.h"
#include "Socket.h"
#include <iostream>
#include "Epoll.h"
using namespace std;

/*
//设置非阻塞的IO
void setnonblocking(int fd)
{
    fcntl(fd,F_SETFL,fcntl(fd,F_SETFL)|O_NONBLOCK);
}
*/
int main(int argc,char* argv[]){
    if(argc!=3){
        printf("usage:./tcpepoll ip port\n");
        printf("example:./tcpepoll 192.168.93.5 5005\n\n");
        return -1;
    }


    //创建服务端用于监听的listenfd
    class Socket servsock(createnonblocking());
    InetAddress servaddr(argv[1],atoi(argv[2]));  //服务端的地址和协议

    servsock.setreuseaddr(true);
    servsock.setreuseport(true);
    servsock.settcpnodelay(true);
    servsock.setkeepalive(true);

    servsock.bind(servaddr);
    servsock.listen();
    

    Epoll ep;
    //ep.addfd(servsock.fd(),EPOLLIN);        //读事件，水平触发
    Channel *servchannel=new Channel(&ep,servsock.fd(),true);
    servchannel->enablereading();

    
    while(true){        //事件循环
        std::vector<Channel*>channels;            //存放epollwait返回的事件
        channels=ep.loop(-1);      //等待监视的fd有事件发生

        //如果infds>0,表示有事件发生的fd的数量
        for(auto &ch:channels) //遍历epoll返回的数组evs
        {
            ch->handleevent(&servsock);
        }
    }

    return 0;
}
