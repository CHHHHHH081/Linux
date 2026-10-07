#pragma once

#include <queue>
#include "Thread.hpp"
#include "Cond.hpp"
#include "log.hpp"

namespace ThreadPoolModule
{
    using namespace ThreadModule;
    using namespace LogModule;
    using namespace Mutexmodule;
    using namespace CondModule;
    const int gthreadnum = 5;
    template <typename T>
    class threadpool
    {
        using func_t = std::function<void()>;

    private:
        void WakeUpOne()
        {
            _cond.Signal();
            LOG(Loglevel::NORMAL) << "唤醒了一个线程";
        }
        void WakeUpAll()
        {
            _cond.Broadcast();
            LOG(Loglevel::NORMAL) << "唤醒了所有线程";
        }

    public:
        threadpool(int thread_num = gthreadnum)
            : _thread_num(thread_num), _sleeper_num(0), _isrunning(false)
        {
            for (int i = 0; i < _thread_num; i++)
            {
                _threads.emplace_back(
                    [this]()
                    { HandlerTask(); });
            }
        }

        static threadpool *GetInstance()
        {
            if (inc == nullptr)
            {
                LockGuard lg(_lock);
                LOG(Loglevel::NORMAL) << "获取单例...";
                if (inc == nullptr)
                {
                    LOG(Loglevel::NORMAL) << "首次使用单例, 创建单例...";
                    inc = new threadpool<T>();
                    inc->Start();
                }
            }
            return inc;
        }

        void HandlerTask()
        {
            char name[128];
            pthread_getname_np(pthread_self(), name, sizeof(name));
            while (1)
            {
                T t;
                {
                    LockGuard lg(_mutex);
                    while (_taskq.empty() && _isrunning)
                    {
                        _sleeper_num++;
                        _cond.Wait(_mutex);
                        _sleeper_num--;
                    }
                    if (!_isrunning && _taskq.empty())
                    {
                        // 该停止了
                        LOG(Loglevel::NORMAL) << "线程 " << name << " 已退出";
                        break;
                    }
                    // 队列中有任务
                    t = _taskq.front();
                    _taskq.pop();
                }
                t();
            }
        }

        void Start()
        {
            if (_isrunning)
                return;
            _isrunning = true;
            for (thread &thread : _threads)
            {
                thread.Start();
                LOG(Loglevel::NORMAL) << "线程 " << thread.name() << " 已启动";
            }
        }

        bool Enqueue(const T &in)
        {
            LockGuard lg(_mutex);
            if (!_isrunning)
            {
                LOG(Loglevel::NORMAL) << "线程池已停止,无法新增任务";
                return false;
            }
            _taskq.push(in);
            WakeUpOne();
            return true;
        }

        void Stop()
        {
            if (!_isrunning)
                return;
            _isrunning = false;
            WakeUpAll();
        }

        void Join()
        {
            for (thread &thread : _threads)
            {
                thread.Join();
            }
        }

    private:
        std::vector<thread> _threads;
        std::queue<T> _taskq;
        bool _isrunning;
        int _thread_num;
        int _sleeper_num;
        Mutex _mutex;
        Cond _cond;

        static threadpool<T> *inc;
        static Mutex _lock;
    };

    template <typename T>
    threadpool<T> *threadpool<T>::inc = nullptr;

    template <typename T>
    Mutex threadpool<T>::_lock;
}
