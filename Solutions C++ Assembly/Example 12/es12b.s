.global fstruct

/* STACK
+-----------------------+
|                       | -48 <-rsp
+-----------------------+
|           |      i    | -40
+-----------------------+
|   st.n2   |   st.n1   | -32
+-----------------------+
|a[7]| . . . . . . |a[0]| -24
+-----------------------+
|           | |a[9]|a[8]| -16
+-----------------------+
|         |c|     a     | -8
+-----------------------+
|                       | <- rbp
+-----------------------+
*/

.set a, -8
.set c, -4
.set i, -40
.set st, -32

fstruct:

# prologo
pushq %rbp
movq %rsp, %rbp
subq $48, %rsp

# passaggio parametri
# dovendo restituire una struct a piu di 16 byte, %rdi deve essere libero
movl %esi, a(%rbp)      # II reg, esi = 32 bit di rsi
movb %dl, c(%rbp)       # III reg, dl = 8 bit di rdx

# st.n1 = a
movl a(%rbp), %eax
movl %eax, st(%rbp)

# st.n2 = 2*a
movl a(%rbp), %eax
imull $2, %eax
movl %eax, st+4(%rbp)

# ciclo for 
movl $0, i(%rbp)

for:
    cmpl $10, i(%rbp)
    jge fine_for
    movzbl c(%rbp), %ebx        # estendo il char su 4 byte per la somma con i
    addl i(%rbp), %ebx          # ebx = c + i
    movslq i(%rbp), %rax        # mi serve per accedere a st.a[i]
    movb %bl, st+8(%rbp,%rax,1)    # indice + scala*1
    incl i(%rbp)
    jmp for

fine_for:
    movq %rdi, %rax         # rdi = indirizzo risultato
    movq st(%rbp), %rdi     # ritorna st interi
    movq %rdi, (%rax)
    movq st+8(%rbp), %rdi   # prendo gli 8 char all'indirizzo -32
    movq %rdi, 8(%rax)
    movw st+16(%rbp), %di   # word = 2 byte, prendo gli ultimi 2 char a -40
    movw %di, 16(%rax)

leave
ret
