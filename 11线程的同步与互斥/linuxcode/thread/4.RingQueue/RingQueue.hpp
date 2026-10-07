#pragma once
#include "Sem.hpp"
#include "Mutex.hpp"
#include <iostream>
#include <vector>

using namespace SemModule;
using namespace Mutexmodule;
using namespace std;

unsigned int defaultcap = 5;

template <typename T>
class RingQueue
{
public:
    RingQueue(unsigned int cap = defaultcap)
        : _blank_sem(cap),
          _data_sem(0),
          _c_step(0),
          _p_step(0),
          _cap(cap)
    {
        _rq.reserve(cap);
    }

    // 左值
    void EnQueue(const T &in)
    {
        // 生产者将任务入队
        _blank_sem.P();
        {
            LockGuard ld(_p_mutex);
            // std::cout << "生产者生产了一个任务 " << in << std::endl;
            std::cout << "生产者生产了一个任务" << std::endl;
            _rq[_p_step] = in;
            _p_step++;
            _p_step %= _cap;
            _data_sem.V();
        }
    }

    // 需要返回值
    void Pop(T *out)
    {
        // 消费者将任务出队
        _data_sem.P();
        {
            LockGuard ld(_c_mutex);
            std::cout << "消费者消费一个任务: " << std::endl;
            *out = _rq[_c_step];
            _c_step++;
            _c_step %= _cap;
            _blank_sem.V();
        }
    }

    // 不需要返回值
    void Pop()
    {
        // 消费者将任务出队
        _data_sem.P();
        {
            LockGuard ld(_c_mutex);
            std::cout << "消费者消费一个任务: " << std::endl;
            _c_step++;
            _c_step %= _cap;
            _blank_sem.V();
        }
    }

private:
    vector<T> _rq;
    Sem _blank_sem;
    Sem _data_sem;
    int _c_step;
    int _p_step;
    int _cap;
    Mutex _c_mutex;
    Mutex _p_mutex;
};
