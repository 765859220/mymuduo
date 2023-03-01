#include "Acceptor.h"
#include "Logger.h"
#include "InetAddress.h"

#include <sys/types.h>    
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>


static int createNonblocking()//创建非阻塞的I/O 
{
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (sockfd < 0) 
    {
        LOG_FATAL("%s:%s:%d listen socket create err:%d \n", __FILE__, __FUNCTION__, __LINE__, errno);
    }
    return sockfd;
}

Acceptor::Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport)//构造函数 
    : loop_(loop)
    , acceptSocket_(createNonblocking()) // socket()
    , acceptChannel_(loop, acceptSocket_.fd())
{
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(true);
    acceptSocket_.bindAddress(listenAddr);// bind()
    //TcpServer::start()会调用Acceptor.listen  
    // 如果有新用户的连接，就要执行一个回调（connfd=》打包成channel=》唤醒subloop）
    //baseLoop监听到acceptChannel_(listenfd)有事件发生 => 调用下面注册的handleRead 
    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));//绑定回调 
    // 只关心读事件，TcpConnection需要注册所有事件类型的回调
}

Acceptor::~Acceptor()
{
    acceptChannel_.disableAll();
    acceptChannel_.remove();
}

void Acceptor::listen()
{
    listenning_ = true;
    acceptSocket_.listen();//listen
    acceptChannel_.enableReading();//acceptChannel_ => Poller
}

//listenfd有事件发生了，就是有新用户连接了
void Acceptor::handleRead()
{
    InetAddress peerAddr; // 传出参数，存储客户端的ip、port
    int connfd = acceptSocket_.accept(&peerAddr);
    if (connfd >= 0)
    {
        if (newConnectionCallback_)
        {
            newConnectionCallback_(connfd, peerAddr);//轮询找到subLoop，唤醒，分发当前的新客户端的Channel
        }
        else//客户端没有办法去服务 
        {
            ::close(connfd);
        }
    }
    else
    {
        LOG_ERROR("%s:%s:%d accept err:%d \n", __FILE__, __FUNCTION__, __LINE__, errno);
        if (errno == EMFILE)
        {
            LOG_ERROR("%s:%s:%d sockfd reached limit! \n", __FILE__, __FUNCTION__, __LINE__);
        }
    }
}
