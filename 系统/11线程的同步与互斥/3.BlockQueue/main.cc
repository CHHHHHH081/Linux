#include "BlockQueue.hpp"

BlockQueue<int> bq(5);

void *producer(void *arg)
{
    int cnt = 0;
    while (1)
    {
        bq.EnQueue(cnt);
        cnt++;
        // sleep(1);
    }
}

void *consumer(void *arg)
{
    while (1)
    {
        bq.Pop();
        sleep(1);
    }
}

int main()
{
    pthread_t p[20], c[30];
    for (int i = 0; i < 20; i++)
    {
        pthread_create(p + i, nullptr, producer, (void *)"producer");
    }
    for (int i = 0; i < 30; i++)
    {
        pthread_create(c + i, nullptr, consumer, (void *)"consumer");
    }
    for (int i = 0; i < 20; i++)
    {
        pthread_join(*(p + i), nullptr);
    }
    for (int i = 0; i < 30; i++)
    {
        pthread_join(*(c + i), nullptr);
    }
    return 0;
}
