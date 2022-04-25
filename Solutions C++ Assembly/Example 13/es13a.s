.include "./../servi2.s"
/* STACK scriviris
+---------------------------+
|                           | -32
+---------------------------+
|                           | -24
+---------------------------+
|              |      i     | -16
+---------------------------+
|             &ss           | -8
+---------------------------+
|                           | <- rbp
+---------------------------+
*/

.text

.set ss, -8
.set i, -16

scriviris:

# prologo
pushq %rbp
movq %rsp, %rbp
subq $16, %rsp

# parametri
movq %rdi, ss(%rbp)

# scriviint(ss.n1)
movq ss(%rbp), %rax
movl (%rax), %edi
call scriviint

# scriviint(ss.n2)
movq ss(%rbp), %rax
movl 4(%rax), %edi
call scriviint

# ciclo for
movq $0, i(%rbp)

for:
    cmpl $10, i(%rbp)
    jge fine_for
    movq ss(%rbp), %rax
    movslq i(%rbp), %rcx
    movb 8(%rax, %rcx, 1), %dil     # 8 bit meno significativi di %rdi
    call scrivichar
    incl i(%rbp)
    jmp for

fine_for:
    jmp nuovalinea
    leave
    ret

.global main

/* STACK main
+---------------------------+
|                 |a[9]| . .| -24
+---------------------------+
|a[7]|  .    .     .   |a[0]| -16
+---------------------------+
|   sa.n2     |     sa.n1   | -8
+---------------------------+
|                           | <- rbp
+---------------------------+
*/

.set sa, -8

main:

# prologo
pushq %rbp
movq %rsp, %rbp
subq $32, %rsp

# fstructr(sa, 5, 'a')
leaq sa(%rbp), %rdi
movl $5, %esi
movb $'a', %dl
call fstructr

# scriviris(sa)
leaq sa(%rbp), %rdi
call scriviris

leave
ret
