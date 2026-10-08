#ifndef __BLOCKQUEUE_HPP
#define __BLOCKQUEUE_HPP

#include <iostream>
#include <unistd.h>
#include <string>
#include <queue>
#include <pthread.h>

template <typename T>
class BlockQueue
{
private:
    bool IsFull() { return _q.size() >= _capcity; }
    bool IsEmpty() { return _q.empty(); }

public:
    BlockQueue(int capcity) : _capcity(capcity), _psleep_cnt(0), _csleep_cnt(0)
    {
        pthread_mutex_init(&_mutex, nullptr);
        pthread_cond_init(&_pcond, nullptr);
        pthread_cond_init(&_ccond, nullptr);
    }

    void EnQueue(T &in)
    {
        // 生产者生产任务，入队
        pthread_mutex_lock(&_mutex);
        while (IsFull())
        {
            // 满了，要阻塞，生产者睡眠
            _psleep_cnt++;
            pthread_cond_wait(&_pcond, &_mutex);
            // 到这里，生产者已经醒了，准备生产
            _psleep_cnt--;
        }
        _q.push(in);
        std::cout << "生产了一个任务：" << _q.back() << std::endl;
        // 看看有没有睡着的消费者，如果有，叫醒它来消费
        if (_csleep_cnt > 0)
            pthread_cond_signal(&_ccond);
        pthread_mutex_unlock(&_mutex);
    }

    void Pop()
    {
        // 消费者消耗任务，出队
        pthread_mutex_lock(&_mutex);
        while (IsEmpty())
        {
            // 空了，要阻塞，消费者睡眠
            _csleep_cnt++;
            pthread_cond_wait(&_ccond, &_mutex);
            _csleep_cnt--;
        }
        _q.pop();
        std::cout << "消费了一个任务：" << _q.back() << std::endl;
        // 看看有没有睡着的生产者，如果有，叫醒它来生产
        if (_psleep_cnt > 0)
            pthread_cond_signal(&_pcond);
        pthread_mutex_unlock(&_mutex);
    }

    T &Front() { return _q.front(); }
    T &Back() { return _q.back(); }

    ~BlockQueue()
    {
        pthread_mutex_destroy(&_mutex);
        pthread_cond_destroy(&_pcond);
        pthread_cond_destroy(&_ccond);
    }

private:
    std::queue<T> _q;
    int _capcity;
    pthread_mutex_t _mutex;
    pthread_cond_t _pcond;
    pthread_cond_t _ccond;
    int _psleep_cnt;
    int _csleep_cnt;
};

#endif
