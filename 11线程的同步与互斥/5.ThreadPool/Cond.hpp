#pragma once
#include <pthread.h>
#include "Mutex.hpp"

namespace CondModule
{
    class Cond
    {
    public:
        Cond()
        {
            pthread_cond_init(&_cond, nullptr);
        }

        void Wait(Mutexmodule::Mutex mutex)
        {
            pthread_cond_wait(&_cond, mutex.GetMutexpointer());
        }

        void Signal()
        {
            pthread_cond_signal(&_cond);
        }

        void Broadcast()
        {
            pthread_cond_broadcast(&_cond);
        }

    private:
        pthread_cond_t _cond;
    };
}
