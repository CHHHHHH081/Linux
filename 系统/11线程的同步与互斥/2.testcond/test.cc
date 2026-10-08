#include <iostream>
#include <pthread.h>
#include <vector>
#include <unistd.h>

int n = 0;
pthread_mutex_t _mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t _cond = PTHREAD_COND_INITIALIZER;

void *RunThread(void *args)
{
    char *name = static_cast<char *>(args);
    while (true)
    {
        {
            pthread_mutex_lock(&_mutex);
            pthread_cond_wait(&_cond, &_mutex);
            n++;
            std::cout << "线程" << name << "计算: " << n << std::endl;
            pthread_mutex_unlock(&_mutex);
        }
    }
}

int main()
{
    int cnt = 5;
    std::vector<pthread_t> pvr;
    while (cnt--)
    {
        pthread_t tid;
        char *name = new char[64];
        int n = snprintf(name, 64, "thread-%d", cnt);
        if (n < 0)
            perror("snprintf error");
        pthread_create(&tid, nullptr, RunThread, name);
        pvr.push_back(tid);
    }

    while (true)
    {
        std::cout << "唤醒一个线程" << std::endl;
        sleep(1);
        pthread_cond_signal(&_cond);
    }

    //先不管回收
    for (auto e : pvr)
    {
        pthread_join(e, nullptr);
    }
    return 0;
}
