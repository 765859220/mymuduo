#include "Thread.h"
#include "CurrentThread.h"

#include <semaphore.h>

std::atomic_int Thread::numCreated_(0);//初始化

Thread::Thread(ThreadFunc func, const std::string &name)//构造函数
    : started_(false)
    , joined_(false)
    , tid_(0)
    , func_(std::move(func))
    , name_(name)
{
    // 线程11（一共创建了11个线程）
    setDefaultName();
}

Thread::~Thread()//析构函数
{
    if (started_ && !joined_)//线程已经运行起来并且不是工作线程join
    {
        thread_->detach();
        // 子线程在后台独立继续运行，主线程无法再取得子线程的控制权，
        // 即使主线程结束，子线程未执行也不会结束。当主线程结束时，由运行时库负责清理与子线程相关的资源,不会造成孤儿线程
		
    }
}

// 创建一个子线程
void Thread::start()
{
    started_ = true;
    sem_t sem;
    sem_init(&sem, false, 0);

    //开启线程
    thread_ = std::shared_ptr<std::thread>(new std::thread([&](){
        //获取线程的tid值
        tid_ = CurrentThread::tid();
        sem_post(&sem);
        // 子线程的tid能够获取到，说明创建成功，notiy主线程继续执行
        func_();//包含一个eventloop
    }));//智能指针指向线程对象

    //start什么时候返回：必须等待获取上面新创建的线程的tid值，此时主线程可以放心的访问子线程的tid，这才算start成功
    // 开启一个新线程执行得比主线程慢，因此需要线程同步
    sem_wait(&sem);
}

void Thread::join()
{
    joined_ = true;
    thread_->join();
}

void Thread::setDefaultName()//给线程设置默认的名字
{
    int num = ++numCreated_;
    if (name_.empty())//线程还没有名字
    {
        char buf[32] = {0};
        snprintf(buf, sizeof buf, "Thread%d", num);
        name_ = buf;
    }
}
