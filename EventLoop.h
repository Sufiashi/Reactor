#pragma once
#include <functional>
#include "Epoll.h"
#include <unistd.h>
#include <sys/syscall.h>
#include <memory>
#include <queue>
#include <mutex>
#include <sys/eventfd.h>
#include <sys/timerfd.h>//定时器
#include <map>
#include "Connection.h"
#include <atomic>

class Channel;
class Epoll;
class Connection;
using spConnection=std::shared_ptr<Connection>;
//事件循环类
class EventLoop
{//都是在主线程中创造，在从线程中运行
private:
    int timetvl_;                                               //闹钟时间间隔
    int timeout_;                                               //Connection对象超时时间单位：秒
    std::unique_ptr<Epoll> ep_;
    std::function<void(EventLoop*)> epolltimeoutcallback_;      //epoll_wait超时的回调函数
    pid_t threadid_;           //事件循环所在线程的id
    std::queue<std::function<void()>> taskqueue_;               //事件循环线程被eventfd唤醒后执行的任务队列
    std::mutex mutex_;                                          //任务队列同步的互斥锁
    int wakeupfd_;                                              //用于唤醒事件循环线程的eventfd
    std::unique_ptr<Channel> wakechannel_;                      //eventfd中的channel
    int timerfd_;                                               //定时器的fd
    std::unique_ptr<Channel> timerchannel_;                     //定时器中的channel
    bool mainloop_;                                             //true表示主事件循环，false表示从事件循环
    std::mutex mmutex_;                                         //保护循环的锁
    std::map<int,spConnection> conns_;                          //存放运行该事件循环全部的Conenction对象
    std::function<void(int)> timercallback_;                    //删除TcpServer中超时的Connection对象，将被设置为TcpServer::removeconn()
    std::atomic_bool stop_;                                     //初始为false，如果设置为true，表示停止事件循环
    
public:
    EventLoop(bool mainloop,int timetvl=30,int timeout=80);            //构造函数
    ~EventLoop();           //析构函数

    void run();             //运行事件循环
    void stop();            //停止事件循环
    //std::unique_ptr<Epoll> ep();            //返回epoll对象的地址

    void updatechannel(Channel *ch);                        // 把channel添加/更新到红黑树上，channel中有fd，也有需要监视的事件。
    void removechannel(Channel *ch);                        // 从红黑树上删除channel
    void setepolltimeoutcallback(std::function<void(EventLoop*)>fn);                         //

    bool isinloopthread();                  //判断当前线程是否为事件循环


    void queueinloop(std::function<void()>fn);          // 把任务添加到任务队列中
    void wakeup();                                      // 用eventfd唤醒事件循环线程
    void handlewakeup();                                // 事件循环线程被eventfd唤醒后执行的函数   
    void handletimer();                                 // 闹钟响时执行的函数

    void newconnection(spConnection conn);              //把connection对象保存在conns_中

    void settimercallback(std::function<void(int)>fn);     //设置TcpServer::removeconn()
};