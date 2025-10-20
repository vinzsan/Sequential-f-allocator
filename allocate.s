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

# struct metadata same asf union { struct anon {} };

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
  cmp rbx,0
  je src_block_free.IF2

src_block_free.FOR1:

  xor rcx,rcx
  cmp [rbx + NODE_T_IS_FREE],rcx
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
  push r12

  mov rsi,0xF # align ((x + 0xF) & ~0xF)
  not rsi
  add rdi,15
  and rdi,rsi

  mov r12,rdi

  call src_block_free

  mov rbx,rax

  cmp rbx,0
  jne expand_metadata.IF1

  mov rdi,r12
  add rdi,NODE_T_SIZEOF
  call expand_brk

  mov r11,rax

  test rax,rax
  jne expand_metadata.IF2
  
  mov rdi,NODE_T_SIZEOF
  call expand_brk

  mov rcx,rax

  mov [rcx + NODE_T_SIZE],r12
  xor rax,rax
  mov [rcx + NODE_T_IS_FREE],rax
  xor rax,rax
  mov [rcx + NODE_T_NEXT],rax

  mov rax,[rip + HEAD_METADATA]
  cmp rax,0
  je expand_metadata.IF3

  mov [rip + TAIL_METADATA + NODE_T_NEXT],rcx
  mov [rip + TAIL_METADATA],rcx

  mov rax,[rcx + NODE_T_SIZEOF]

  leave
  ret

expand_metadata.IF1:

  mov rax,0
  mov [rbx + NODE_T_IS_FREE],rax

  mov rax,[rbx + NODE_T_SIZEOF]
  leave

  ret

expand_metadata.IF2:

  xor rax,rax
  leave

  ret

expand_metadata.IF3:

  mov rax,[rcx + NODE_T_SIZEOF]
  leave

  ret


_start:

  xor rbp,rbp
  and rsp,-16
  push rbp
  mov rbp,rsp
  sub rsp,32

  xor rax,rax

  mov QWORD PTR [rip + HEAD_METADATA],rax
  mov QWORD PTR [rip + TAIL_METADATA],rax

  mov rdi,1024
  call expand_metadata

  mov rdi,32
  call expand_metadata

  mov rdi,1024
  call expand_metadata

  mov rdi,128
  call expand_metadata

  leave
  mov rax,60
  xor rdi,rdi
  syscall
