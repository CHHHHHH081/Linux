#include <iostream>
#include <unistd.h>
#include <signal.h>

void PrintSig(sigset_t set)
{
    printf("我是一个进程,pid: %d\n", getpid());
    for (int i = 31; i > 0; i--)
    {
        if (sigismember(&set, i) == 1)
        {
            printf("1");
        }
        else
        {
            printf("0");
        }
    }
    printf("\n");
}

void handler(int sig)
{
    printf("我是2号信号,捕捉成功\n");
}

int main()
{
    sigset_t set, oset;
    sigemptyset(&set);
    sigemptyset(&oset);
    sigaddset(&set, 2);
    sigprocmask(SIG_BLOCK, &set, &oset);
    int cnt = 0;
    while (1)
    {
        signal(2, handler);
        int n = sigpending(&set);
        (void)n;
        PrintSig(set);
        sleep(1);
        cnt++;
        if (cnt == 10)
        {
            sigprocmask(SIG_SETMASK, &oset, nullptr);
        }
    }
    return 0;
}

// void sighandler(int sig)
// {
//     if (sig == 9 || sig == 19)
//     {
//         std::cout << "Can Not Catch Sig: " << sig << std::endl;
//         return;
//     }
//     std::cout << "信号：" << sig << std::endl;
//     // exit(1);
// }

// int main()
// {
//     for (int i = 1; i <= 32; i++)
//         signal(i, sighandler);

//     // int cnt = 0;
//     // while (1)
//     //{
//     //     std::cout << "hello,cnt:" << cnt << std::endl;
//     //     sleep(1);
//     // }

//     for (int i = 1; i < 32; i++)
//     {
//         if (i == 9 || i == 19)
//             continue;
//         raise(i);
//         sleep(1);
//     }
//     // int *p = nullptr;
//     //*p = 100;
//     // int a = 2;
//     // a /= 0;
//     abort();
//     return 0;
// }