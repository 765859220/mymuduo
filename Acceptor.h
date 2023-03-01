#pragma once
#include "noncopyable.h"
#include "Socket.h"
#include "Channel.h"

#include <functional>

class EventLoop;
class InetAddress;

class Acceptor : noncopyable
{
public:
    using NewConnectionCallback = std::function<void(int sockfd, const InetAddress&)>;
    Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport);
    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback &cb) 
    {
        newConnectionCallback_ = cb;
    }

    bool listenning() const { return listenning_; }
    void listen();
private:
    void handleRead();
    
    EventLoop *loop_;//Acceptor用的就是用户定义的那个baseLoop，也称作mainLoop
    Socket acceptSocket_; // listenfd
    Channel acceptChannel_;  // listenfd也需要poller监听，poller操作的单位是channel，把listenfd打包成channel
    NewConnectionCallback newConnectionCallback_; // 把accpet返回的新clientfd分发给subloop
    bool listenning_;
};
