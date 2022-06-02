/*
    classe cl
    0              7
    +---------------+
  0 |  st1  |       |
    +---------------+
  8 |      v[0]     |
    +---------------+
 16 |      . .      |
    +---------------+
 24 |      . .      |
    +---------------+
 32 |      v[3]     |
    +---------------+

    stack costruttore cl
    7               0
    +---------------+
    |   .  . |st2[0]| -32
    +---------------+
    | st2[3] | . .  | -24
    +---------------+
    |   i    |    |c| -16
    +---------------+
    |      this     | -8
    +---------------+
    |    old rbp    | <- rbp
    +---------------+
*/

.set this, -8
.set c, -16
.set i, -12
.set st2, -32

.global _ZN2clC1Ec3st2
_ZN2clC1Ec3st2:

# prologo
pushq %rbp
movq %rsp, %rbp
subq $32, %rsp

# passaggio parametri
movq %rdi, this(%rbp)
movb %sil, c(%rbp)
movq %rdx, st2(%rbp)
movq %rcx, st2+8(%rbp)

# ciclo for uno
movl $0, i(%rbp)
for_uno:
    cmpl $4, i(%rbp)
    jge fine_for_uno

    movslq i(%rbp), %rcx
    movq this(%rbp), %r8
    movb c(%rbp), %al
    movb %al, (%r8, %rcx, 1)
    # v[i] = s2.vd[i] - s.vc[i]
    movl st2(%rbp, %rcx, 4), %ebx
    movsbl %al, %eax
    subl %eax, %ebx
    movslq %ebx, %rbx
    movq %rbx, 8(%r8, %rcx, 8)
    incl i(%rbp)
    jmp for_uno
fine_for_uno:

leave
ret

/*
   stack elab1
   +-----------------+
   | xxx   | cla st1 | -64
   +-----------------+
   |      cla v[0]   | -56
   +-----------------+
   |       . .       | -48
   +-----------------+
   |       . .       | -40
   +-----------------+
   |     cla v[3]    | -32
   +-----------------+
   |     st2&        | -24
   +-----------------+
   |        |  st1   | -16
   +-----------------+
   |       this      | -8
   +-----------------+
   |     old rbp     | <- rbp
   +-----------------+
*/

.set this, -8
.set st1, -16
.set st2, -24
.set cla, -64

.global _ZN2cl5elab1E3st1R3st2
_ZN2cl5elab1E3st1R3st2:
# prologo
pushq %rbp
movq %rsp, %rbp
subq $64, %rsp

# passaggio parametri
movq %rdi, this(%rbp)
movl %esi, st1(%rbp)
movq %rdx, st2(%rbp)

# chiamata al costruttore
leaq cla(%rbp), %rdi
movb $'f', %sil
movq st2(%rbp), %r8
movq (%r8), %rdx
movq 8(%r8), %rcx
call _ZN2clC1Ec3st2

# for due
movl $0, i(%rbp)
for_due:
   cmpl $4, i(%rbp)
   jge fine_for_due

   movslq i(%rbp), %rcx
   movb st1(%rbp), %al
   movq this(%rbp), %r8
   # s1.vc[i] >= s.vc[i]
   cmpb %al, (%r8, %rcx, 1)
   jge dopo_if_uno
   # s.vc[i] = cla.s.vc[i]
   movb cla(%rbp, %rcx, 1), %bl
   movb %bl, (%r8, %rcx, 1)
   
dopo_if_uno:
   movq cla+8(%rbp, %rcx, 8), %rax
   # cla.v[i] > v[i]
   cmpq %rax, 8(%r8, %rcx, 8)
   jg dopo_if_due
   # v[i] = v[i] + cla.v[i]
   addq 8(%r8, %rcx, 8), %rax
   movq %rax, 8(%r8, %rcx, 8)
dopo_if_due:
   incl i(%rbp)
   jmp for_due
fine_for_due:

leave
ret
