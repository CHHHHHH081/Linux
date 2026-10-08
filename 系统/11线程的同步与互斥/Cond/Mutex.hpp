#include <pthread.h>
#include <iostream>
#include <string>
#include <unistd.h>

namespace Mutexmodule
{
    class Mutex
    {
    public:
        Mutex()
        {
            try
            {
                int n = pthread_mutex_init(&_Mutex, nullptr);
                if (n != 0)
                    throw n;
            }
            catch (const int &n)
            {
                std::cerr << "init error code: " << n << '\n';
            }
        }

        void Lock()
        {
            try
            {
                int n = pthread_mutex_lock(&_Mutex);
                if (n != 0)
                    throw n;
            }
            catch (const int &n)
            {
                std::cerr << "lock error code: " << n << '\n';
            }
        }

        void Unlock()
        {
            try
            {
                int n = pthread_mutex_unlock(&_Mutex);
                if (n != 0)
                    throw n;
            }
            catch (const int &n)
            {
                std::cerr << "unlock error code: " << n << '\n';
            }
        }

        pthread_mutex_t *GetMutexpointer() { return &_Mutex; }

        ~Mutex()
        {
            try
            {
                int n = pthread_mutex_destroy(&_Mutex);
                if (n != 0)
                    throw n;
            }
            catch (const int &n)
            {
                std::cerr << "destroy error code: " << n << '\n';
            }
        }

    private:
        pthread_mutex_t _Mutex;
    };

    class LockGuard
    {
    public:
        LockGuard(Mutex &Mutex) : _Mutex(Mutex)
        {
            _Mutex.Lock();
        }
        ~LockGuard()
        {
            _Mutex.Unlock();
        }

    private:
        Mutex &_Mutex;
    };
}
