#pragma once
#include <functional>
#include "Socket.h"
#include "InetAddress.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Connection.h"
#include <memory>
class Acceptor
{
private:
    EventLoop*loop_;           //没有所有权，只能使用常引用
    Socket servsock_;          //服务端用于监听的socket，在构造函数用创建
    Channel acceptchannel_;   //Acceptor对应的channel，在构造函数中创建
    std::function<void(std::unique_ptr<Socket>)> newconnectioncb_;      //处理客户端连接请求的回调函数，将指向TcpServer::newconnection()
public:
    Acceptor(EventLoop*loop,const std::string& ip,uint16_t port);
    ~Acceptor();
    void newconnection();    // 处理新客户端连接请求。

    void setnewconnectioncb(std::function<void(std::unique_ptr<Socket>)> fn);      //设置回调函数
};