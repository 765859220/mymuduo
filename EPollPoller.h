#pragma once

#include "Poller.h"
#include "Timestamp.h"

#include <vector>
#include <sys/epoll.h>

class Channel;

/**
 * epoll的使用                 对应的EPollPoller
 * epoll_create  ——      构造函数，并在析构中关闭epollfd
 * epoll_ctl     ——      updateChannel+removeChannel     
 * epoll_wait    ——               poll
 */ 
 
class EPollPoller : public Poller
{
public:
    EPollPoller(EventLoop *loop);
    ~EPollPoller() override;

    //重写基类Poller的抽象方法
    Timestamp poll(int timeoutMs, ChannelList *activeChannels) override;
    void updateChannel(Channel *channel) override;
    void removeChannel(Channel *channel) override;
    //上面两个方法表示epoll_ctrl的行为

private:
    static const int kInitEventListSize = 16;// 初始化vector长度 

    //填写活跃的连接
    void fillActiveChannels(int numEvents, ChannelList *activeChannels) const;
    //更新channel通道，调用epoll_ctl;
    void update(int operation, Channel *channel);
    // fd及其检测事件的可扩容数组
    using EventList = std::vector<epoll_event>;
    
    // 内核中的epoll实例，由epoll_create创建
    int epollfd_;
    EventList events_;//epoll_wait的第二个参数，发生事件的fd
};
