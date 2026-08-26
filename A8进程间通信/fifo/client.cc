#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include "comm.hpp"

int main()
{
    int fd = open(FIFO_FILE, O_WRONLY);
    if (fd == -1)
        std::cerr << "open file fail" << std::endl;
    std::string s;
    while (1)
    {
        std::cout << "Please in: " << std::endl;
        std::cin >> s;
        int t = write(fd, s.c_str(), s.size());
        if (t > 0)
        {
            std::cout << "I say: " << s << std::endl;
            s[t] = 0;
        }
    }
    return 0;
}