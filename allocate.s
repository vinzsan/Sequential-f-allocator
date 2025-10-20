.code64
.intel_syntax noprefix

.section .data
  
  HEAD: .quad 0
  TAIL: .quad 0

.set NODE_T_SIZE, 0
.set NODE_T_IS_FREE, 8 
.set NODE_T_NEXT, 16 
# optional buat sizeof
.set NODE_T_SIZEOF, 24

.align 16 

# struct metadata

  HEAD_METADATA: .quad 0 
  TAIL_METADATA: .quad 0

.align 16 

NODE_T:
  .quad 0
  .long 0
  .long 0 # padding
  .quad 0 # Next struct / neighbours

.section .text
.global _start

expand_brk:
  
  push rbp
  mov rbp,rsp
  sub rsp,16 

  mov r12,rdi

  xor rax,rax
  cmp QWORD ptr [rip + HEAD],rax
  je expand_brk.NULL

  xor rbx,rbx
  mov rbx,QWORD ptr [rip + TAIL]
  add QWORD ptr [rip + TAIL],r12

  mov rax,12 
  mov rdi,QWORD ptr [rip + TAIL]
  syscall

  cmp rax,-1 
  je expand_brk.ERR
  
  mov rax,rbx
  leave
  ret

expand_brk.NULL:

  mov rax,12
  xor rdi,rdi
  syscall

  cmp rax,-1 
  je expand_brk.ERR

  mov QWORD ptr [rip + HEAD],rax

  add rax,r12
  mov QWORD ptr [rip + TAIL],rax
  
  mov rax,12 
  mov rdi,QWORD ptr [rip + TAIL]
  syscall
  leave

  ret

expand_brk.ERR:

  xor rax,rax
  leave

  ret

#----------------------------------#

src_block_free:

  push rbp
  mov rbp,rsp
  sub rsp,32
  push r13
  push rbx
  push rcx

  mov r13,rdi

.align 16 

  mov rbx,[rip + HEAD_METADATA]

src_block_free.FOR1:

  cmp [rbx + NODE_T_IS_FREE],1
  je src_block_free.IF1
  cmp [rbx + NODE_T_SIZE],r13
  jnb src_block_free.IF1

  mov rbx,[rbx + NODE_T_NEXT]

  cmp rbx,0
  je src_block_free.IF2

  jmp src_block_free.FOR1

src_block_free.IF1:

  mov rax,rbx

  pop r13
  pop rbx
  pop rcx

  leave
  ret

src_block_free.IF2:

  mov rax,0

  pop r13
  pop rbx
  pop rcx

  leave
  ret

expand_metadata:

  push rbp
  mov rbp,rsp
  sub rsp,16 

  push rbx
  push rcx
  push r11

  mov rsi,0xF # align ((x + 0xF) & ~0xF)
  not rsi
  add rdi,15
  and rdi,rsi

  mov rdi,rdi
  call src_block_free

  sub rsp,NODE_T_SIZEOF
  lea rbx,[rsp]

  mov rbx,rax

  cmp rbx,0
  jg 

  mov [rbx + NODE_T_IS_FREE],0

  leave
  ret

  cmp rbx,0
  jg expand_metadata.IF1

expand_metadata.IF1:

  cmp rbx,


_start:

  xor rbp,rbp
  and rsp,-16
  push rbp
  mov rbp,rsp
  sub rsp,32

  mov rdi,1024
  call expand_brk

  mov rdi,32
  call expand_brk

  leave
  mov rax,60
  xor rdi,rdi
  syscall
