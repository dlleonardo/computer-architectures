/*  classe cl
     0 . . . . . . . . 7
    +-------------------+
   0| st1.vc  |         |
    +-------------------+
   8|    long v[0]      |
    +-------------------+
  16|       . .         |
    +-------------------+
  24|       . .         |
    +-------------------+
  32|    long v[3]      |
    +-------------------+

    stack costruttore cl
    +7 . . . . . . . . 0
    +-------------------+
    |     i    |      |c| -24
    +-------------------+
    |       st2& s2     | -16
    +-------------------+
    |       this        | -8
    +-------------------+
    |       old rbp     | <- rbp
    +-------------------+
*/

.set this, -8
.set st2, -16
.set c, -24
.set i, -20

.global _ZN2clC1EcR3st2
_ZN2clC1EcR3st2:

# prologo
pushq %rbp
movq %rsp, %rbp
subq $32, %rsp

# passaggio parametri
movq %rdi, this(%rbp)
movb %sil, c(%rbp)
movq %rdx, st2(%rbp)

# ciclo for
movl $0, i(%rbp)
for_uno:
    cmpl $4, i(%rbp)
    jge fine_for_uno

    movslq i(%rbp), %rcx
    movsbl c(%rbp), %ebx
    addl i(%rbp), %ebx          # rbx = c + i
    movq this(%rbp), %r8
    movb %bl, (%r8, %rcx, 1)    # s.vc[i] = rbx

    movq st2(%rbp), %r9
    movq this(%rbp), %r8
    movb (%r8, %rcx, 1), %bl    # s.vc[i]
    movsbl %bl, %ebx
    addl (%r9, %rcx, 4), %ebx   # ebx = s2.vd[i] + s.vc[i]
    movslq %ebx, %rbx
    movq %rbx, 8(%r8, %rcx, 8)

    incl i(%rbp)
    jmp for_uno

fine_for_uno:

leave
ret

/*  stack elab1
    7 . . . . . . . . . 0
    +-------------------+
    |         |cla.st1  | -72
    +-------------------+
    |   cla.long v[0]   | -64
    +-------------------+
    |        . .        | -56
    +-------------------+
    |        . .        | -48
    +-------------------+
    |   cla.long v[3]   | -40
    +-------------------+
    |   . .   |  st2[0] | -32
    +-------------------+
    | st2[3]  |   . .   | -24
    +-------------------+
    |     i   |    st1  | -16
    +-------------------+
    |       this        | -8
    +-------------------+
    |       old rbp     | <-rbp
    +-------------------+
*/

.set this, -8
.set st1, -16
.set i, -12
.set st2, -32
.set cla, -72

.global _ZN2cl5elab1E3st13st2
_ZN2cl5elab1E3st13st2:
# prologo
pushq %rbp
movq %rsp, %rbp
subq $80, %rsp

# passaggio parametri
movq %rdi, this(%rbp)
movl %esi, st1(%rbp)
movq %rdx, st2(%rbp)
movq %rcx, st2+8(%rbp)

# chiamata al costruttore cl
leaq cla(%rbp), %rdi
movb $'a', %sil
leaq st2(%rbp), %rdx
call _ZN2clC1EcR3st2

# ciclo for
movl $0, i(%rbp)
for_due:
    cmpl $4, i(%rbp)
    jge fine_for_due

    movslq i(%rbp), %rcx
    movq this(%rbp), %r8
    movb (%r8, %rcx, 1), %al
    cmpb st1(%rbp, %rcx, 1), %al    # al = s.vc[i]
    jg dopo_if
    movb cla(%rbp, %rcx, 1), %bl    # bl = cla.s.vc[i]
    movq this(%rbp), %r9
    movb %bl, (%r9, %rcx, 1)        # s.vc[i] = cla.s.vc[i]
    movq cla+8(%rbp, %rcx, 8), %rdx
    movq %rdx, 8(%r9, %rcx, 8)

dopo_if:
    incl i(%rbp)
    jmp for_due

fine_for_due:

leave
ret
