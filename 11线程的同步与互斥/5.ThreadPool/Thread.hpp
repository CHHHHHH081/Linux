#ifndef __THREAD_HPP
#define __THREAD_HPP

#include <unistd.h>
#include <pthread.h>
#include <string>
#include <iostream>
#include <functional>
#include <cstdlib>
#include <cstdio>

namespace Thread
{
    static int num = 1;
    template <typename T>
    class thread
    {
        using func_t = std::function<void(T &)>;

    private:
        static void *routine(void *args)
        {
            thread<T> *thd = static_cast<thread<T> *>(args);
            thd->_isrunning = true;
            thd->_func(thd->_data);
            return nullptr;
        }

        void EnableDetach() { _joinable = false; }

    public:
        thread(func_t func, T &data) : _func(func), _data(data), _joinable(true)
        {
            _name = "Thread-" + std::to_string(num++);
            _pid = getpid();
        }

        bool Detach()
        {
            if (_isrunning == false)
                EnableDetach();
            else
            {
                try
                {
                    int n = pthread_detach(_tid);
                    if (n != 0)
                        throw n;
                    else
                        std::cout << "detach success" << std::endl;
                }
                catch (const int &n)
                {
                    std::cerr << "detach error code: " << n << '\n';
                    return false;
                }
            }
            return true;
        }

        bool Start()
        {
            pthread_t tid;
            try
            {
                int n = pthread_create(&tid, nullptr, routine, this);
                _tid = tid;
                _isrunning = true;
                if (n != 0)
                    throw n;
            }
            catch (const int &n)
            {
                std::cerr << "create error code: " << n << '\n';
                return false;
            }
            std::cout << "create success, tid: " << _tid << std::endl;
            if (_joinable == false)
                Detach();
            return true;
        }

        bool Join()
        {
            if (_joinable == false)
            {
                std::cerr << "cannot join!" << std::endl;
                return false;
            }
            else
            {
                try
                {
                    int n = pthread_join(_tid, nullptr);
                    if (n != 0)
                        throw n;
                }
                catch (const int &n)
                {
                    std::cerr << "join fail, error code: " << n << '\n';
                    return false;
                }
            }
            std::cout << "join success!" << std::endl;
            return true;
        }

        bool Stop()
        {
            if (_isrunning == true)
            {
                try
                {
                    int n = pthread_cancel(_tid);
                    if (n != 0)
                        throw n;
                }
                catch (const int &n)
                {
                    std::cerr << "stop error code: " << n << '\n';
                    return false;
                }
                std::cout << "stop success!" << std::endl;
                return true;
            }
            else
            {
                std::cout << "thread has stopped!" << std::endl;
                return false;
            }
        }

        std::string name() { return _name; }

    private:
        pthread_t _tid;
        pid_t _pid;
        std::string _name;
        bool _joinable;
        bool _isrunning;
        T &_data;
        void *ret;
        func_t _func;
    };
}

#endif
