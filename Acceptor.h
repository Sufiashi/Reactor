#pragma once
#include <functional>
#include "Socket.h"
#include "InetAddress.h"
#include "Channel.h"
#include "EventLoop.h"

class Acceptor
{
private:
    EventLoop *loop_;           //Acceptor对应一个事件循环，在构造函数中传入，只能使用，但不属于这个类
    Socket *servsock_;          //服务端用于监听的socket，在构造函数用创建
    Channel * acceptchannel_;   //Acceptor对应的channel，在构造函数中创建
public:
    Acceptor(EventLoop *loop,const std::string& ip,uint16_t port);
    ~Acceptor();
};