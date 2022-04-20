.global raddoppia

/* STACK
+7 . . . . . . +0
+---------------+
|               | -24
+---------------+
|   i   |   n   | -16
+---------------+
|       a       | -8
+---------------+
|               | <- rbp
+---------------+
*/

.set a, -8
.set n, -16
.set i, -12

raddoppia:

# prologo
pushq %rbp
movq %rsp, %rbp
sub $16, %rsp

# rdi, rsi, rdx
movq %rdi, a(%rbp)
movl %esi, n(%rbp)
movl $0, i(%rbp)

# inizio ciclo for
for:
    movl n(%rbp), %eax
    cmpl %eax, i(%rbp)      # i < n
    jge fine_ciclo          # salta se i >= n

    movslq i(%rbp), %rcx    # estensione di segno da long a quad, mi serve per accedere ad a[i]

    movl (%rdi, %rcx, 4), %ebx      # rdi = a(%rbp), rdi + rcx*4
    imull $2, %ebx
    movl %ebx, (%rdi, %rcx, 4)
    incl i(%rbp)
    jmp for

fine_ciclo:
    leave
    ret
