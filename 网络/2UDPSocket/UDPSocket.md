# 一、接口介绍

## 1. 头文件

```c
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
```

## 2. 创建套接字：socket

```c
int socket(int domain, int type, int protocol);
```

### 2.1 参数介绍

- **domain（协议族/地址族）**：指定通信的领域，表明是用于本地进程间通信还是网络通信。
    - **AF_UNIX / AF_LOCAL**：用于 Unix 本地进程间通信。
    - **AF_INET**：用于 IPv4 互联网协议。
    - **AF_INET6**：用于 IPv6 互联网协议。

- **type（套接字类型）**：指定通信的数据传输服务类型。
    - **SOCK_STREAM**：提供顺序、可靠、双向、基于连接的字节流通信（**TCP**）。
    - **SOCK_DGRAM**：支持数据报通信，即无连接、不可靠、固定最大长度的消息（**UDP**）。

- **protocol（协议类型）**：通常设置为 0，系统会根据 domain 和 type 自动推导出默认的协议类型（例如针对AF_INET 和 SOCK_DGRAM 默认选择 UDP 协议）。

### 2.2 返回值

成功返回文件描述符fd；失败返回-1，并设置错误码。这与创建文件非常相似，网络套接字底层也是文件。

## 3. 填写socketaddr_in

`struct socketaddr_in`的结构如下：

```c
struct sockaddr_in {
    sa_family_t    sin_family;  // 16位地址族 = AF_INET
    in_port_t      sin_port;    // 16位端口，必须是【网络字节序】
    struct in_addr sin_addr;    // 32位IP，  必须是【网络字节序】
    char           sin_zero[8]; // 填充字节，用于保持与struct sockaddr大小一致，用 bzero/memset 清零即可
};
struct in_addr { uint32_t s_addr; };   // 32 位 IP
```

- **地址类型（sa_family）**：通过内核宏 `__SOCKADDR_COMMON(sin_)` 展开后为 `sa_family_t sin_family`，用于指定协议族（如 `AF_INET`）。其中 `##` 是 C 语言预处理器运算符，用于将符号拼接合并。
- **端口号（sin_port）**：16 位无符号整数，表示通信进程的端口号。
- **IP 地址（sin_addr）**：内部包含一个 32 位无符号整数，表示 IPv4 地址。
- **填充物（sin_zero）**：填充 8 个字节，保证整个 `struct sockaddr_in` 结构体的大小与通用的 `struct sockaddr`（14字节地址数据 + 2字节类型）相契合。

## 4. 字节序 + IP地址转换 + bind

**端口：`htons` / `ntohs`**

```c
uint16_t htons(uint16_t hostshort);   // host → network, short(16位)，填 sin_port 用
uint16_t ntohs(uint16_t netshort);    // network → host, short(16位)，打印端口用
// 配套还有 32 位的 htonl / ntohl，IP 用它
```

**IP：字符串 ⇄ `in_addr`**

**字符串 → `in_addr`：**

```c
in_addr_t inet_addr(const char *cp);
```

其转换原理就是将字符串如：`192.168.0.1`按`.`分成4个整数，每个整数占用一个字节，拼接组成32位整数，再将其调整为大端序。

**`in_addr` → 字符串：**

```c
char *inet_ntoa(struct in_addr in);
```

**`bind`绑定：**

```c
int bind(int sockfd, const struct sockaddr *addr,socklen_t addrlen);
```

## 5. 数据接收与响应处理：recvfrom

UDP 服务器的核心逻辑往往是一个死循环，持续监听并接收客户端发送的数据。

`recvfrom` 函数用于接收面向无连接的数据包：

```c
ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
                 struct sockaddr *src_addr, socklen_t *addrlen);
```

- **sockfd**：`socket()` 函数返回的套接字文件描述符。
- **buf**：用于存放接收数据的内存缓冲区指针。
- **len**：缓冲区的最大可用字节数。
- **flags**：接收控制标志位，通常设置为 `0` 表示阻塞式接收。
- **src_addr**：输出型参数，填入一个 `struct sockaddr_in` 强转后的指针，用于获取发送方（客户端）的 IP 地址和端口号信息。
- **addrlen**：输入输出型参数，传入时为 `src_addr` 结构体的大小（`sizeof(struct sockaddr_in)`），返回时为内核实际写入的地址信息结构体大小。
- **返回值**：成功时返回**实际接收到的数据字节数**；若连接关闭或出错则返回 `-1` 并设置 `errno`。

## 6. 数据发送：sendto

当服务器需要向客户端响应消息，或者客户端向服务器发送数据时，使用 `sendto` 系统调用：

```c
ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen);
```

- **sockfd**：通信使用的套接字文件描述符。
- **buf / len**：用户级缓冲区指针及需要发送的数据长度。
- **flags**：发送控制标志，通常设为 0。
- **dest_addr / addrlen**：目的端的套接字地址信息（struct sockaddr_in 转换而来）及其结构体大小，明确指定**数据发给谁**。
- **返回值**：功时返回实际发送的字节数；出错时返回 `-1` 并设置 `errno`。
- 于 UDP 是无连接的，发送端成功调用 `sendto` 仅意味着数据被交付给操作系统网络栈发送，并不保证对端一定能够接收到。

# 二、整体流程

## 1. 服务端

### 创建socket并bind绑定ip:port​

中间需要利用`bzero`函数清零结构体。

```cpp
_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
if (_sockfd < 0)
{
    LOG(LogLevel::FATAL) << "socket error: " << strerror(errno);
    exit(1);
}

struct sockaddr_in local;
bzero(&local, sizeof(local));
local.sin_family		= AF_INET;
local.sin_port			= htons(_port);
local.sin_addr.s_addr 	= inet_addr(_ip.c_str());
int n = bind(_sockfd, (struct sockaddr *)&local, sizeof(local));
if (n < 0)
{
    LOG(LogLevel::FATAL) << "bind error: " << strerror(errno);
    exit(2);
}
```

### 事件循环

```cpp
char buffer[1024];
for (;;)                                       // 服务器永远不退出
{
    struct sockaddr_in peer;                   // 「谁发的」，由内核填
    socklen_t len = sizeof(peer);              // ★ 必须初始化，且每轮都重置

    ssize_t n = recvfrom(_sockfd, buffer, sizeof(buffer) - 1, 0,
                         (struct sockaddr *)&peer, &len);
    if (n < 0)
    {
        LOG(LogLevel::ERROR) << "recvfrom error: " << strerror(errno);
        continue;                              // ★ 绝不能 break，服务端要一直活着
    }

    buffer[n] = 0;                             // ★ 手动补 '\0'
    std::string peer_ip = inet_ntoa(peer.sin_addr);
    uint16_t peer_port  = ntohs(peer.sin_port);
    LOG(LogLevel::INFO) << "[" << peer_ip << ":" << peer_port << "]# " << buffer;

    std::string result = Handler(std::string(buffer));   // ⑤ 业务

    sendto(_sockfd, result.c_str(), result.size(), 0,    // ⑥ 用 peer 原路发回
           (struct sockaddr *)&peer, len);
}
```

## 2. 客户端

**UDP的一个fd可以同时进行读写，因此UDP是全双工的。**

### 创建socket——与服务端一样

```cpp
int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
if (sockfd < 0) { std::cerr << "socket error: " << strerror(errno) << std::endl; return 2; }
```

### 客户端隐式bind

client 要不要进行 bind？**一定要 bind 的！但是不需要显式 bind**，client 会在首次发送数据的时候自动进行 bind。 为什么？server 端的端口号一定是众所周知、不可改变的；而**client 会非常多**，client 需要 port，**bind 随机端口**。

- 底层必须有本地端口才能发数据 → 所以「要 bind」；
- 但客户端数量极多，端口写死必然冲突 → 所以让 OS 在首次 `sendto` 时**自动随机分配一个临时端口**；
- 客户端端口是多少**无所谓，唯一就行**；服务端端口必须众所周知。

### 填充sockaddr_in

```cpp
struct sockaddr_in server;
bzero(&server,sizeof(server));
server.sin_family      = AF_INET;
server.sin_port        = htons(server_port);
server.sin_addr.s_addr = inet_addr(server_ip.c_str());
```

注意：这里填的是**服务端的** `ip:port`，不是自己的，首先要通过argv拿到server端socket，才能填充。这就是客户端唯一需要「指定地址」的地方。

### 循环收发

```cpp
while (true)
{
    std::string input;
    std::cout << "Please Enter# ";
    std::getline(std::cin, input);

    // 发送（首次 sendto 时 OS 自动 bind 本地随机端口）
    ssize_t n = sendto(sockfd, input.c_str(), input.size(), 0,
                       (struct sockaddr *)&server, sizeof(server));
    if (n < 0) { std::cerr << "sendto error: " << strerror(errno) << std::endl; continue; }

    // 接收回显
    char buffer[1024];
    struct sockaddr_in temp;                        // 收到的可能是别人，一般不看
    socklen_t len = sizeof(temp);
    ssize_t m = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0,
                         (struct sockaddr *)&temp, &len);
    if (m < 0) { std::cerr << "recvfrom error: " << strerror(errno) << std::endl; continue; }

    buffer[m] = 0;
    std::cout << "server echo# " << buffer << std::endl;
}
close(sockfd);
```

## 关于绑定

**当我们直接绑定云服务器的公网IP时，会绑定失败**。因为在云服务器上，网卡上配置的通常是**内网IP（私网IP）**，公网 IP 是通过 NAT（网络地址转换）映射到云服务器上的，它并没有直接配置到云服务器的本地网卡设备上。

**如果我们选择绑`127.0.0.1`（本地环回地址）或内网IP，会有很大的局限性**。

- `127.0.0.1`：数据只能在系统内绕圈，只允许同一台主机上的客户端访问，无法跨网络访问。
- 内网IP：当客户端使用 `127.0.0.1` 发送数据时将无法被接收；反之亦然。即：**显式绑定特定地址后，客户端必须且只能通过该特定地址才能成功访问**。

为了解决访问的局限性，我们可以使用 INADDR_ANY 进行绑定：

```c
// 在宏定义中，INADDR_ANY 即为 0x00000000 (全 0 地址)
#define INADDR_ANY ((in_addr_t) 0x00000000)

// 服务器端初始化代码：
local.sin_addr.s_addr = INADDR_ANY; // 自动转换为网络字节序的全 0
```

当我们绑定之后，在内核中，该被绑定的IP变为`0.0.0.0`，即接受任意IP访问。

