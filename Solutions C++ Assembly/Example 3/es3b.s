.global elab3

/* STACK
+7 . . . . . . . . +0
+-------------------+
|         XXX       | -32  <- rsp
+-------------------+
|    j     |   i    | -24
+-------------------+
|    n2    |    n1  | -16
+-------------------+
|         tot       | -8
+-------------------+
|    vecchio rbp    | <- rbp
+-------------------+
*/

.set tot, -8
.set n1, -16
.set n2, -12
.set i, -24
.set j, -20

elab3:

# prologo
pushq %rbp
movq %rsp, %rbp
subq $32, %rsp

# rdi = tot, esi = n1, edx = n2
movq %rdi, tot(%rbp)
movl %esi, n1(%rbp)
movl %edx, n2(%rbp)

# calcolo di i
movl n1(%rbp), %eax
addl n2(%rbp), %eax
movl %eax, i(%rbp)

# calcolo di j
movl n1(%rbp), %eax
subl n2(%rbp), %eax
movl %eax, j(%rbp)

# calcolo di tot
movl i(%rbp), %eax
imull j(%rbp), %eax
movq tot(%rbp), %rcx
movl %eax, (%rcx)

leave
ret
