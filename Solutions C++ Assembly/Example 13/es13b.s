.global fstructr

/* STACK
+-----------------------+
|                       | -32 <- rsp
+-----------------------+
|            |     i    | -24
+-----------------------+
|          |c|     a    | -16
+-----------------------+
|          &st          | -8
+-----------------------+
|                       | <- rbp
+-----------------------+
*/

.set st, -8
.set a, -16
.set c, -12
.set i, -24

fstructr:

# prologo
pushq %rbp
movq %rsp, %rbp
subq $32, %rsp

# parametri
movq %rdi, st(%rbp)     
movl %esi, a(%rbp)      
movb %dl, c(%rbp)       # %dl = 8 bit meno significativi di %rdx

# st.n1 = a
movq st(%rbp), %rax
movl a(%rbp), %ebx
movl %ebx, (%rax)    # accedo a st.n1

#st.n2 = 2*a
movq st(%rbp), %rax
movl a(%rbp), %ebx
imull $2, %ebx
movl %ebx, 4(%rax)

# ciclo for
movl $0, i(%rbp)

for:
    cmpl $10, i(%rbp)
    jge fine_for
    # c + i
    movl i(%rbp), %eax
    movzbl c(%rbp), %ebx
    addl %ebx, %eax             # eax = c + i
    movslq i(%rbp), %rcx
    movq st(%rbp), %rdx
    movb %al, 8(%rdx,%rcx,1)    # devo accedere alla st puntata nel main rdx = st(%rbp)    
    incl i(%rbp)
    jmp for

fine_for:

leave
ret
