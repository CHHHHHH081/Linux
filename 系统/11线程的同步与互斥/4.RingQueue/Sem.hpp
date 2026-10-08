#pragma once
#include <semaphore.h>
namespace SemModule
{
    unsigned int defaultval = 0;
    class Sem
    {
    public:
        Sem(unsigned int sem_value = defaultval)
        {
            sem_init(&_sem, 0, sem_value);
        }

        void P()
        {
            sem_wait(&_sem);
        }

        void V()
        {
            sem_post(&_sem);
        }

        ~Sem()
        {
            sem_destroy(&_sem);
        }

    private:
        sem_t _sem;
    };
}