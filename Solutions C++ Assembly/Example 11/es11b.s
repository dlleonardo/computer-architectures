.global fair

/* STACK
+-------------------+
|                   | -32 <- rsp
+-------------------+
|         |  ss.n2  | -24
+-------------------+
|    |ss.c|  ss.n1  | -16
+-------------------+
|        &ss        | -8
+-------------------+
|       old rbp     | <- rbp
+-------------------+
*/

.set ss, -8

fair:

# prologo
pushq %rbp
movq %rsp, %rbp
subq $32, %rsp

# passaggio parametri
movq %rdi, ss(%rbp)

# ss.n1 = ss.n1 + 5;
movq ss(%rbp), %rax     # rax = indirizzo di ss
addl $5, (%rax)         # contenuto di rax + 5

# ss.c = ss.c + 1
addb $1, 4(%rax)        # rax+4 = accedo al contenuto di ss.c

# ss.n2 = ss.n2 + 10
addl $10, -8(%rax)       #rax+8 = accedo al contenuto di ss.n2

leave
ret
