#include <iostream>
#include <unistd.h>
#include <signal.h>

void sighandler(int sig)
{
    if (sig == 9 || sig == 19)
    {
        std::cout << "Can Not Catch Sig: " << sig << std::endl;
        return;
    }
    std::cout << "信号：" << sig << std::endl;
    // exit(1);
}

int main()
{
    for (int i = 1; i <= 32; i++)
        signal(i, sighandler);

    // int cnt = 0;
    // while (1)
    //{
    //     std::cout << "hello,cnt:" << cnt << std::endl;
    //     sleep(1);
    // }

    for (int i = 1; i < 32; i++)
    {
        if (i == 9 || i == 19)
            continue;
        raise(i);
        sleep(1);
    }
    // int *p = nullptr;
    //*p = 100;
    // int a = 2;
    // a /= 0;
    abort();
    return 0;
}