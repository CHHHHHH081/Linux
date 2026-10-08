#include "comm.hpp"

int main()
{
    FileOp clientop(PATH, FIFO_FILE);
    clientop.openw();
    clientop.Write();
    clientop.Close();
    return 0;
}