#include "shm.hpp"
#include "comm.hpp"

int main()
{
    umask(0);
    shm shm(CREATOR);
    char *addr = (char *)shm.VirtualAddr();

    NamedFifo fifo(FIFO_FILE);
    FileOp serverop(PATH, FIFO_FILE);
    serverop.openr();
    ssize_t rdcnt;

    while (1)
    {
        rdcnt = serverop.Read();
        if (rdcnt == 0)
            break;
        printf("%s\n", addr);
    }
    serverop.Close();
    return 0;
}