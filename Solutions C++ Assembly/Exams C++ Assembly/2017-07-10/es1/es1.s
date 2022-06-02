/*  costruttore cl
    0                   7
    +-------------------+
  0 |  v1[4]  |  v2[4]  |
    +-------------------+
  8 |    long v3[0]     |
    +-------------------+
 16 |       . .         |
    +-------------------+
 24 |       . .         |
    +-------------------+
 32 |    long v3[3]     |
    +-------------------+
    stack cl
    7                   0    
    +-------------------+
    |                   | -24
    +-------------------+
    |     i   |  st1 ss | -16
    +-------------------+
    |       this        | -8
    +-------------------+
    |      old rbp      | <- rbp
    +-------------------+
*/

.set this, -8
.set st1, -16
.set i, -12

.global _ZN2clC1E3st1
_ZN2clC1E3st1:
# prologo
pushq %rbp
movq %rsp, %rbp
subq $16, %rsp

# passaggio dei parametri
movq %rdi, this(%rbp)
movl %esi, st1(%rbp)

# ciclo for
movl $0, i(%rbp)
for_uno:
   cmpl $4, i(%rbp)
   jge fine_for_uno
   movslq i(%rbp), %rcx

   movb st1(%rbp, %rcx, 1), %al     # ss.vi[i]
   movq this(%rbp), %r8
   movb %al, (%r8, %rcx, 1)
   movb %al, 4(%r8, %rcx, 1)

   salb $1, %al
   movsbq %al, %rax
   movq %rax, 8(%r8, %rcx, 8)

   incl i(%rbp)
   jmp for_uno
fine_for_uno:

leave
ret

/* costruttore cl2
   0                    7
   +--------------------+
   |   long ar2[]       | -24
   +--------------------+
   |     i    | st1 s1  | -16
   +--------------------+
   |        this        | -8
   +--------------------+
   |        old rbp     | <- rbp
   +--------------------+
*/

.set this, -8
.set st1, -16
.set i, -12
.set ar2, -24

.global _ZN2clC1E3st1Pl
_ZN2clC1E3st1Pl:

# prologo
pushq %rbp
movq %rsp, %rbp
subq $32, %rsp

# passaggio dei parametri
movq %rdi, this(%rbp)
movl %esi, st1(%rbp)
movq %rdx, ar2(%rbp)

# ciclo for
movl $0, i(%rbp)
for_due:
   cmpl $4, i(%rbp)
   jge fine_for_due
   movslq i(%rbp), %rcx
   movb st1(%rbp, %rcx, 1), %al
   movq this(%rbp), %r8
   movb %al, (%r8, %rcx, 1)
   movb %al, 4(%r8, %rcx, 1)
   movq ar2(%rbp), %r9
   movq (%r9, %rcx, 8), %rbx
   movq %rbx, 8(%r8, %rcx, 8)
   incl i(%rbp)
   jmp for_due
fine_for_due:

leave
ret

/* stack elab1
   0                    7
   +--------------------+
   |    ind. ritorno    | -88
   +--------------------+
   |   cla v1 | cla v2  | -80
   +--------------------+
   |         cla        | -72
   +--------------------+
   |         cla        | -64
   +--------------------+
   |         cla        | -56
   +--------------------+
   |         cla        | -48
   +--------------------+
   |   i     |    st1   | -40
   +--------------------+
   |  st2    |   . .    | -32
   +--------------------+
   |  . .    |    st2   | -24
   +--------------------+
   |       ar1[]        | -16
   +--------------------+
   |       this         | -8
   +--------------------+
   |      old rbp       | <- rbp
   +--------------------+
 */

.set this, -8
.set ar1, -16
.set st2, -32
.set st1, -40
.set i, -44
.set cla, -80
.set ritorno, -88

.global _ZN2cl5elab1EPc3st2
_ZN2cl5elab1EPc3st2:
# prologo
pushq %rbp
movq %rsp, %rbp
subq $96, %rsp
# passaggio dei parametri
movq %rdi, ritorno(%rbp)
movq %rsi, this(%rbp)
movq %rdx, ar1(%rbp)
movq %rcx, st2(%rbp)
movq %r8, st2+8(%rbp)
# ciclo for
movl $0, i(%rbp)
for_tre:
   cmpl $4, i(%rbp)
   jge fine_for_tre
   movslq i(%rbp), %rcx
   movq ar1(%rbp), %r9
   movb (%r9, %rcx, 1), %al
   movb %al, st1(%rbp, %rcx, 1)
   incl i(%rbp)
   jmp for_tre
fine_for_tre:
# chiamata a cla
movq ritorno(%rbp), %rdi
movl st1(%rbp), %esi
call _ZN2clC1E3st1
# ciclo for
movl $0, i(%rbp)
for_quattro:
   cmpl $4, i(%rbp)
   jge fine_for_quattro
   movslq i(%rbp), %rcx
   movl st2(%rbp, %rcx, 4), %eax
   movslq %eax, %rax
   movq ritorno(%rbp), %r8
   movq %rax, 8(%r8, %rcx, 8)
   incl i(%rbp)
   jmp for_quattro
fine_for_quattro:

leave
ret
