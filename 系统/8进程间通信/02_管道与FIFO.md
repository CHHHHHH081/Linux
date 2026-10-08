# A8｜管道与 FIFO

## 1. 匿名管道

`pipe(int fd[2])` 创建一条单向字节流：`fd[0]` 为读端，`fd[1]` 为写端。常在 `fork()` 后使用，父子进程继承同一组文件描述符。

```c
int fd[2];
if (pipe(fd) == -1) perror("pipe");
pid_t pid = fork();
if (pid == 0) {                 // 子进程只读
    close(fd[1]);
    char buf[128];
    ssize_t n = read(fd[0], buf, sizeof buf);
    close(fd[0]);
} else {                        // 父进程只写
    close(fd[0]);
    write(fd[1], "hello", 5);
    close(fd[1]);
}
```

管道没有消息边界，`write` 写入的内容可能被多次 `read` 读出。单次写入不超过 `PIPE_BUF` 时，在多写端场景下通常具有原子性；不要把它误解为完整的应用层消息协议。

## 2. 阻塞、EOF 与 SIGPIPE

- 读端无数据且仍有写端：`read` 默认阻塞。
- 所有写端关闭：`read` 返回 0，表示 EOF。
- 没有读端时写入：通常触发 `SIGPIPE`，忽略信号后 `write` 返回 `EPIPE`。
- `O_NONBLOCK` 可改为非阻塞，但调用方必须处理 `EAGAIN/EWOULDBLOCK`。

父子进程必须及时关闭自己不用的端点，否则内核仍认为存在读者或写者，导致另一端无法收到 EOF 或一直阻塞。

## 3. FIFO 命名管道

```bash
mkfifo /tmp/request.fifo
```

FIFO 在文件系统中有路径，互不相关的进程可以通过 `open/read/write` 通信。以读写方式打开时仍遵循管道的阻塞和 EOF 规则；程序退出时可用 `unlink` 删除名字。FIFO 适合简单的本机生产者—消费者模型，不适合需要复杂双向协议的服务。

## 4. 常见错误

只关闭一端、未检查 `read/write` 返回值、把字节流当成消息流、未处理信号中断（`EINTR`），都是管道程序最常见的问题。
