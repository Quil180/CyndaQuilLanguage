global _start
_start:
  mov rax, 2
  push rax
  mov rax, 2
  push rax
  push QWORD [rsp + 8]
  pop rax
  pop rbx
  sub rax, rbx
  push rax
  pop rax
  test rax, rax
  jz label0
  mov rax, 52
  push rax
  mov rax, 60
  pop rdi
  syscall
  add rsp, 0
label0:
  mov rax, 1
  push rax
  mov rax, 60
  pop rdi
  syscall
  mov rax, 60
  mov rdi, 0
  syscall
