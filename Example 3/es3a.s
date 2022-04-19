.include "./../servi2.s"
.global main

/* STACK
+--------------------+
|    b     |    a    |  -16  <- rsp
+--------------------+
|         ris        |  -8
+--------------------+
|      vecchio rbp   |  <- rbp
+--------------------+
*/

.set ris, -8
.set a, -16
.set b, -12

.text
main:

# prologo
pushq %rbp
movq %rsp, %rbp
subq $16, %rsp

# rdi, rsi, rdx
# lettura di a
call leggiint
movl %eax, a(%rbp)

#lettura di b
call leggiint
movl %eax, b(%rbp)

# chiamata a elab3
leaq ris(%rbp), %rdi    # indirizzo di ris, viene passato per &
movl a(%rbp), %esi
movl b(%rbp), %edx
call elab3

# chiamata a scriviint
mov (%rdi), %edi
call scriviint
call nuovalinea

leave
ret
