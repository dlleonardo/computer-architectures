.global elab2

.set n1, -8
.set n2, -4
.set i, -16
.set j, -12

/*
STACK
+7 . . . . . . +0
+---------------+
|   j   |   i   | -16  <- rsp
+---------------+
|   n2  |   n1  | -8
+---------------+
|  vecchio rbp  |  <- rbp
+---------------+
*/

elab2:

# prologo
pushq %rbp
movq %rsp, %rbp
subq $16, %rsp

/* rdi, rsi, rdx, rxc .. */
movl %edi, n1(%rbp)
movl %esi, n2(%rbp)

# calcolo di i
movl n1(%rbp), %eax
addl n2(%rbp), %eax     # eax = eax + n2
movl %eax, i(%rbp)

# calcolo di j
movl n1(%rbp), %eax
subl n2(%rbp), %eax     # eax = eax - n2
movl %eax, j(%rbp)

# valore di return
movl i(%rbp), %eax
imull j(%rbp), %eax     # i*j

leave
ret
