.global _ZN2clC1E3st1

/*
    classe cl
    +0                     +7
    +-----------------------+
   0|          v2[0]        |
    +-----------------------+
   8|          v2[1]        |
    +-----------------------+
  16|          v2[2]        |
    +-----------------------+
  24|          v2[3]        | 
    +-----------------------+
  32|v1[0]|..v1[3]|  v3[0]  |
    +-----------------------+
  40|     v3[1]   |  v3[2]  |
    +-----------------------+
  48|     v3[3]   |  XXXXX  |
    +-----------------------+
  56|                       |
    +-----------------------+

stack costruttore cl
+7                    +0
+-----------------------+
|           |     i     | -32 <- rsp
+-----------------------+
|    vi[1]  |    vi[0]  | -24
+-----------------------+
|    vi[3]  |    vi[2]  | -16
+-----------------------+
|          this         | -8
+-----------------------+
|         old rbp       | <- rbp
+-----------------------+
*/

.set v2_classe, 0
.set v1_classe, 32
.set v3_classe, 36

.set this, -8
.set st1, -24
.set i, -32

_ZN2clC1E3st1:

# prologo
pushq %rbp
movq %rsp, %rbp
subq $32, %rsp

# passaggio parametri rdi rsi rdx
movq %rdi, this(%rbp)
movq %rsi, st1(%rbp)
movq %rdx, st1+8(%rbp)

# ciclo for
movl $0, i(%rbp)

for:
    cmpl $4, i(%rbp)
    jge fine_for

    movslq i(%rbp), %rcx
    # ss.vi[i]
    movl st1(%rbp,%rcx,4), %eax
    
    # v1[i] = ss.vi[i]
    movq this(%rbp), %r8
    movb %al, 32(%r8,%rcx,1)

    # v2[i] = ss.vi[i]*2
    sal $1, %eax
    movslq %eax, %rax
    movq this(%rbp), %r8
    movq %rax, (%r8,%rcx,8)

    # v3[i] = 4*ss.vi[i]
    movl st1(%rbp,%rcx,4), %eax
    sal $2, %eax
    movq this(%rbp), %r8
    movl %eax, 36(%r8,%rcx,4)

    incl i(%rbp)
    jmp for

fine_for:

leave
ret

/*  costruttore cl2
        classe cl
    +0                     +7
    +-----------------------+
   0|          v2[0]        |
    +-----------------------+
   8|          v2[1]        |
    +-----------------------+
  16|          v2[2]        |
    +-----------------------+
  24|          v2[3]        | 
    +-----------------------+
  32|v1[0]|..v1[3]|  v3[0]  |
    +-----------------------+
  40|     v3[1]   |  v3[2]  |
    +-----------------------+
  48|     v3[3]   |  XXXXX  |
    +-----------------------+
  56|                       |
    +-----------------------+

    stack cl2
    +-----------------------+
    |           |     i     | -40
    +-----------------------+
    |         ar2[]         | -32 
    +-----------------------+
    |    st1    |   st1[0]  | -24
    +-----------------------+
    |    st[3]  |   st1     | -16
    +-----------------------+
    |         this          | -8
    +-----------------------+
    |         old rbp       | <- rbp
    +-----------------------+
*/
.global _ZN2clC1E3st1Pi

_ZN2clC1E3st1Pi:

.set this, -8
.set st1, -24
.set ar2, -32
.set i, -40

# prologo
pushq %rbp
movq %rsp, %rbp
subq $48, %rsp

# passaggio parametri
movq %rdi, this(%rbp)
movq %rsi, st1(%rbp)
movq %rdx, st1+8(%rbp)
movq %rcx, ar2(%rbp)
movl $0, i(%rbp)

# ciclo for
for_cl:
    cmpl $4, i(%rbp)
    jge fine_for_cl 

    movslq i(%rbp), %rcx
    movl st1(%rbp, %rcx, 4), %eax
    movq this(%rbp), %r8
    movb %al, 32(%r8, %rcx, 1)

    sal $3, %eax
    movslq %eax, %rax
    movq %rax, (%r8, %rcx, 8)

    movq ar2(%rbp), %r9
    movl (%r9, %rcx, 4), %ebx
    movl %ebx, 36(%r8, %rcx, 4)

    incl i(%rbp)
    jmp for_cl

fine_for_cl:

leave
ret

/*  ELAB1
    la funzione elab1 restituisce una classe per valore,
    che occupa piu di 16 byte,
    bisogna seguire un approccio diverso

    classe cl
    +0                     +7
    +-----------------------+
   0|          v2[0]        |
    +-----------------------+
   8|          v2[1]        |
    +-----------------------+
  16|          v2[2]        |
    +-----------------------+
  24|          v2[3]        | 
    +-----------------------+
  32|v1[0]|..v1[3]|  v3[0]  |
    +-----------------------+
  40|     v3[1]   |  v3[2]  |
    +-----------------------+
  48|     v3[3]   |  XXXXX  |
    +-----------------------+
  56|                       |
    +-----------------------+
    stack elab1
    %rax %rdx
    %rdi [indirizzo dove salvare classe]
    %rsi this
    %rdx puntatore
    %rcx riferimento
    +-----------------------+
    |       ind. ritorno    | -56
    +-----------------------+
    |            |    i     | -48
    +-----------------------+
    |      st1   |   st1[0] | -40
    +-----------------------+
    |     st1[3] |    st1   | -32
    +-----------------------+
    |        st2& s2        | -24
    +-----------------------+
    |         *ar1          | -16
    +-----------------------+
    |         this          | -8
    +-----------------------+ 
    |        old rbp        | <- rbp
    +-----------------------+
*/

.set this, -8
.set ar1, -16
.set st2, -24
.set i, -48
.set ritorno, -56
.set st1, -40

.global _ZN2cl5elab1EPKcR3st2

_ZN2cl5elab1EPKcR3st2:

# prologo
pushq %rbp
movq %rsp, %rbp
subq $64, %rsp

# rdi contiene l'indirizzo di ritorno in cui 
# salvare la classe, e' il primo parametro
movq %rdi, ritorno(%rbp)
movq %rsi, this(%rbp)
movq %rdx, ar1(%rbp)
movq %rcx, st2(%rbp)

# primo ciclo for
movl $0, i(%rbp)
for_uno:
    cmpl $4, i(%rbp)
    jge fine_for_uno
    
    # ar1[i] + i
    movslq i(%rbp), %rcx
    movq ar1(%rbp), %r8
    movb (%r8, %rcx, 1), %al
    movsbl %al, %eax
    addl i(%rbp), %eax
    # s1.vi[i] = ar1[i] + i
    movl %eax, st1(%rbp, %rcx, 4)

    incl i(%rbp)
    jmp for_uno

fine_for_uno:

# cl cla(s1)
# passaggio dei parametri
# parametri this, s1
# rdi = this, %rsi %rdx = s1 
movq ritorno(%rbp), %rdi
movq st1(%rbp), %rsi
movq st1+8(%rbp), %rdx
call _ZN2clC1E3st1

# secondo ciclo for
movl $0, i(%rbp)

for_due:
    cmpl $4, i(%rbp)
    jge fine_for_due

    movslq i(%rbp), %rcx
    movq st2(%rbp), %r8
    movb (%r8, %rcx, 1), %al
    movsbl %al, %eax
    movq ritorno(%rbp), %r8
    movl %eax, 36(%r8, %rcx, 4)

    incl i(%rbp)
    jmp for_due

fine_for_due:

leave 
ret
