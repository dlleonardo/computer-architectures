.global fai

/* STACK
+7 . . . . . . . . +0
+-------------------+
|         |  ss.n2  | -32 <- rsp
+-------------------+
|    |ss.c|   ss.n1 | -24
+-------------------+
|         |  st.n2  | -16
+-------------------+
|    |st.c|   st.n1 | -8
+-------------------+
|      old rbp      | <- rbp
+-------------------+
*/

.set st, -8
.set ss, -24

fai:

# prologo
pushq %rbp
movq %rsp, %rbp
subq $32, %rsp

# passaggio parametri
movq %rdi, st(%rbp)
movl %esi, st-8(%rbp)

# ss.n1 = st.n1 + 5
movl st(%rbp), %eax
addl $5, %eax
movl %eax, ss(%rbp)

# ss.c = st.c + 1
movb st+4(%rbp), %al
addb $1, %al
movb %al, ss+4(%rbp)

# ss.n2 = st.n2 + 10
movl st-8(%rbp), %eax
addl $10, %eax
movl %eax, ss-8(%rbp)

# passaggio dei parametri
movq ss(%rbp), %rax
movl ss-8(%rbp), %edx

leave
ret
