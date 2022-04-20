.include "./../servi2.s"
.global main

/* STACK
+7 . . . . . . +0
+---------------+
|               | -32
+---------------+
|               | -24
+---------------+
|               | -16 <- %rsp
+---------------+
|   a   |   b   | -8
+---------------+
|    old rbp    | <- rbp
+---------------+
*/

.set b, -8
.set a, -4

.text
main:

# prologo
pushq %rbp
movq %rsp, %rbp
subq $16, %rsp

# rdi, rsi, rdx
call leggiint
movl %eax, a(%rbp)

call leggiint
movl %eax, b(%rbp)

# passaggio dei parametri a add()
leaq a(%rbp), %rdi
movl b(%rbp), %esi
call add

movl a(%rbp), %edi
call scriviint
call nuovalinea

leave
ret
