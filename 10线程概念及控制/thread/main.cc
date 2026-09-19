#include "Thread.hpp"
// #include <iostream>
// #include <unistd.h>
// #include <thread>
// using namespace std;

// void routine(int &n)
// {

//     while (n++ < 100)
//     {
//         std::cout << "---我是新线程, n: " << n << ", tid: " << pthread_self() << std::endl;
//         sleep(1);
//     }
// }

// int main()
// {
//     int n = 0;
//     thread td(routine, n);
//     while (n < 100)
//     {
//         std::cout << "我是主线程, n: " << n << ", tid: " << pthread_self() << std::endl;
//         sleep(1);
//     }
//     td.join();
// }

int n = 0;

void routine(int &n)
{
    while (n++ < 100)
    {
        std::cout << "---我是新线程, n: " << n << ", tid: " << pthread_self() << std::endl;
        sleep(1);
    }
}

int main()
{
    Thread::thread<int> thd(routine, n);
    thd.Start();
    while (n < 100)
    {
        std::cout << "我是主线程, n: " << n << ", tid: " << pthread_self() << std::endl;
        sleep(1);
    }
    thd.Join();
    return 0;
}
