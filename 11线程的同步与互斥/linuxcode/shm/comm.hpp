#pragma once

#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <string>
#include <fcntl.h>
#include <unistd.h>

#define PATH "."
#define FIFO_FILE "fifo"

class NamedFifo
{
public:
    NamedFifo(std::string path, std::string name)
        : _path(path), _name(name)
    {
        _fifoname = _path + "/" + _name;
        int n = mkfifo(_fifoname.c_str(), 0666);
        if (n != 0)
            std::cerr << "mkfifo fail" << std::endl;
    }

    NamedFifo(std::string name)
        : _path(PATH), _name(name)
    {
        _fifoname = _path + "/" + _name;
        int n = mkfifo(_fifoname.c_str(), 0666);
        if (n != 0)
            std::cerr << "mkfifo fail" << std::endl;
    }

    std::string Path() { return _path; }
    std::string Name() { return _name; }
    std::string Fifoname() { return _fifoname; }

    ~NamedFifo()
    {
        unlink(_fifoname.c_str());
    }

private:
    std::string _path;
    std::string _name;
    std::string _fifoname;
};

class FileOp
{
public:
    FileOp(std::string path, std::string name)
        : _path(path), _name(name), _fd(-1)
    {
        _fifoname = _path + "/" + _name;
    }
    FileOp(NamedFifo fifo)
        : _path(fifo.Path()), _name(fifo.Name()), _fifoname(fifo.Fifoname()), _fd(-1)
    {
    }
    void openr()
    {
        _fd = open(FIFO_FILE, O_RDONLY);
        if (_fd == -1)
        {
            std::cerr << "open file fail" << std::endl;
            return;
        }
        else
        {
            std::cout << "open file success" << std::endl;
        }
    }
    void openw()
    {
        _fd = open(FIFO_FILE, O_WRONLY);
        if (_fd == -1)
        {
            std::cerr << "open file fail" << std::endl;
            return;
        }
        else
        {
            std::cout << "open file success" << std::endl;
        }
    }
    ssize_t Read()
    {
        char c;
        int n = read(_fd, &c, 1);
        return n;
    }
    void Write()
    {
        char c = 'o';
        write(_fd, &c, 1);
    }
    void Close()
    {
        if (_fd > 0)
        {
            close(_fd);
        }
        else
        {
            std::cerr << "fd==-1" << std::endl;
        }
    }

private:
    std::string _path;
    std::string _name;
    std::string _fifoname;
    int _fd;
};
