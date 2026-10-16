# 一、API介绍

## 1. socket

**socket()** 函数用于打开一个网络通讯端口。如果调用成功，它会像 **open()** 系统调用一样，返回一个套接字文件描述符，后续的网络读写操作都可以围绕该描述符展开。

```c
int socket(int domain, int type, int protocol);
```

对于TCP协议，传入的协议家族应该是 **SOCK_STREAM**，表示面向连接的、可靠的字节流传输协议。

## 2. bind

与UDP完全相同。

## 3. listen





## 4. accept

**重点：返回值**