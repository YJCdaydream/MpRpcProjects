#pragma once

#include <mutex>    // 互斥锁
#include <condition_variable>   // 进程间的通信
#include <queue>
#include <thread>

template <typename T>
class LockQueue
{
private:
    std::queue<T> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_condvariable;    

public:
    // 多个work进程都会写日志
    void Push(const T &data)    
    {
        // 将要入队，则给队列上锁
        std::lock_guard<std::mutex> lock(m_mutex);

        m_queue.push(data);

        m_condvariable.notify_one();    // 去提醒写进程开始写
        // 添加完后，锁自动释放掉
    }

    // 只有一个进程出
    T Pop()
    {
        // 出队也要上锁
        std::unique_lock<std::mutex> lock(m_mutex);

        // 首先判断队列是否为空
        while(m_queue.empty())
        {
            // 日志队列为空，线程进入wait状态，同时释放掉自己的锁资源
            m_condvariable.wait(lock);
        }

        T data = m_queue.front();
        m_queue.pop();

        return data;
    }
};