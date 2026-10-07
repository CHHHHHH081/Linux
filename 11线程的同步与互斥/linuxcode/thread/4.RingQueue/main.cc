#include "RingQueue.hpp"
#include "TaskManager.hpp"
#include <cstdlib>

TaskManager tm;

task_t RandTask()
{
    return tm.GetTask(rand() % tm.Tasknum());
}

RingQueue<task_t> rq;

void *producer(void *arg)
{
    int cnt = 0;
    while (1)
    {
        rq.EnQueue(RandTask());
        // sleep(1);
    }
}

void *consumer(void *arg)
{
    while (1)
    {
        task_t task;
        rq.Pop(&task);
        task();
        sleep(1);
    }
}

int main()
{
    srand(0);
    pthread_t p[2], c[3];
    pthread_create(p, nullptr, producer, (void *)"producer1");
    pthread_create(p + 1, nullptr, producer, (void *)"producer2");
    pthread_create(c, nullptr, consumer, (void *)"consumer1");
    pthread_create(c + 1, nullptr, consumer, (void *)"consumer2");
    pthread_create(c + 2, nullptr, consumer, (void *)"consumer3");

    pthread_join(*(p), nullptr);
    pthread_join(*(p + 1), nullptr);
    pthread_join(*(c), nullptr);
    pthread_join(*(c + 1), nullptr);
    pthread_join(*(c + 2), nullptr);
    return 0;
}
