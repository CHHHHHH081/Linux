#include "Socket.hpp"

using namespace SocketModule;

class Request
{
public:
    Request() {}
    Request(int x, int y, char oper)
        : _x(x), _y(y), _oper(oper)
    {
    }
    std::string Serialize()
    {
    }
    void Deserialize(std::string)
    {
    }

private:
    int _x;
    int _y;
    char _oper;
};

class Response
{
public:
    Response() {}
    Response(int res, int code)
        : _result(res), _code(code)
    {
    }
    std::string Serialize()
    {
    }
    void Deserialize(std::string)
    {
    }

private:
    int _result;
    int _code;
};

class Protocal
{
public:
    Protocal() {}
    void GetRequest(std::shared_ptr<Socket> &sock, InetAddr &addr)
    {
    }

private:
};
