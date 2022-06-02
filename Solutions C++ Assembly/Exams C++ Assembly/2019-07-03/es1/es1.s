/*  costruttore cl
    0                   7
    +-------------------+
  0 | vv1[4]  |    xxx  |
    +-------------------+
  8 |      vv2[0]       |
    +-------------------+
 16 |       . .         |
    +-------------------+
 24 |       . .         |
    +-------------------+
 32 |     vv2[3]        |
    +-------------------+
    stack cl
    +-------------------+
    |                   | -32
    +-------------------+
    |         |    i    | -24
    +-------------------+
    |        char v[]   | -16
    +-------------------+
    |        this       | -8
    +-------------------+
    |       old rbp     | <- rbp
    +-------------------+
*/

.set this, -8
.set v, -16
.set i, -24

.global _ZN2clC1EPc
_ZN2clC1EPc:
# prologo
pushq %rbp
movq %rsp, %rbp
subq $32, %rsp

# passaggio dei parametri
movq %rdi, this(%rbp)
movq %rsi, v(%rbp)

# ciclo for
movl $0, i(%rbp)
for_uno:
    cmpl $4, i(%rbp)
    jge fine_for_uno
    movslq i(%rbp), %rcx
    movq v(%rbp), %r9
    movb (%r9, %rcx, 1), %al
    movq this(%rbp), %r8
    movb %al, (%r8, %rcx, 1)
    movsbq %al, %rax
    movq %rax, 8(%r8, %rcx, 8)
    incl i(%rbp)
    jmp for_uno
fine_for_uno:

leave
ret

/*  stack elab1
    +-------------------+
    |                   | -32
    +-------------------+
    |     st& ss        | -24
    +-------------------+
    |    i     |     d  | -16
    +-------------------+
    |        this       | -8
    +-------------------+
    |       old rbp     | <- rbp
    +-------------------+
*/

.set this, -8
.set d, -16
.set i, -12
.set st, -24

.global _ZN2cl5elab1EiR2st
_ZN2cl5elab1EiR2st:
# prologo
pushq %rbp
movq %rsp, %rbp
subq $32, %rsp

# passaggio dei parametri
movq %rdi, this(%rbp) 
movl %esi, d(%rbp)
movq %rdx, st(%rbp)

# ciclo for
movl $0, i(%rbp)
for_due:
    cmpl $4, i(%rbp)
    jge fine_for_due
    movslq i(%rbp), %rcx
    movq st(%rbp), %r8
    movslq d(%rbp), %rax
    movq this(%rbp), %r9
    cmpq 8(%r8, %rcx, 8), %rax
    jl dopo_if
    movb (%r8, %rcx, 1), %bl
    addb %bl, (%r9, %rcx, 1)
dopo_if:
    movl d(%rbp), %ebx
    addl i(%rbp), %ebx
    movslq %ebx, %rbx
    movq %rbx, 8(%r9, %rcx, 8)
    incl i(%rbp)
    jmp for_due
fine_for_due:
leave
ret
