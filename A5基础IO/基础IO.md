# 一、理解文件

0KB的文件是占据空间的。**文件=内容+属性**，我们对文件的操作都是围绕着内容和属性展开的。

**狭义理解：**

- 文件在磁盘里。

- 磁盘是永久性存储介质，因此文件在磁盘上的存储是永久性的。
- 磁盘是外设（即是输出设备也是输入设备）。
- 磁盘上的文件本质是对文件的所有操作，都是对外设的输入和输出简称IO。

**广义理解：Linux一切皆文件。**

***

**系统角度：**

- 对文件的操作本质是进程对文件的操作。
- 磁盘的管理者是操作系统。
- 文件的读写本质不是通过C语言/C++的库函数来操作的（这些库函数只是为用户提供方便），而是通过文件相关的系统调⽤接口来实现的。
- 操作系统一定会提供系统调用，使用户能够操作磁盘硬件。`fopen`、`fclose`等等都是封装了底层OS的系统调用。

访问文件，需要先打开文件。打开文件的是进程。==**对文件的操作，本质是进程对文件的操作。**==

# 二、C语言的文件

C会默认打开三个输入输出流，分别是`stdin`、`stdout`、`stderr`。

- `stdin`：标准输入，键盘文件。
- `stdout`：标准输出，显示器文件。
- `stderr`：标准错误，显示器文件。

```c
#include <stdio.h>

extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;
```

***

**文件打开方式：**

```
r Open text file for reading.
The stream is positioned at the beginning of the file.
    
r+ Open for reading and writing.
The stream is positioned at the beginning of the file.
    
w Truncate(缩短) file to zero length or create text file for writing.
The stream is positioned at the beginning of the file.

w+ Open for reading and writing.
The file is created if it does not exist, otherwise it is truncated.
The stream is positioned at the beginning of the file.

a Open for appending (writing at end of file).
The file is created if it does not exist.
The stream is positioned at the end of the file.

a+ Open for reading and appending (writing at end of file).
The file is created if it does not exist. The initial file position
for reading is at the beginning of the file,
but output is always appended to the end of the file.
```

# 三、系统IO

```c
int open(const char *pathname, int flags);
int open(const char *pathname, int flags, mode_t mode);
```

打开可能或创建一个文件。

`flags`可以为宏：`O_RDONLY`（读）、`O_WRONLY`（写）、`O_RDWR`（读写）、`O_CREAT`（不存在就新建）等等。

当`flag`带上`O_TRUNC`（清空）时，打开文件内容会清空，不带它时就不会清空。`O_APPEND`为追加内容。

如果不带第三个参数权限位，创建文件时的权限是乱码。

**`mode_t umask(mode_t mask);`**设置权限掩码。如果不设置，就是系统默认。

**`close(fd)`**关闭文件，`fd`为`open`的返回值。

**`ssize_t write(int fd, const void *buf, size_t count);`**写入内容。

注意，写入内容为字符串时，`count`为`strlen`，不能`+1`。

对于`write`，系统不关心写入的类型，当写入类型为其他类型时，需要先格式化为字符串才能写入，此格式化由语言提供或自己实现。

**`ssize_t read(int fd, void *buf, size_t count);`**读数据。读取成功返回文件大小，失败返回小于0。

***

打开成功时，`open`的返回值`fd`一定大于等于0。**其中`fd`0、1、2三者为标准输入、标准输出、标准错误。**

再看`FILE *fopen(const char *pathname, const char *mode);`，其中，`FILE`指什么呢？

`FILE`是C语言中定义的一个结构体，由`typedef`而来。

无论是什么语言，在OS接口中，OS只认`fd`，即文件描述符。因此，`FILE`中一定封装了`fd`，它就是`fileno`。

为什么这样设计`FILE`呢？

`FILE`中封装了各个系统的文件标识符，根据平台条件编译，使代码具备可移植性。