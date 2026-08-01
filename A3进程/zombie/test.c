#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
    pid_t id=fork();
    if(id==0)
    {
        int cnt=5;
        while(cnt--)
        {
            sleep(1);
            printf("我是一个子进程，pid:%d\n",getpid());
        }
    }
    else
    {
        while(1)
        {
            sleep(1);
            printf("我是一个父进程，pid:%d\n",getpid());
        }
    }
    return 0;
}
