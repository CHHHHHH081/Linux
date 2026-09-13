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
    void Read()
    {
        char buf[1024];
        while (1)
        {
            int n = read(_fd, buf, sizeof(buf) - 1);
            if (n > 0)
            {
                buf[n] = 0;
                std::cout << "client says: " << buf << std::endl
                          << std::flush;
            }
            else if (n == 0)
            {
                std::cout << "client close!" << std::endl;
                break;
            }
            else
            {
                std::cout << "read error!" << std::endl;
                break;
            }
        }
    }
    void Write()
    {
        std::string s;
        while (1)
        {
            std::cout << "Please input: " << std::endl;
            std::getline(std::cin, s);
            write(_fd, s.c_str(), s.size());
        }
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
