
code.o:     file format elf64-x86-64


Disassembly of section .text:

0000000000000000 <run>:
   0:	f3 0f 1e fa          	endbr64 
   4:	55                   	push   %rbp
   5:	48 89 e5             	mov    %rsp,%rbp
   8:	48 8d 05 00 00 00 00 	lea    0x0(%rip),%rax        # f <run+0xf>
   f:	48 89 c7             	mov    %rax,%rdi
  12:	e8 00 00 00 00       	call   17 <run+0x17>
  17:	90                   	nop
  18:	5d                   	pop    %rbp
  19:	c3                   	ret    
