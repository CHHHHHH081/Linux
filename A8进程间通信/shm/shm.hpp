#pragma once

#include <cerrno>
#include <cstdio>
#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define PATH "."

const int gsize = 4096;
const int gproj_id = 0;
const int gdefaultid = -1;

class shm
{
private:
    void Createhelper(int flg)
    {
        key_t k = ftok(PATH, gproj_id);
        if (k == -1)
        {
            perror("ftok fail!\n");
            exit(1);
        }
        _key = k;
        _shmid = shmget(_key, gsize, flg | 0666);
        if (_shmid == -1)
        {
            perror("shmget fail!\n");
            exit(1);
        }
        printf("Create/Get success! _shmid: %d\n", _shmid);
    }

public:
    shm() : _size(gsize), _shmid(gdefaultid), _start_mem(nullptr)
    {
    }

    void Create()
    {
        Createhelper(IPC_CREAT | IPC_EXCL);
    }

    void Get()
    {
        Createhelper(IPC_CREAT);
    }

    void Attach()
    {
        _start_mem = shmat(_shmid, nullptr, 0);
        if ((long long)_start_mem < 0)
        {
            perror("shmat fail!\n");
            exit(1);
        }
        printf("shmat success, VirtualAddr: %p", _start_mem);
    }

    void *VirtualAddr()
    {
        printf("VirtualAddr: %p\n", _start_mem);
        return _start_mem;
    }

    void Destroy()
    {
        if (_shmid == gdefaultid)
            return;
        int n = shmctl(_shmid, IPC_RMID, nullptr);
        if (n >= 0)
        {
            printf("Destroy success!\n");
        }
        else
        {
            printf("Destroy fail!\n");
        }
    }

    ~shm()
    {
        Destroy();
    }

private:
    key_t _key;
    void *_start_mem;
    int _shmid;
    int _size;
};
