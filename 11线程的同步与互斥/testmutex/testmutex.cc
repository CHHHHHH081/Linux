#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <string>

class ThreadData
{
public:
    ThreadData(const std::string &name, pthread_mutex_t &plock)
        : _plock(&plock), _name(name)
    {
    }
    pthread_mutex_t *_plock;
    std::string _name;
};

int ticket = 100;

void *route(void *arg)
{
    ThreadData *t = static_cast<ThreadData *>(arg);
    while (1)
    {
        pthread_mutex_lock(t->_plock);
        if (ticket > 0)
        {
            usleep(1000);
            printf("%s sells ticket:%d\n", t->_name.c_str(), ticket);
            ticket--;
            pthread_mutex_unlock(t->_plock);
        }
        else
        {
            pthread_mutex_unlock(t->_plock);
            break;
        }
    }
    return nullptr;
}
int main(void)
{
    pthread_t t1, t2, t3, t4;
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);

    ThreadData *td1 = new ThreadData("thread 1", mutex);
    pthread_create(&t1, NULL, route, td1);
    ThreadData *td2 = new ThreadData("thread 2", mutex);
    pthread_create(&t2, NULL, route, td2);
    ThreadData *td3 = new ThreadData("thread 3", mutex);
    pthread_create(&t3, NULL, route, td3);
    ThreadData *td4 = new ThreadData("thread 4", mutex);
    pthread_create(&t4, NULL, route, td4);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);
    pthread_join(t4, NULL);

    pthread_mutex_destroy(&mutex);
    return 0;
}
