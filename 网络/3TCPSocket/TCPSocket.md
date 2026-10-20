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

```c
int listen(int sockfd, int backlog);
```

**listen()** 函数声明 **sockfd** 处于监听状态。当有多个客户端同时发起连接请求时，这些连接请求需要排队等待处理。**listen()** 决定了最多允许有多少个客户端处于连接等待队列（全连接队列）中。

- sockfd： 已经绑定（bind）了地址和端口号的套接字文件描述符。
- backlog： 指定提示队列的最大排队连接数。即最多允许有 backlog 个客户端处于连接等待状态；如果收到更多的连接请求则会直接忽略。该值通常不会设置得太大（通常设置为 5 或 128 等）。
- 返回值： 成功返回 0，失败返回 -1。

```bash
# 查看所有 TCP 和 UDP 监听及连接状态（显示 PID 及程序名）
netstat -tnap

# 仅查看处于 LISTEN 状态的套接字
netstat -tnlp
```

当`listen`后，没有连接时，进程将处于`LISTEN`状态。

```bash
Proto Recv-Q Send-Q Local Address           Foreign Address         State
tcp        0      0 0.0.0.0:8080            0.0.0.0:*               LISTEN
```

而当我们用`telnet [IP] [Port]`建立连接后，进程将会处于`ESTABLISH`状态。

```bash
Proto Recv-Q Send-Q Local Address           Foreign Address         State      
tcp        0      0 0.0.0.0:8080            0.0.0.0:*               LISTEN      
tcp        0      0 127.0.0.1:38924         127.0.0.1:8080          ESTABLISHED
tcp        0      0 127.0.0.1:8080          127.0.0.1:38924         ESTABLISHED
```

这里连接后却出现了两个`ESTABLISH`，是因为：

- **TCP 是全双工通信**： 通信双方既是发送方也是接收方，双向通道独立存在。
- **同机测试**： 客户端（Client）与服务器（Server）部署在同一台机器上，系统在回环网卡（127.0.0.1）上同时记录了“客户端端口 -> 服务器端口”以及“服务器端口 -> 客户端端口”的两条对称链路连接信息。

## 4. accept

获取连接。

**说明：TCP建立连接的过程与是否调用accept无关，accept只是用于获取已经建立的连接**

```c
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
```

- **sockfd：** 监听套接字描述符（即 **`listensockfd`**），它专门负责传入 **accept()** 用于获取连接。
- **addr：** 输出型参数，用于获取连接成功的客户端网络地址信息（包含客户端的 IP 和端口号）。
- **addrlen：** 值-结果参数，传入时为 **addr** 结构体的长度，返回时为实际写入的地址结构体大小。
- **返回值：** **成功时返回一个全新的套接字文件描述符（服务套接字）**；如果获取失败，则返回 **-1**。**后续所有的数据读写都是在这个新的文件描述符上进行的。**

## 5. 发送/接收数据

**UDP** 是面向数据报的，具有特定的收发接口（如 **recvfrom/sendto**）；而 **TCP** 是面向字节流的，因此可以直接使用标准的 Linux 文件 I/O 系统调用 **read()** 与 **write()** 进行数据通信。