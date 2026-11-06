# UDP Socket 编程

> **本篇定位**：从"UDP 协议是什么"到"能写出一个像样的 UDP 服务端"，把 API 用法、参数方向、地址绑定这些容易含糊的地方钉死。
>
> **配套代码**：`linux-network-programming/UDP/` 下的三个工程——
>
> | 工程 | 内容 | 本篇对应小节 |
> | --- | --- | --- |
> | `UDPEchoServer` | 回显服务器（最小可用示例） | 二、整体流程；三、四个必须想清楚的问题 |
> | `UDPTranslateSever` | 英译中翻译服务器（引入 `InetAddr`、`dict`） | 四、从回显到一个真正的服务 |
> | `UDPChatRoom` | 聊天室（路由 + 线程池 + 广播） | 4.3 聊天室 |
>
> **前置知识**：[第 1 篇 网络基础](../1网络基础/网络基础.md)（IP / 端口 / 字节序 / `sockaddr`）。

# 〇、先看 UDP 协议本身

写代码之前，先把 UDP 报文的样子记住，API 的行为就都能解释了。

## 0.1 报文格式：固定 8 字节头

| 源端口 (2B) | 目的端口 (2B) | 长度 (2B) | 校验和 (2B) | 数据… |
| --- | --- | --- | --- | --- |

只有 4 个字段，所有"复杂"的功能（连接管理、确认重传、排序、流量控制）UDP 一个都没有——**这些工作要么不做，要么交给应用层自己做**。

由此可以直接推出几条实践结论：

- **最大载荷 65507 字节**（`65535 - 20`(IP 头) `- 8`(UDP 头)）；
- 以太网 MTU = 1500 时，**单个 UDP 载荷建议不超过 1472 字节**（`1500 - 20 - 8`），否则会触发 IP 分片，任一分片丢失整包报废；
- **长度字段** 说明 UDP 天然知道一包的边界，所以**没有"粘包"问题**——一次 `sendto` 对应一次 `recvfrom`。

## 0.2 无连接意味着什么

| 特征 | 含义 | 代码上的体现 |
| --- | --- | --- |
| 无连接 | 发送前不握手，不维护连接状态 | 没有 `listen` / `accept` / `connect` |
| 不可靠 | 不确认、不重传、不排序 | `sendto` 成功 ≠ 对方收到 |
| 保留边界 | 一次发一个数据报 | 一次 `recvfrom` 收一个完整数据报 |
| 支持广播 / 组播 | 一包可以发给多个接收者 | 目的地址填广播地址或加入组播组 |
| 内核有接收缓冲 | `SO_RCVBUF` 满了就丢包 | 服务端处理慢会静默丢包 |

> `sendto` 成功只代表**数据被交给了内核协议栈**。内核把数据发到网卡之后，丢不丢、到不到、顺序如何，UDP 一概不管。这是理解"为什么 UDP 服务器必须自己处理超时和重传"的起点。

# 一、接口介绍

## 1. 头文件

```c
#include <sys/types.h>
#include <sys/socket.h>   // socket / bind / recvfrom / sendto
#include <netinet/in.h>   // sockaddr_in / INADDR_ANY / htons
#include <arpa/inet.h>    // inet_addr / inet_ntoa / inet_pton / inet_ntop
#include <unistd.h>       // close
#include <cerrno>         // errno
#include <cstring>        // strerror / bzero / memset
```

## 2. 创建套接字：socket

```c
int socket(int domain, int type, int protocol);
```

### 2.1 参数介绍

- **domain（协议族 / 地址族）**：指定通信的领域，表明是用于本地进程间通信还是网络通信。
  - **AF_UNIX / AF_LOCAL**：用于 Unix 本地进程间通信；
  - **AF_INET**：用于 IPv4 互联网协议；
  - **AF_INET6**：用于 IPv6 互联网协议。

- **type（套接字类型）**：指定通信的数据传输服务类型。
  - **SOCK_STREAM**：提供顺序、可靠、双向、基于连接的字节流通信（**TCP**）；
  - **SOCK_DGRAM**：支持数据报通信，即无连接、不可靠、固定最大长度的消息（**UDP**）。

- **protocol（协议类型）**：通常设置为 `0`，系统会根据 `domain` 和 `type` 自动推导出默认的协议类型（例如 `AF_INET` + `SOCK_DGRAM` 默认选择 UDP）。

### 2.2 返回值

成功返回文件描述符 `fd`；失败返回 `-1`，并设置 `errno`。

这与创建文件非常相似——**网络套接字底层也是文件**，所以 `read` / `write` / `close` 同样作用于 socket，`fd` 耗尽（`EMFILE`）和打开太多文件是同一类问题。

## 3. 填写 sockaddr_in

`struct sockaddr_in` 的结构如下：

```c
struct sockaddr_in {
    sa_family_t    sin_family;  // 16 位地址族 = AF_INET
    in_port_t      sin_port;    // 16 位端口，必须是【网络字节序】
    struct in_addr sin_addr;    // 32 位 IP，  必须是【网络字节序】
    char           sin_zero[8]; // 填充字节，用于保持与 struct sockaddr 大小一致，用 bzero/memset 清零即可
};
struct in_addr { uint32_t s_addr; };   // 32 位 IP
```

- **地址类型（`sa_family`）**：通过内核宏 `__SOCKADDR_COMMON(sin_)` 展开后为 `sa_family_t sin_family`，用于指定协议族（如 `AF_INET`）。其中 `##` 是 C 语言预处理器运算符，用于将符号拼接合并。
- **端口号（`sin_port`）**：16 位无符号整数，表示通信进程的端口号，**必须转成网络字节序**。
- **IP 地址（`sin_addr`）**：内部包含一个 32 位无符号整数，表示 IPv4 地址，**必须是网络字节序**。
- **填充物（`sin_zero`）**：填充 8 个字节，保证整个 `struct sockaddr_in` 结构体的大小（16 字节）与通用的 `struct sockaddr`（2 字节类型 + 14 字节地址数据）相契合。

> **为什么一定要 `bzero` / `memset` 清零？**
> `sin_zero` 不清零在多数系统上"看起来没问题"，但它是未初始化内存，属于 undefined behavior；而且某些实现会校验填充区。养成 `bzero(&addr, sizeof(addr))` 的习惯，代价极小。
>
> 另外，结构体的字段顺序在不同平台上可能被宏重排，**永远不要手动逐字节拼 `sockaddr_in`**，用字段赋值 + 字节序转换函数。

## 4. 字节序 + IP 地址转换 + bind

### 4.1 端口：`htons` / `ntohs`

```c
uint16_t htons(uint16_t hostshort);   // host → network, short(16 位)，填 sin_port 用
uint16_t ntohs(uint16_t netshort);    // network → host, short(16 位)，打印端口用
// 配套还有 32 位的 htonl / ntohl，IP 用它
```

| 函数 | 方向 | 位数 | 典型用途 |
| --- | --- | --- | --- |
| `htons` | 主机 → 网络 | 16 | `sin_port = htons(port)` |
| `ntohs` | 网络 → 主机 | 16 | `ntohs(peer.sin_port)` 打印 |
| `htonl` | 主机 → 网络 | 32 | 手工构造 `sin_addr.s_addr` |
| `ntohl` | 网络 → 主机 | 32 | 手工解析 `sin_addr.s_addr` |

### 4.2 IP：字符串 ⇄ `in_addr`

**字符串 → `in_addr`：**

```c
in_addr_t inet_addr(const char *cp);
```

其转换原理就是将字符串如 `192.168.0.1` 按 `.` 分成 4 个整数，每个整数占用一个字节，拼接组成 32 位整数，再将其调整为大端序。

> ⚠️ `inet_addr` 的返回值有歧义：出错时返回 `INADDR_NONE`（`0xFFFFFFFF`），而 `255.255.255.255` 的合法值恰好也是 `0xFFFFFFFF`，**无法区分**。生产代码更推荐 `inet_pton`。

**`in_addr` → 字符串：**

```c
char *inet_ntoa(struct in_addr in);
```

> ⚠️ `inet_ntoa` 使用**静态缓冲区**，不可重入、非线程安全，详见 6.1。

### 4.3 bind 绑定

```c
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```

服务端必须 `bind`：要把 socket 和"众所周知的端口"关联起来，客户端才知道往哪里发。

- 成功返回 `0`，失败返回 `-1` 并设置 `errno`；
- 常见错误：`EADDRINUSE`（端口被占用）、`EACCES`（绑定 1024 以下端口但没有 root）、`EADDRNOTAVAIL`（绑定了本机不存在的 IP，**比如云服务器的公网 IP**）。

## 5. 数据接收：recvfrom

UDP 服务器的核心逻辑往往是一个死循环，持续监听并接收客户端发送的数据。

`recvfrom` 函数用于接收面向无连接的数据包：

```c
ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
                 struct sockaddr *src_addr, socklen_t *addrlen);
```

- **sockfd**：`socket()` 函数返回的套接字文件描述符。
- **buf**：用于存放接收数据的内存缓冲区指针。
- **len**：缓冲区的最大可用字节数。**若数据报比 `len` 大，多出的部分会被丢弃**，返回值是被截断后的长度。
- **flags**：接收控制标志位，通常设置为 `0` 表示阻塞式接收；`MSG_DONTWAIT` 非阻塞、`MSG_PEEK` 窥探不取出。
- **src_addr**：输出型参数，传入一个 `struct sockaddr_in` 强转后的指针，用于获取发送方（客户端）的 IP 地址和端口号信息。
- **addrlen**：**输入输出型参数**。传入时为 `src_addr` 结构体的大小（`sizeof(struct sockaddr_in)`），返回时被内核改写为实际写入的地址信息结构体大小。
- **返回值**：成功时返回**实际接收到的数据字节数**；出错返回 `-1` 并设置 `errno`。

> **两个容易忽略的细节**
>
> 1. **UDP 的 `recvfrom` 返回 0 是合法的**——表示收到一个 0 字节的数据报，**不是**"对端关闭"（那是 TCP 的语义）。
> 2. **`len` 每轮都要重置**。对本例的 `sockaddr_in` 而言长度恒为 16，重置只是防御性习惯；但对变长地址（如 `sockaddr_un`）不重置会直接 `EINVAL`。

## 6. 数据发送：sendto

当服务器需要向客户端响应消息，或者客户端向服务器发送数据时，使用 `sendto` 系统调用：

```c
ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen);
```

- **sockfd**：通信使用的套接字文件描述符。
- **buf / len**：用户级缓冲区指针及需要发送的数据长度。
- **flags**：发送控制标志，通常设为 `0`。
- **dest_addr / addrlen**：目的端的套接字地址信息（`struct sockaddr_in` 转换而来）及其结构体大小，明确指定**数据发给谁**。
- **返回值**：成功时返回实际发送的字节数；出错时返回 `-1` 并设置 `errno`。
- 由于 UDP 是无连接的，发送端成功调用 `sendto` 仅意味着数据被交付给操作系统网络栈发送，并不保证对端一定能够接收到。

> `len` 超过 `65507` 时返回 `EMSGSIZE`。想发更长的数据，请自己在应用层分片（这也是第 4 篇自定义协议要解决的问题）。

## 7. 关闭与错误处理

```c
close(sockfd);   // 释放 fd；UDP 没有"挥手"，直接关
```

错误处理的标准姿势：

```cpp
if (n < 0) {
    LOG(Loglevel::ERROR) << "sendto error: " << strerror(errno);
}
```

`strerror(errno)` 是排查网络问题最快的工具——`EADDRINUSE`、`EACCES`、`EADDRNOTAVAIL`、`EMSGSIZE`、`ENETUNREACH` 各自对应一类问题（见 6.2）。

## 8. API 速查表

| 场景 | 服务端 | 客户端 |
| --- | --- | --- |
| 创建 | `socket(AF_INET, SOCK_DGRAM, 0)` | 同左 |
| 绑定本地地址 | **必须** `bind`（端口众所周知） | **隐式 bind**（首次 `sendto` 自动分配） |
| 发送 | `sendto`（发给 `peer`） | `sendto`（发给 `server`） |
| 接收 | `recvfrom`（拿到 `peer`） | `recvfrom`（拿到响应方地址） |
| 连接管理 | 无 `listen` / `accept` | 无 `connect` |
| 关闭 | `close` | `close` |

# 二、整体流程

## 2.1 时序图

```
客户端                                          服务端
  │  socket()                                      socket()
  │  (隐式 bind 在首次 sendto)                      bind(INADDR_ANY:port)
  │                                                recvfrom()  ← 阻塞等待
  │  sendto("hello")  ──────────────────────────►  recvfrom 返回 ("hello", peer)
  │                                                handler("hello") → "Hello, hello"
  │  recvfrom()  ←──────────────────────────────  sendto(result, peer)
  │  ...循环...                                     ...循环...
```

注意服务端**从头到尾只有一个 socket**，也没有"连接"这个东西：它靠 `recvfrom` 的输出参数 `peer` 记住"这一包是谁发的"，然后原路 `sendto` 回去。

## 2.2 服务端

### 2.2.1 创建 socket 并 bind 绑定 ip:port

中间需要利用 `bzero` 函数清零结构体。

```cpp
_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
if (_sockfd < 0)
{
    LOG(Loglevel::ERROR) << "server socket fail: " << strerror(errno);
    exit(1);
}

struct sockaddr_in addr;
bzero(&addr, sizeof(addr));
addr.sin_family      = AF_INET;
addr.sin_port        = htons(_port);
addr.sin_addr.s_addr = INADDR_ANY;          // 关键：监听本机所有网卡

int n = bind(_sockfd, (const sockaddr *)&addr, sizeof(addr));
if (n < 0)
{
    LOG(Loglevel::ERROR) << "server bind fail: " << strerror(errno);
    exit(2);
}
```

### 2.2.2 事件循环

```cpp
for (;;)                                    // 服务器永远不退出
{
    char buf[1024];
    struct sockaddr_in peer;                // 「谁发的」，由内核填
    socklen_t len = sizeof(peer);           // ★ 必须初始化，且每轮都重置

    ssize_t n = recvfrom(_sockfd, buf, sizeof(buf) - 1, 0,
                         (struct sockaddr *)&peer, &len);
    if (n < 0)
    {
        LOG(Loglevel::ERROR) << "server recv error: " << strerror(errno);
        continue;                           // 单次出错不应拖垮整个服务
    }

    buf[n] = 0;                             // ★ 手动补 '\0'，recvfrom 不加终止符
    string result = _handler(string(buf));  // 业务：交给 handler 处理

    // 目标信息已经在 peer 中了，原路发回
    sendto(_sockfd, result.c_str(), result.size(), 0,
           (struct sockaddr *)&peer, len);
}
```

三个 `★` 是这段代码里最容易写错的地方，也是面试常问的点：

| 标记 | 为什么必须这样写 |
| --- | --- |
| `len = sizeof(peer)` 每轮重置 | `len` 是输入输出参数，内核会改写它；不同地址类型长度不同 |
| `buf[n] = 0` | `recvfrom` 只搬运字节，不保证 `\0`；`string(buf)` 靠它找结尾 |
| 出错 `continue` | 一次 `EINTR` / `ECONNREFUSED`（收到 ICMP 端口不可达）不该让服务端退出 |

> **代码现状**：仓库里三个服务端在 `recvfrom` 失败时都写了 `exit(3)`，属于"能跑通"的学习版实现。真实服务应当区分错误类型：`EINTR` 直接重试，其余记录日志后 `continue`——**服务端进程的存活优先级高于单条请求**。

## 2.3 客户端

**UDP 的一个 fd 可以同时进行读写，因此 UDP 是全双工的。**

（更准确地说：socket 返回的 fd 本身既可读也可写，TCP/UDP 都支持全双工；UDP 的特点在于**不需要建立连接就能向任意对端 `sendto`**。）

### 2.3.1 创建 socket——与服务端一样

```cpp
int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
if (sockfd < 0) { LOG(Loglevel::ERROR) << "socket error: " << strerror(errno); return 2; }
```

### 2.3.2 客户端隐式 bind

client 要不要进行 bind？**一定要 bind，但是不需要显式 bind**：client 会在首次发送数据的时候自动进行 bind。

为什么？

- 底层必须有本地端口才能发数据 → 所以「要 bind」；
- 但客户端数量极多，端口写死必然冲突 → 所以让 OS 在首次 `sendto` 时**自动随机分配一个临时端口**；
- 客户端端口是多少**无所谓，唯一就行**；服务端端口必须众所周知。

> 这也解释了为什么 `ss -uanp` 看到的客户端端口每次运行都不一样（落在 `ip_local_port_range` 范围内）。

### 2.3.3 填充 sockaddr_in

```cpp
struct sockaddr_in server;
bzero(&server, sizeof(server));
server.sin_family      = AF_INET;
server.sin_port        = htons(server_port);
server.sin_addr.s_addr = inet_addr(server_ip.c_str());
```

注意：这里填的是**服务端的** `ip:port`，不是自己的。要先通过 `argv` 拿到 server 端地址，才能填充。这就是客户端唯一需要「指定地址」的地方。

### 2.3.4 循环收发

```cpp
while (true)
{
    std::string input;
    std::cout << "Please Enter# ";
    std::getline(std::cin, input);

    // 发送（首次 sendto 时 OS 自动 bind 本地随机端口）
    ssize_t n = sendto(sockfd, input.c_str(), input.size(), 0,
                       (struct sockaddr *)&server, sizeof(server));
    if (n < 0) { LOG(Loglevel::ERROR) << "sendto error: " << strerror(errno); continue; }

    // 接收回显
    char buffer[1024];
    struct sockaddr_in temp;                    // 收到的可能是别人，一般不看
    socklen_t len = sizeof(temp);
    ssize_t m = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0,
                         (struct sockaddr *)&temp, &len);
    if (m < 0) { LOG(Loglevel::ERROR) << "recvfrom error: " << strerror(errno); continue; }

    buffer[m] = 0;
    std::cout << "server echo# " << buffer << std::endl;
}
close(sockfd);
```

> **`recvfrom` 是阻塞的**：如果服务端不回包、或者包丢了，客户端就永久卡在 `recvfrom` 上。生产代码要给 socket 设置接收超时（`SO_RCVTIMEO`）或改用 `select` / `poll` / `epoll`——这也是后续"IO 多路复用"要解决的问题。

# 三、四个必须想清楚的问题

## 3.1 关于绑定：为什么服务端写 `INADDR_ANY`

**当我们直接绑定云服务器的公网 IP 时，会绑定失败。** 因为在云服务器上，网卡上配置的通常是**内网 IP（私网 IP）**，公网 IP 是通过 NAT（网络地址转换）映射到云服务器上的，它并没有直接配置到云服务器的本地网卡设备上。此时 `bind` 会返回 `EADDRNOTAVAIL`。

**如果我们选择绑 `127.0.0.1`（本地环回地址）或内网 IP，会有很大的局限性**：

- `127.0.0.1`：数据只能在系统内绕圈，只允许同一台主机上的客户端访问，无法跨网络访问；
- 内网 IP：当客户端使用 `127.0.0.1` 发送数据时将无法被接收；反之亦然。即：**显式绑定特定地址后，客户端必须且只能通过该特定地址才能成功访问**。

为了解决访问的局限性，我们可以使用 `INADDR_ANY` 进行绑定：

```c
// 在宏定义中，INADDR_ANY 即为 0x00000000 (全 0 地址)
#define INADDR_ANY ((in_addr_t) 0x00000000)

// 服务器端初始化代码（变量名对应 2.2.1 的 addr）：
addr.sin_addr.s_addr = INADDR_ANY;   // 全 0 本身就是网络字节序，无需 htonl
```

当我们绑定之后，在内核中，该被绑定的 IP 变为 `0.0.0.0`，即接受任意 IP 访问。

> 一句话记忆：**绑定地址是"我监听谁"，不是"我是谁"。** 服务端监听 `0.0.0.0:port` 意味着"本机所有网卡、所有 IP 上发往这个端口的数据都收"。

## 3.2 为什么 `recvfrom` 能拿到客户端地址，而客户端不用 `bind`

因为**客户端在 `sendto` 时内核已经替它 `bind` 过了**，地址记录在这个 fd 上。服务端 `recvfrom` 时，内核从**收到的 UDP 报头**里把源 IP（来自 IP 头）和源端口（来自 UDP 头）填进 `peer`，不需要任何额外状态。

所以"地址"这件事全部藏在**报头**里——这正是第 1 篇反复强调的"报头要能说明载荷交给谁 / 来自谁"。

## 3.3 UDP 没有连接，那服务端怎么区分不同客户端？

**只靠 `peer`（源 IP + 源端口）**，而且这个状态不在内核里，而在**你的代码里**。

这也意味着：

- 服务端重启，所有"客户端状态"全丢（聊天室尤其明显）；
- 任何人都能伪造源地址发一包（UDP 无握手，天然易受反射放大攻击）；
- 想让内核帮忙过滤对端，可以对 UDP socket 调 `connect()`——之后只收发该对端的包，但**仍然没有握手和重传**，"connected UDP" 只是加了地址过滤。

## 3.4 为什么 UDP 服务端"单线程"就够跑通三个项目？

因为 UDP 的 `recvfrom` **不创建新连接**，一个 socket 就能服务所有客户端，天然是"多路复用"的：

| 服务器类型 | 并发单位 | 需要 `accept` 吗 |
| --- | --- | --- |
| UDP 服务端 | 一个 socket 服务所有客户端 | 不需要 |
| TCP 服务端 | 每个 `accept` 返回一个新 fd，代表一条连接 | 需要 |

UDP 的瓶颈不在这里，而在**业务逻辑的耗时**：如果 handler 里有磁盘 IO、解析大文件等慢操作，`recvfrom` 就会停止收包，内核接收缓冲（`SO_RCVBUF`）填满后开始丢包。这就是 `UDPChatRoom` 里引入线程池的原因（见 4.3）。

# 四、从回显到一个真正的服务

三个工程其实是**同一个骨架**，只是 handler 的形态在演进。这也是面向对象封装的价值：`UDPSocket` 只负责"收 + 发"，业务被抽成回调。

## 4.1 回显服务器：把业务做成策略

```cpp
using func_t = std::function<std::string(const std::string)>;
```

服务端拿到 `string`，返回 `string`，完全不知道"回显"这件事：

```cpp
string defaulthandler(string s)
{
    string ret = "Hello, ";
    ret += s;
    return ret;
}

int main(int argc, char *argv[])
{
    uint16_t port = stoi(argv[1]);
    auto usvr = make_unique<UDPSocket>(port, defaulthandler);
    usvr->Init();      // socket + bind
    usvr->Start();     // 死循环：recvfrom → handler → sendto
}
```

**这就是"回调 / 策略模式"在网络编程中的典型用法**：`UDPSocket` 是框架（不变），`handler` 是业务（可变）。后面换成翻译、聊天室，都是换一个 `handler`。

## 4.2 翻译服务器：handler 需要知道"谁在问"

把 handler 的签名从 `string(const string)` 升级为 `string(const string, InetAddr&)`：

```cpp
using func_t = std::function<std::string(const std::string, InetAddr &)>;
```

`InetAddr` 把 `sockaddr_in` 包装成一个对象，省去每次手写 `inet_ntoa` + `ntohs`。下面是**聊天室工程里演进后的完整版本**（`UDPTranslateSever` 里的早期版本只有 `IP()` / `Port()` / `AddrString()`，且用的是线程不安全的 `inet_ntoa`）：

```cpp
class InetAddr
{
public:
    InetAddr(struct sockaddr_in addr) : _addr(addr)
    {
        char ipbuffer[64];
        inet_ntop(AF_INET, &_addr.sin_addr, ipbuffer, sizeof(ipbuffer));  // 线程安全
        _ip = ipbuffer;
        _port = ntohs(addr.sin_port);
        _addrstring = _ip + ":" + std::to_string(_port);
    }

    std::string IP() const { return _ip; }
    uint16_t Port() const { return _port; }
    const struct sockaddr_in &GetAddr() const { return _addr; }
    std::string AddrString() const { return _addrstring; }
    bool operator==(const InetAddr &addr) const { return _addrstring == addr._addrstring; }

private:
    struct sockaddr_in _addr;
    std::string _ip;
    uint16_t _port;
    std::string _addrstring;
};
```

业务侧只需要关心"词表里有没有这个词"：

```cpp
dict dict;
dict.LoadDict();                                   // 启动时一次性加载 dictionary.txt

auto usvr = make_unique<UDPSocket>(port, [&dict](const string eng, InetAddr &addr) -> string
                                   { return dict.Translate(eng, addr); });
```

而这背后的 `Translate` 就是一次哈希表查找，`dictionary.txt` 的格式是 `english: 中文`：

```cpp
string Translate(string eng, InetAddr &addr)
{
    auto it = _dicmap.find(eng);
    if (it == _dicmap.end())
        return "NONE";
    return it->second;
}
```

> **设计要点**：`dict` 在 `main` 里构造、被 lambda **按引用捕获**，生命周期覆盖整个服务；`LoadDict()` 只在启动时执行一次。千万不要把"读文件"放进 handler 里——那会让每次请求都产生磁盘 IO。

## 4.3 聊天室：路由与广播

聊天室需要"把一条消息发给所有人"，于是 handler 的职责从"返回结果"变成了"**执行一个动作**"：

```cpp
using func_t = std::function<void(int, const std::string &, InetAddr &)>;
```

`Route` 维护在线用户表，并负责广播：

```cpp
void MessageRoute(int sockfd, const std::string &msg, InetAddr &addr)
{
    LockGuard lg(_mutex);                    // 在线表是共享资源，必须加锁
    if (!IsExist(addr)) AddUser(addr);       // 第一次说话即视为上线
    string send_msg = addr.AddrString() + "# " + msg;
    for (auto &user : _online_users)
    {
        sendto(sockfd, send_msg.c_str(), send_msg.size(), 0,
               (const sockaddr *)&user.GetAddr(), sizeof(user.GetAddr()));
    }
    if (msg == "QUIT") DeleteUser(addr);     // 主动退出
}
```

而 `main` 里做的是"把路由任务丢进线程池"，让 `recvfrom` 尽快回到收包状态：

```cpp
auto tp = threadpool<task_t>::GetInstance();
Route r;

auto usvr = make_unique<UDPSocket>(port, [&r, &tp](int sockfd, const string &msg, InetAddr &addr)
                                   {
    task_t task = std::bind(&Route::MessageRoute, &r, sockfd, msg, addr);
    tp->Enqueue(task);
});
```

架构上分成了三层，职责很清晰：

```
UDPSocket        只负责「收包 / 发包」        ——  传输层之上的一层薄封装
   ↓ 回调
handler          只负责「把这件事交给谁做」    ——  分发层
   ↓ 入队
Route + 线程池   只负责「业务：谁在线、发给谁」 ——  业务层
```

> ⚠️ 一个值得注意的细节：`InetAddr` 的 `==` 比较的是 `"ip:port"` 字符串。在 NAT 后面，多个真实用户可能共享同一个公网 IP，靠端口区分；而客户端每次重启端口都会变，所以"在线用户"实际是"在线的 socket 地址"，而不是"用户账号"。要做真正的用户体系，需要在应用层协议里加用户名（见第 4 篇）。

## 4.4 三个工程对比

| | EchoServer | TranslateSever | ChatRoom |
| --- | --- | --- | --- |
| handler 签名 | `string(const string)` | `string(const string, InetAddr&)` | `void(int, const string&, InetAddr&)` |
| 是否用 `InetAddr` | 否（`inet_ntoa`） | 是 | 是 |
| 是否用线程池 | 否 | 否 | 是 |
| 业务状态 | 无 | 词表（只读） | 在线用户表（读写，加锁） |
| 关键知识点 | 最小骨架 | 回调 + 封装 | 广播 + 并发 + 锁 |

# 五、进阶：现代写法与常见 API

## 5.1 `inet_pton` / `inet_ntop`

这里介绍两个现代网络编程常用的 API：

```c
// 字符串 IP -> 二进制网络字节序 IP
int inet_pton(int af, const char *src, void *dst);

// 二进制网络字节序 IP -> 字符串 IP
const char *inet_ntop(int af, const void *src, char *dst, socklen_t size);
```

**支持协议族**：第一个参数 `af` 可显式指定 `AF_INET` (IPv4) 或 `AF_INET6` (IPv6)。

**绝对线程安全**：`inet_ntop` 要求调用者**显式传入一个由调用方维护的内存缓冲区指针**（如栈上分配的 `char ipbuffer[64]`），避免了内部静态缓冲区的依赖，每个线程在自己的栈帧空间中操作，从根本上消除了线程安全隐患。

对比：

| | `inet_addr` / `inet_ntoa` | `inet_pton` / `inet_ntop` |
| --- | --- | --- |
| IPv6 | 不支持 | 支持 |
| 线程安全 | `inet_ntoa` 不安全 | 安全 |
| 错误判断 | `INADDR_NONE` 有歧义 | 返回 1 / 0 / -1，语义明确 |
| 推荐度 | 仅老代码 / 教学 | **新代码首选** |

## 5.2 关于 `inet_ntoa`（一定要知道的历史包袱）

```c
char *inet_ntoa(struct in_addr in);
```

`inet_ntoa` 内部使用了一个**静态局部缓冲区（static buffer）**来保存转换后的字符串。在**多线程并发等==多次调用该接口==的环境下**，如果多个线程同时调用 `inet_ntoa`，后续线程的转换结果会覆盖之前线程缓存在静态区中的 IP 字符串，引发严重的并发数据竞争问题。

因此它有上位替代：`inet_ntop`。

> 仓库里 `UDPEchoServer` 和 `UDPTranslateSever` 的早期 `InetAddr` 用的是 `inet_ntoa`，`UDPChatRoom` 已经换成了 `inet_ntop` + 自己的缓冲区——**这个演进过程本身就是最好的反面教材**。

## 5.3 其他值得知道的 socket 选项

```c
int opt = 1;
setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));   // 快速重启、避免 TIME_WAIT 影响

struct timeval tv = {3, 0};
setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));      // 接收超时，避免永久阻塞
```

| 选项 | 作用 |
| --- | --- |
| `SO_REUSEADDR` | 允许绑定处于 `TIME_WAIT` 的端口（TCP 服务端几乎必备） |
| `SO_RCVBUF` | 调整接收缓冲区；UDP 丢包严重时可适当调大 |
| `SO_RCVTIMEO` / `SO_SNDTIMEO` | 给阻塞收发加超时 |
| `SO_BROADCAST` | 允许发送广播包（目的地址为 `255.255.255.255` 时） |

# 六、常见坑与排错

## 6.1 `inet_ntoa` 的线程安全问题

```cpp
// ❌ 多线程下会串数据：所有调用共享同一块静态缓冲区
std::string ip = inet_ntoa(peer.sin_addr);
LOG(Loglevel::INFO) << ip << " 上线";     // 中间别的线程可能又调了一次 inet_ntoa

// ✅ inet_ntop + 自己的缓冲区
char ipbuf[64];
inet_ntop(AF_INET, &peer.sin_addr, ipbuf, sizeof(ipbuf));
std::string ip = ipbuf;
```

`inet_ntoa` 返回的是指向静态区的指针，**后一次调用会覆盖前一次的结果**——即使在同一线程里，"先拿到指针、再拷贝"之间如果发生了其他调用，也可能被改写。原理见 5.2。

## 6.2 `bind` 失败的三类原因

| `errno` | 原因 | 解决 |
| --- | --- | --- |
| `EADDRINUSE` | 端口被占用（上次没退干净 / 别的进程在用） | `ss -uanp \| grep <port>` 查占用者；或加 `SO_REUSEADDR` |
| `EACCES` | 绑定 1024 以下端口但非 root | 换端口，或用 `CAP_NET_BIND_SERVICE` |
| `EADDRNOTAVAIL` | 绑定了本机不存在的 IP（典型：云服务器公网 IP） | 用 `INADDR_ANY` 或内网 IP |

## 6.3 客户端收不到回包

按这个顺序排查：

1. **地址填对了吗**：`inet_addr` 参数是不是 `strerror` 吞掉了错误（返回值没检查）；
2. **防火墙 / 安全组**：云服务器的安全组默认只放行特定端口，UDP 要单独放行；
3. **绑错地址**：服务端绑了 `127.0.0.1`，外部自然访问不到；
4. **包确实丢了**：UDP 无重传，弱网下直接丢——用 `tcpdump` 确认包有没有到达网卡。

```bash
ss -uanp                                   # 看端口有没有在监听、被谁占用
netstat -anu | grep 8888                   # 老工具，等价用法
tcpdump -i any -nn udp port 8888           # 抓包：确认发出去 / 收进来了
```

## 6.4 其他高频错误

| 现象 | 原因 |
| --- | --- |
| 收到的数据末尾有乱码 | 忘了 `buf[n] = '\0'` |
| 只能收到第一条消息 | `recvfrom` 放在了循环外 |
| 第二次 `recvfrom` 返回 `EINVAL` | `addrlen` 没重置（变长地址场景） |
| 大包收不全 | `len` 小于数据报长度，**多余部分被静默丢弃** |
| 服务端莫名其妙退出 | `recvfrom` 返回 -1 就 `exit`，遇到 `EINTR` 直接挂 |
| 多线程下 IP 打印串了 | `inet_ntoa` 静态缓冲区竞争 |
| 认为"`sendto` 成功就是发出去了" | 只代表交给了内核，UDP 无确认 |

# 七、小结

## 7.1 一页纸小结

| 问题 | 答案 |
| --- | --- |
| UDP 服务端要几步 | `socket` → `bind` → 循环 `recvfrom` / `sendto` |
| UDP 客户端要几步 | `socket` → 填服务端地址 → 循环 `sendto` / `recvfrom`（隐式 bind） |
| 服务端绑什么地址 | `INADDR_ANY`（`0.0.0.0`），而不是公网 IP / `127.0.0.1` |
| 为什么要 `bzero` | `sin_zero` 填充区必须清零，避免 UB |
| 谁需要 `htons` | 端口一定需要；IP 用 `inet_addr` / `inet_pton` 时已经是大端 |
| 一次发包的大小上限 | 理论上 65507；以太网上建议 ≤ 1472（避免 IP 分片） |
| 为什么 UDP 没有粘包 | 报文头有长度字段，保留数据边界 |
| 怎么让服务端不卡死 | handler 里不做慢操作；必要时用线程池 / IO 多路复用 |

## 7.2 自测题

1. `recvfrom` 的 `addrlen` 为什么是"输入输出型参数"？不重置会怎样？
2. 客户端没有显式 `bind`，服务端为什么还能拿到客户端的端口？
3. 服务端 `bind` 到 `0.0.0.0:8888` 和 `192.168.1.10:8888`，行为有什么区别？
4. `sendto` 返回 100，能说明对端收到了 100 字节吗？
5. `recvfrom` 返回 0，在 UDP 里意味着什么？在 TCP 的 `recv` 里又意味着什么？
6. 为什么聊天室要给 `Route` 加锁？不加会出什么问题？
7. `inet_ntoa` 在多线程下为什么危险？怎么改？
8. 一个 UDP 数据报 2000 字节，超过 MTU 1500，会发生什么？

<details>
<summary>参考答案（点击展开）</summary>

1. 调用前告诉内核缓冲区多大，调用后内核写回实际大小；不重置在变长地址（`sockaddr_un`）场景会 `EINVAL`。
2. 内核在首次 `sendto` 时自动完成隐式 `bind`，源端口写在 UDP 报头里，服务端从报头解析。
3. `0.0.0.0` 收本机所有网卡的包；绑定具体 IP 只收该 IP 上的包，`127.0.0.1` 来的包收不到。
4. 不能。只说明内核接收了这 100 字节，UDP 不保证送达。
5. UDP：收到一个 0 字节数据报（合法）；TCP：对端关闭连接（EOF）。
6. `_online_users` 会被多个线程同时读写，不加锁会出现迭代器失效 / 数据竞争（`std::vector` 非线程安全）。
7. 内部静态缓冲区被多次调用覆盖，导致线程间串数据；换成 `inet_ntop` + 自有缓冲区。
8. IP 层分片为多个分片，任一分片丢失整个数据报都不可用；建议应用层控制单包 ≤ 1472 字节。

</details>

## 7.3 下一步

- **TCP**：`listen` / `accept` / `connect`，三次握手，连接 fd 的创建与回收 → [第 3 篇 TCP Socket](../3TCPSocket/TCPSocket.md)
- **协议设计**：如何在一个 UDP/TCP 字节流上定义"长度 + 类型 + 载荷"，解决粘包与序列化 → [第 4 篇 应用层自定义协议与序列化](../4应用层自定义协议与序列化/应用层协议与序列化.md)
