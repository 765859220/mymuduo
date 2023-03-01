#pragma once

#include "noncopyable.h"
#include "Timestamp.h"

#include <functional>
#include <memory>


// 头文件中只给出类型的前置声明，在源文件中再include "EventLoop.h"
// 但是前置声明只能用于指针和引用（因为指针不需要知道变量具体的类型），普通类型还是要include
class EventLoop; 

/**
 * Channel 理解为通道，封装了sockfd和其感兴趣的event，如EPOLLIN、EPOLLOUT事件
 * 还绑定了poller返回的具体事件
 */ 
 
class Channel : noncopyable
{
public:
    using EventCallback = std::function<void()>;//事件回调 
    using ReadEventCallback = std::function<void(Timestamp)>;//只读事件的回调 

    Channel(EventLoop *loop, int fd);//构造函数 
    ~Channel();//析构函数 

    //fd得到poller通知以后，处理事件的
    //调用相应的回调方法来处理事件 
    void handleEvent(Timestamp receiveTime);  

    //设置回调函数对象,实参执行完就不用了，所以直接用move转移给成员变量
    void setReadCallback(ReadEventCallback cb) { readCallback_ = std::move(cb); }
    void setWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); }
    void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); }
    void setErrorCallback(EventCallback cb) { errorCallback_ = std::move(cb); }

    //防止当channel被手动remove掉，channel还在执行回调操作，就是上面这些回调操作 
    void tie(const std::shared_ptr<void>&);

    int fd() const { return fd_; }
    int events() const { return events_; }//fd所感兴趣的事件 
    // set_revents不是给channel自己用的，poller监听到事件，设置了channel的fd相应事件
    void set_revents(int revt) { revents_ = revt; } 

    //设置fd相应的事件状态，要让fd对这个事件感兴趣 
    //update就是调用epoll_ctl，向poller（epoll）添加该fd感兴趣的事件
    void enableReading() { events_ |= kReadEvent; update(); }//赋上去 用或  
    void disableReading() { events_ &= ~kReadEvent; update(); }//取反再与，去掉 
    void enableWriting() { events_ |= kWriteEvent; update(); }
    void disableWriting() { events_ &= ~kWriteEvent; update(); }
    void disableAll() { events_ = kNoneEvent; update(); }

    //返回fd当前的事件状态
    bool isNoneEvent() const { return events_ == kNoneEvent; }
    bool isWriting() const { return events_ & kWriteEvent; }
    bool isReading() const { return events_ & kReadEvent; }

    int index() { return index_; }
    void set_index(int idx) { index_ = idx; }

    //one loop per thread
    EventLoop* ownerLoop() { return loop_; }// 返回当前channel属于的eventloop 
    void remove();// 删除channel 
private:

    void update();//更新，内部对象调用 
    void handleEventWithGuard(Timestamp receiveTime);//受保护的处理事件 
    
    //表示当前fd和其状态，是没有对任何事件感兴趣，还是对读或者写感兴趣 
    static const int kNoneEvent;//都不感兴趣 
    static const int kReadEvent;//读事件 
    static const int kWriteEvent;//写事件 

    EventLoop *loop_;//事件循环
    const int fd_;//fd, Poller监听的对象
    int events_;//注册fd感兴趣的事件
    int revents_;//poller返回的具体发生的事件
    int index_;

    std::weak_ptr<void> tie_;//绑定自己 
    bool tied_; // 防止remove channle之后还在使用

    //因为channel通道里面能够获知fd最终发生的具体的事件revents，所以它负责调用具体事件的回调操作
    //这些回调是用户设定的，通过接口传给channel来负责调用 ，channel才知道fd上是什么事件 
    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
};
