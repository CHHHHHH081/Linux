
hello.o:     file format elf64-x86-64


Disassembly of section .text:

0000000000000000 <main>:
   0:	f3 0f 1e fa          	endbr64 
   4:	55                   	push   %rbp
   5:	48 89 e5             	mov    %rsp,%rbp
   8:	48 8d 05 00 00 00 00 	lea    0x0(%rip),%rax        # f <main+0xf>
   f:	48 89 c7             	mov    %rax,%rdi
  12:	e8 00 00 00 00       	call   17 <main+0x17>
  17:	b8 00 00 00 00       	mov    $0x0,%eax
  1c:	e8 00 00 00 00       	call   21 <main+0x21>
  21:	b8 00 00 00 00       	mov    $0x0,%eax
  26:	5d                   	pop    %rbp
  27:	c3                   	ret    
