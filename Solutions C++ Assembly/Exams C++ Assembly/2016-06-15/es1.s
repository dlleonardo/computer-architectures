/*  COSTRUTTORE CL
    classe salvata in memoria
    variabili della classe st1 s, int v[4]
  +0 . . . . . . . . . . +7
  +-----------------------+
0 |vc[0]| . . . . .|vc[7] |  <- st1 s              
  +-----------------------+
8 |    v[0]   |    v[1]   | <- int v[4]
  +-----------------------+
16|    v[2]   |    v[3]   |
  +-----------------------+

stack costruttore cl
+7 . . . . . . . . . . +0
+-----------------------+
|                       | -32 <- rsp
+-----------------------+
|           &s2         | -24
+-----------------------+
|      i     |        |c| -16
+-----------------------+
|          this         | -8  puntatore a cl
+-----------------------+
|     vecchio rbp       | <- rbp
+-----------------------+
*/

.set st1_classe, 0
.set v_classe, 8

.set this, -8
.set c, -16
.set i, -12
.set s2, -24

.global _ZN2clC1EcR3st1

_ZN2clC1EcR3st1:

# prologo
pushq %rbp
movq %rsp, %rbp
subq $32 ,%rsp

# passaggio parametri, rdi, rsi, rdx
movq %rdi, this(%rbp)
movb %sil, c(%rbp)
movq %rdx, s2(%rbp)

# primo ciclo for
movl $0, i(%rbp)

primo_for:
    cmpl $8, i(%rbp)
    jge fine_primo_for

    # c + i 
    movsbl c(%rbp), %eax
    addl i(%rbp), %eax                      # eax = c + i
    movslq i(%rbp), %rbx
    movq this(%rbp), %rcx                   # this(%rbp) vc membro classe st1
    # rcx = s membro di classe
    # rbx = indice i
    # st1_classe = vc elemento di struct
    movb %al, (%rcx, %rbx, 1)       # s.vc[i] = c + i

    incl i(%rbp)
    jmp primo_for

fine_primo_for:
    movl $0, i(%rbp)

secondo_for:
    cmpl $4, i(%rbp)
    jge fine_secondo_for

    movslq i(%rbp), %r8
    movq this(%rbp), %rcx
    movq s2(%rbp), %rbx                     # rbx = indirizzo di s2
    # accedo a s2.vc[i]
    # rbx = s2.vc
    # r8 = indice i
    movb (%rbx, %r8, 1), %al
    subb (%rcx, %r8, 1), %al
    movsbl %al, %eax
    movl %eax, 8(%rcx, %r8, 4)

    incl i(%rbp)
    jmp secondo_for

fine_secondo_for:
    leave
    ret

.global _ZN2cl5elab1E3st1R3st2
.text
/* stack elab1
this puntatore alla classe,
st1 parametro per valore,
st2 passato per riferimento (indirizzo),
i indice intero per array,
dichiarazione oggetto cla che va allocato
sullo stack, occupa 3 righe, si utilizza
l'indirizzo piu' piccolo!
+-----------------------+
|           cla (st1)   | -56 <- rsp
+-----------------------+
|           cla v       | -48
+-----------------------+
|           cla v       | -40
+-----------------------+
|            |    i     | -32
+-----------------------+
|          st2& s2      | -24
+-----------------------+
|      st1 s1 (8 char)  | -16
+-----------------------+
|            this       | -8
+-----------------------+
|                       | <- rbp
+-----------------------+
*/

.set this, -8
.set st1_s1, -16
.set st2_s2, -24
.set i, -32
# etichetta classe numero piu' piccolo!
.set classe, -56

_ZN2cl5elab1E3st1R3st2:
# prologo
pushq %rbp
movq %rsp, %rbp
subq $64, %rsp

# passaggio dei parametri cl::elab1
movq %rdi, this(%rbp)
movq %rsi, st1_s1(%rbp) 
movq %rdx, st2_s2(%rbp)

# chiamata al costruttore 3 parametri
leaq classe(%rbp), %rdi
movb $'f', %sil
leaq st1_s1(%rbp), %rdx
call _ZN2clC1EcR3st1

# ciclo for
movl $0, i(%rbp)

for:
    cmpl $4, i(%rbp)
    jge fine_for

    movslq i(%rbp), %rcx
    # ottengo s1.vc[i], s1 e' dentro lo stack
    movb st1_s1(%rbp, %rcx, 1), %dl
    movq this(%rbp), %rdi
    movb st1_classe(%rdi,%rcx,1), %bl
    cmpb %dl, %bl
    jge dopo_if

    # cla.s.vc[i] e' salvata
    # sullo stack locale di elab1
    movb classe + st1_classe(%rbp,%rcx,1), %al
    movq this(%rbp), %rdi
    movb %al, st1_classe(%rdi, %rcx, 1)

dopo_if:
    # cla.v[i]
    movl classe + v_classe(%rbp,%rcx,4), %eax
    # v[i] < cla.v[i]
    # v_classe(%rdi, %rcx, 4) = v[i]
    # rdi puntatore this
    movq this(%rbp), %rdi
    cmpl %eax, v_classe(%rdi, %rcx, 4)
    jge dopo_if_due

    # cla.v[i] + i
    addl i(%rbp), %eax
    movl %eax, v_classe(%rdi,%rcx,4) 

dopo_if_due:
    incl i(%rbp)
    jmp for

fine_for:

leave
ret
