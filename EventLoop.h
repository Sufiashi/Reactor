#pragma once
#include "Epoll.h"

class Channel;
class Epoll;

//事件循环类
class EventLoop
{
private:
    Epoll *ep_;

public:
    EventLoop();            //构造函数
    ~EventLoop();           //析构函数

    void run();             //运行事件循环
    Epoll *ep();            //返回epoll对象的地址

    void updatechannel(Channel *ch);                        // 把channel添加/更新到红黑树上，channel中有fd，也有需要监视的事件。
};