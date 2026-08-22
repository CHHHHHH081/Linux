#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstring>
#include <cstdio>

void ChildWrite(pid_t id, int fds[])
{
    char buf[1024];
    int cnt = 0;
    while (true)
    {
        snprintf(buf, sizeof(buf), "%d--I'm child,my pid: %d", cnt++, id);
        write(fds[1], buf, strlen(buf));
        sleep(1);
    }
}

void FatherRead(int fds[])
{
    char buf[1024];
    while (true)
    {
        buf[0] = 0;
        int n = read(fds[0], buf, sizeof(buf) - 1);
        if (n > 0)
        {
            buf[n] = 0;
        }
        std::cout << "Child says: " << buf << std::endl;
        sleep(1);
    }
}

int main()
{
    int fds[2] = {0}; // fds[0]读，fds[1]写
    int n = pipe(fds);
    if (n < 0)
    {
        std::cerr << "error" << std::endl;
    }
    pid_t id = fork();
    if (id == 0)
    {
        // 子进程，关闭读
        close(fds[0]);
        // 写入
        ChildWrite(getpid(), fds);
        close(fds[1]);
        exit(1);
    }
    // 父进程，关闭写
    close(fds[1]);
    FatherRead(fds);
    waitpid(id, nullptr, 0);
    close(fds[0]);
    return 0;
}
