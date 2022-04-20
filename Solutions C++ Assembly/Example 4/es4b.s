.global add

/* STACK
+7 . . . . . . . . . . +0
+-----------------------+
|          |      i     |  -16 <- rsp
+-----------------------+
|          p            |  -8
+-----------------------+
|    vecchio rbp        |  <- rbp
+-----------------------+
*/

.set p, -8
.set i, -16

add:

# prologo
pushq %rbp
movq %rsp, %rbp
subq $16, %rsp

// rdi, rsi, rdx
movq %rdi, p(%rbp)
movl %esi, i(%rbp)

movq p(%rbp), %rax      # rax = indirizzo dell'intero da modificare
movl i(%rbp), %ebx
addl %ebx, (%rax)       # (%rax) accesso indiretto all'indirizzo

leave
ret
