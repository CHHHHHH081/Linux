#include "shm.hpp"
#include "comm.hpp"

int main()
{
    shm shm(USER);
    char *addr = (char *)shm.VirtualAddr();
    int cur = 0;
    NamedFifo fifo(FIFO_FILE);
    FileOp clientop(PATH, FIFO_FILE);
    clientop.openw();
    for (char c = 'A'; c < 'Z'; c++)
    {
        sleep(1);
        addr[cur] = c;
        sleep(1);
        addr[cur + 1] = c;
        cur += 2;
        clientop.Write();
    }
    clientop.Close();
    return 0;
}
