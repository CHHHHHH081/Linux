#pragma once

#include <iostream>
#include <string>
#include <sys/ipc.h>
#include <sys/shm.h>

const int gsize = 4096;

class shm
{
public:
private:
    key_t _key;
    int _shmid;
    int _size;
};
