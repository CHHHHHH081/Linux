#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include "comm.hpp"

int main()
{
    umask(0);
    int n = mkfifo(FIFO_FILE, 0666);
    if (n != 0)
        std::cerr << "mkfifo fail" << std::endl;
    int fd = open(FIFO_FILE, O_RDONLY);
    if (fd == -1)
        std::cerr << "open file fail" << std::endl;
    char buf[1024];
    while (1)
    {
        int t = read(fd, buf, sizeof(buf));
        if (t > 0)
        {
            buf[t] = 0;
            std::cout << "client says: " << buf << std::endl;
        }
    }
    close(fd);
    return 0;
}
