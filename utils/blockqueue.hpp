#pragma once
#include <mutex>
#include <queue>
#include <memory>
#include <thread>
#include <assert.h>
#include <condition_variable>
#include "noncopyable.h"
namespace utils
{
    template<typename T>
    class BlockingQueue : noncopyable
    {
    public:
        using queue_type = std::queue<T>;

        BlockingQueue()
        {
        }

        ~BlockingQueue()
        {
            
        }

        void push(const T& x)
        {
			std::lock_guard<std::mutex> lock(m_mtx);
			m_queue.push(x);
			m_Condit.notify_one();
        }

        void push(T&& x)
        {
            std::lock_guard<std::mutex> lock(m_mtx);
            m_queue.push(std::move(x));
            m_Condit.notify_one();
        }

        T wait_and_pop()
        {
            std::unique_lock<std::mutex> lock(m_mtx);
            /*
            wait()是条件变量的成员函数，用来等一个东西，如果第二个参数lambda表达式返回值是false，那么wait将解锁第一个参数（互斥量），并堵塞到本行。
            堵塞到什么时候呢？堵塞到其他某个线程调用notify_one（）成员函数为止。如果返回true，那么wait()直接返回。
            如果没有第二个参数，就跟默认第二个参数返回false效果一样。

            my_cond.notify_one();//尝试吧wait的线程唤醒，执行完这行，wait就被唤醒了
            只能通知一个线程，如果两个线程干的不同的事情，都卡在wait，怎么唤醒多个线程？notify_all，这两个线程只有一个线程能获取到锁，另一个没拿到锁，就一直不断的去获得锁

            当其他notify_one()将wait唤醒之后，wait不断尝试获取互斥量锁，如果获取不到，流程就卡在wait这里等着获取。如果获取到了，wait就获取到锁（就等于上锁）；
            （1）如果wait有第二个参数（lambda）,就判断这个表达式，如果lambda为false，那么流程如上，又休眠，再等待唤醒；
            （2）如果表达式为true，那么wait返回，流程走下来（此时互斥量锁还是锁着的），流程只要能走到这里来，这个互斥量锁一定是锁着的。
            （3）如果没有第二个参数，wait就返回，流程走下来
            */

            while(m_queue.empty())
                m_Condit.wait(lock);
            /*
            For performance reasons, the POSIX API allows the OS to wake up your thread even if the condition has not been fulfilled (that's called a spurious wakeup).
            */
            assert(!m_queue.empty());
            T front(std::move(m_queue.front()));
            m_queue.pop();
            return front;
        }

        queue_type drain()
        {
            queue_type queue;
            {
                std::lock_guard<std::mutex> lock(m_mtx);
                queue = std::move(m_queue);
                assert(m_queue.empty());
            }
            return queue;
        }

        size_t size() const
        {
            std::lock_guard lock(m_mtx);
            return m_queue.size();
        }

    private:
        std::condition_variable m_Condit;
        mutable std::mutex m_mtx;
        queue_type m_queue;
    };
}