.include "./../servi2.s"
.text

/* STACK scriviris
+-----------------------+
|           &ss         | -16 <- rsp
+-----------------------+
|            |    i     | -8
+-----------------------+
|                       | <- rbp
+-----------------------+
*/

.set i, -8
.set ss, -16

scriviris:

# prologo
pushq %rbp
movq %rsp, %rbp
subq $16, %rsp

movq %rdi, ss(%rbp)
movq ss(%rbp), %rbx

# scriviint(ss.n1) 
movl (%rbx), %edi
call scriviint
movl 4(%rbx), %edi
call scriviint

# ciclo for
movl $0, i(%rbp)

for:
    cmpl $10, i(%rbp)    
    jge fine_for
    movslq i(%rbp), %rcx
    movb 8(%rbx, %rcx, 1), %dil     # ss.a[i]
    call scrivichar
    incl i(%rbp)
    jmp for

fine_for:
    call nuovalinea

leave
ret

/* STACK main
+---------------------------+
|    sa.n2    |     sa.n1   | -48
+---------------------------+
|sa.a[7]| . . . . . |sa.a[0]| -40
+---------------------------+
|                |sa.a[9]|  | -32
+---------------------------+
|            lav            | -24
+---------------------------+
|            lav            | -16
+---------------------------+
|                 |    lav  | -8
+---------------------------+
|                           | <- rbp
+---------------------------+
*/

.global main
.set sa, -48
.set lav, -24

main: 

# prologo
pushq %rbp 
movq %rsp, %rbp
subq $48, %rsp

# chiamata a costruttore fstruct(5,'a')
leaq lav(%rbp), %rdi    # parametro aggiuntivo per indirizzo
movl $5, %esi           # parametro 5 II reg
movb $'a', %dl          # parametro 'a' III reg
call fstruct

# assegno sa = fstruct(5,'a')
movq lav(%rbp), %rax        # lav in sa, 18 byte
movq %rax, sa(%rbp)
movq lav+8(%rbp), %rax
movq %rax, sa+8(%rbp)
movw lav+16(%rbp), %ax
movw %ax, sa+16(%rbp)

# scriviris(sa)
leaq sa(%rbp), %rdi
call scriviris
movl $0, %eax

leave 
ret
