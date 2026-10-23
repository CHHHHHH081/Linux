# Linux Notes

English | [简体中文](README.md)

![Notes](https://img.shields.io/badge/notes-Chinese-blue)
![Language](https://img.shields.io/badge/code-C-00599C?logo=c&logoColor=white)
![Platform](https://img.shields.io/badge/platform-Linux-FCC624?logo=linux&logoColor=black)

Notes written while learning Linux system programming — from everyday topics like permissions and tooling, through processes, basic I/O, file systems, signals and threads, to the networking part that is still being expanded.

Each chapter is a standalone Markdown note with diagrams and runnable C snippets. Example code lives next to its chapter, most of it with a `Makefile`, so `make` is usually all it takes. Images are kept in each chapter's `assets/` directory and referenced by relative paths, so the notes render offline once cloned.

> These are personal study notes mixed with course material, written in a fairly casual tone; a few conclusions may be imprecise. Corrections via Issues or PRs are welcome.

**The notes themselves are written in Chinese.** This file is the English version of the repository overview.

## Repository layout

```text
Linux/
├── 系统/
│   ├── 1权限/
│   ├── 2开发工具/
│   │   └── ProgressBar/
│   ├── 3进程/
│   │   ├── ALLENV/
│   │   ├── ONLY/
│   │   └── zombie/
│   ├── 4进程控制/
│   │   └── testforexe/
│   ├── 5基础IO/
│   ├── 6Ext系列文件系统/
│   ├── 7库的制作与原理/
│   │   ├── main/
│   │   └── testelf/
│   ├── 8进程间通信/
│   ├── 9Linux进程信号/
│   ├── 10线程概念及控制/
│   └── 11线程的同步与互斥/
├── 网络/
│   ├── 1网络基础/
│   ├── 2UDPSocket/
│   └── 3TCPSocket/
├── README.md
└── README.en.md
```

Each chapter directory holds one Markdown note; most also have an `assets/` folder for images, and a few contain example-code subdirectories. Chapter contents and code descriptions are in the tables below.

## Index

### System programming

| # | Note | Contents |
| --- | --- | --- |
| 1 | [权限 (Permissions)](系统/1权限/权限.md) | `sudo` and `/etc/sudoers`, users and permission bits, `chmod`/`chown`/`chgrp`, default permissions and `umask`, directory permissions and the sticky bit |
| 2 | [开发工具 (Tooling)](系统/2开发工具/开发工具.md) | Package managers, `vim`, `gcc`/`g++`, static vs. dynamic libraries, `make`/`Makefile` rules and variables, the progress-bar program |
| 3 | [进程概念 (Processes)](系统/3进程/进程概念.md) | von Neumann architecture and the memory hierarchy, where the OS fits, process states/priority/switching, the O(1) scheduler, environment variables, address space and page tables |
| 4 | [进程控制 (Process control)](系统/4进程控制/进程控制.md) | `fork` and copy-on-write, `exit`/`_exit`, `wait`/`waitpid` and exit status, program replacement with the `exec` family |
| 5 | [基础 IO (Basic I/O)](系统/5基础IO/基础IO.md) | Files and file descriptors, C stdio vs. system calls, how redirection works, "everything is a file", user-space buffers |
| 6 | [Ext 系列文件系统 (Ext filesystems)](系统/6Ext系列文件系统/Ext系列文件系统.md) | Disk geometry, CHS vs. LBA, blocks and block groups, inodes, directories and dentries, mounting, hard and soft links |
| 7 | [库的制作与原理 (Libraries)](系统/7库的制作与原理/库的制作与原理.md) | Building and using static/dynamic libraries, object files, the ELF format, static vs. dynamic linking, library loading |
| 8 | [进程间通信 (IPC)](系统/8进程间通信/进程间通信.md) | Anonymous pipes and process pools, FIFOs, System V shared memory/message queues/semaphores, how the kernel organises IPC objects. Also see the [review outline](系统/8进程间通信/00_复习提纲.md) and four quick-reference notes: [overview](系统/8进程间通信/01_IPC总览.md), [pipes & FIFO](系统/8进程间通信/02_管道与FIFO.md), [message queues & shared memory](系统/8进程间通信/03_消息队列与共享内存.md), [choosing an approach](系统/8进程间通信/04_选择与实践.md) |
| 9 | [进程信号 (Signals)](系统/9Linux进程信号/进程信号.md) | How signals are generated (keyboard/syscalls/hardware exceptions/software conditions), pending and blocked sets with `sigset_t`, the delivery path and user/kernel mode, reentrant functions, `volatile`, `SIGCHLD` |
| 10 | [线程概念及控制 (Threads)](系统/10线程概念及控制/线程概念及控制.md) | Paging and page tables, TLB and page faults, threads vs. processes, the POSIX thread API, LWP and TCB, object-oriented thread wrappers, thread-local storage |
| 11 | [线程的同步与互斥 (Synchronisation)](系统/11线程的同步与互斥/线程的同步与互斥.md) | Mutexes and how locking works at the hardware level, RAII-style wrappers, condition variables, producer-consumer, POSIX semaphores and ring buffers, thread pools |

### Networking

| # | Note | Contents | Status |
| --- | --- | --- | --- |
| 1 | [网络基础 (Networking basics)](网络/1网络基础/网络基础.md) | Protocol layering, OSI and TCP/IP models, LAN and cross-network delivery, MAC vs. IP addresses, port numbers, TCP and UDP characteristics, network byte order, `sockaddr` structures | Done |
| 2 | [UDP Socket](网络/2UDPSocket/UDPSocket.md) | `socket`/`bind`/`recvfrom`/`sendto`, filling in `sockaddr_in`, full server and client flow, implicit client bind, helpers such as `inet_ntoa` | Done |
| 3 | [TCP Socket](网络/3TCPSocket/TCPSocket.md) | `socket`/`bind`/`listen`/`accept`, listening and connection states, full-duplex byte-stream I/O | In progress |

## Example code

| Directory | What it shows | Build & run |
| --- | --- | --- |
| [ProgressBar](系统/2开发工具/ProgressBar) | A progress bar refreshed with `\r`, illustrating line buffering | `make && ./progressbar` |
| [ALLENV](系统/3进程/ALLENV) | Walking the third `main` argument to print every environment variable | `make && ./allenv` |
| [ONLY](系统/3进程/ONLY) | Reading and checking the `USER` variable with `getenv` | `make && ./OnlyMe` |
| [zombie](系统/3进程/zombie) | A child exits while the parent never reaps it, leaving a zombie | `gcc test.c -o test && ./test`, then watch with `ps` in another terminal |
| [testforexe](系统/4进程控制/testforexe) | A child replaces its image with `execl` to run `ls`; the parent waits | `make && ./test` |
| [main](系统/7库的制作与原理/main) | Creating a static library with `ar` and installing its headers | `make` produces `libfile.a`; link with `gcc main.c -Icfiles -L. -lfile -o main` |
| [testelf](系统/7库的制作与原理/testelf) | Hand-running `.c → .s → .o → executable`, then inspecting ELF and address space | No `Makefile`; follow the commands in the note |

> Threading examples need the POSIX threads library: `gcc demo.c -o demo -lpthread` (`-pthread` also works on recent glibc).

## Getting started

```bash
git clone https://github.com/CHHHHHH081/Linux.git
cd Linux
```

Open the repository in any Markdown-capable editor — the VS Code Markdown preview works well. A C/C++ IntelliSense config is included at [网络/.vscode/c_cpp_properties.json](网络/.vscode/c_cpp_properties.json); it points at `/usr/bin/gcc`, so adjust it if your toolchain lives elsewhere.

## Environment

- **OS**: Linux (the commands and system calls used here are distribution-agnostic; Ubuntu/CentOS both fine)
- **Compiler**: `gcc` / `g++`; examples are mostly C, and the IPC and thread-wrapper chapters use C++ classes
- **Build**: `make` (the `Makefile`s use `wildcard`, `%` pattern rules, `$@` / `$^` / `$<`)
- **Observation tools**: `gdb`, `ps`, `top`, `netstat`, `strace`
- **Threads**: POSIX Threads (`-lpthread`)

## How to read

1. Go through the system chapters in order from `1` to `11` — later chapters lean on earlier material (the thread chapters reuse the address-space and page-table discussion a lot). For a single topic, start with that chapter's table of contents.
2. The process, filesystem and signal chapters take the kernel's point of view; run the sample code alongside them and watch `ps`, `strace` and `/proc`.
3. For networking, read [networking basics](网络/1网络基础/网络基础.md) first to get layering, MAC/IP/port and byte order straight, then move on to the UDP and TCP APIs.
4. Chapter 8's directory also holds a [review outline](系统/8进程间通信/00_复习提纲.md) that works well as a self-test checklist.

## Roadmap

- [ ] Finish the networking part: cover TCP `connect`, send/receive and the server loop, then add multi-process/multi-thread servers and protocol design
- [ ] Fix the 17 images in [进程概念](系统/3进程/进程概念.md) that still point at an absolute local path (`D:/我的文档/...`); move them into `assets/` so they render on GitHub
- [ ] Remove committed build artefacts (`*.o`, `*.a`, `*.tgz`) and add matching rules to `.gitignore`
- [ ] Unify formatting: heading levels, code-block language tags, image captions
- [ ] Add an open-source license

## Notes

This repository is for study and discussion only. No license has been declared yet; if you quote or republish the notes, please credit the source.

Issues pointing out mistakes are welcome, and so are PRs.
