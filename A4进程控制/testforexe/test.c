#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
int main()
{
    printf("准备开始\n");
    if(fork()==0)
    {
        sleep(1);
        //子进程
        execl("/usr/bin/ls","/usr/bin/ls","-ln","-a",NULL);
        exit(1);
    }
    waitpid(-1,NULL,0);
    printf("进程运行完毕\n");
    return 0;
}
