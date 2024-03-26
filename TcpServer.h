#pragma once
#include "EventLoop.h"
#include "Socket.h"
#include "Channel.h"
#include "Acceptor.h"
#include "Connection.h"
#include <map>
#include "ThreadPool.h"
#include <unistd.h>
#include <sys/syscall.h>
#include <memory>
#include <mutex> 
//TCP网络服务类
class TcpServer
{
private:
    std::unique_ptr<EventLoop> mainloop_;                    //主事件循环，使用堆内存和栈内存都可以
    std::vector<std::unique_ptr<EventLoop>> subloops_;       //从事件循环，只能用堆内存
    int threadnum_;                        //线程池大小
    ThreadPool threadpool_;                //线程池
    std::mutex mmutex_;                    //保护conns_的互斥锁
    

    Acceptor acceptor_;    //一个TcpServer只有一个Acceptr对象
    std::map<int,spConnection> conns_;       //一个Tcpserver有多个Connection对象，存放在map容器中
    std::function<void(spConnection)> newconnectioncb_;          // 回调EchoServer::HandleNewConnection()。
    std::function<void(spConnection)> closeconnectioncb_;        // 回调EchoServer::HandleClose()。
    std::function<void(spConnection)> errorconnectioncb_;         // 回调EchoServer::HandleError()。
    std::function<void(spConnection,std::string &message)> onmessagecb_;        // 回调EchoServer::HandleMessage()。
    std::function<void(spConnection)> sendcompletecb_;            // 回调EchoServer::HandleSendComplete()。
    std::function<void(EventLoop*)>  timeoutcb_;                       // 回调EchoServer::HandleTimeOut()。
    //Connection *
public:
    TcpServer(const std::string &ip,const uint16_t port,int threadnum=3);
    ~TcpServer();

    void start();       //运行事件循环
    void stop();        //停止事件循环和IO线程

    //事件处理函数
    void newconnection(std::unique_ptr<Socket> clientsock);    // 处理新客户端连接请求。
    void closeconnection(spConnection conn);     //关闭客户端的连接，在Connection类中回调此函数
    void errorconnection(spConnection conn);     //关闭客户端连接，在Connection类中回调此函数
    void sendcomplete(spConnection conn);        //数据发送完成，在Connection类中调用此函数
    void onmessage(spConnection connn,std::string& message);        //处理客户端的请求报文，在Connection类中回调此函数
    void epolltimeout(EventLoop *loop);         //epoll_wait()超时，在Eventloop类中回调此函数

    //设置回调函数
    void setnewconnectioncb(std::function<void(spConnection)> fn);
    void setcloseconnectioncb(std::function<void(spConnection)> fn);
    void seterrorconnectioncb(std::function<void(spConnection)> fn);
    void setonmessagecb(std::function<void(spConnection,std::string &message)> fn);
    void setsendcompletecb(std::function<void(spConnection)> fn);
    void settimeoutcb(std::function<void(EventLoop*)> fn);

    void removeconn(int fd);                    //删除conns_中的Connection对象，在EventLoop::handletimer()中回调此函数

};