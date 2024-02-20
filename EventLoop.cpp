#include "EventLoop.h"

//事件循环类
/*
class EventLoop
{
private:
    Epoll ep_;

public:
    EventLoop();            //构造函数
    ~EventLoop();           //析构函数

    void run();             //运行事件循环
}
*/
EventLoop::EventLoop():ep_(new Epoll)
{
}
EventLoop::~EventLoop()
{
    delete ep_;
}

void EventLoop::run()
{
    while(true){        //事件循环
        std::vector<Channel*>channels;            //存放epollwait返回的事件
        channels=ep_->loop(-1);      //等待监视的fd有事件发生

        //如果infds>0,表示有事件发生的fd的数量
        for(auto &ch:channels) //遍历epoll返回的数组evs
        {
            ch->handleevent();
        }
    }
}

Epoll * EventLoop::ep(){
    return ep_;
}
// 把channel添加/更新到红黑树上，channel中有fd，也有需要监视的事件。
void EventLoop::updatechannel(Channel *ch)                        
{
    ep_->updatechannel(ch);
}