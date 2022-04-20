.include "./../servi2.s"

.global main

/* STACK
+7 . . . . . . . . +0
+-------------------+
|                   | -24
+-------------------+
|        pun        | -16 <- rsp
+-------------------+
|   n      |   m    | -8
+-------------------+
|                   | <- rbp
+-------------------+
*/

.set m, -8
.set n, -4
.set pun, -16

.text
main:

# prologo
pushq %rbp
movq %rsp, %rbp
subq $16, %rsp

call leggiint
movl %eax, n(%rbp)
call leggiint
movl %eax, m(%rbp)

leaq pun(%rbp), %rdi    # indirizzo pun
leaq n(%rbp), %rsi      # indirizzo di n
leaq m(%rbp), %rdx      # indirizzo di m
call trovamin

movq pun(%rbp), %rax    # *pun sarebbe il contenuto puntato da pun
movl (%rax), %edi       # accesso indiretto al contenuto, %edi = contenuto puntato da pun
call scriviint
call nuovalinea

leave
ret
