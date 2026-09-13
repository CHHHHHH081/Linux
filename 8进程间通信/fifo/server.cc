#include "comm.hpp"

int main()
{
    umask(0);
    NamedFifo fifo(PATH, FIFO_FILE);
    // mkfifo(FIFO_FILE, 0666);
    FileOp serverop(PATH, FIFO_FILE);
    serverop.openr();
    serverop.Read();
    serverop.Close();
    return 0;
}
