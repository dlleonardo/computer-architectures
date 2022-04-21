.include "./../servi2.s"
.text
/*
+-----------+
|     |ss.n2| -16 <- rsp
+-----------+
||ss.c|ss.n1| -8
+-----------+
|   old rbp | <- rbp
+-----------+
*/

.set ss, -8

leggis:

# prologo
pushq %rbp
movq %rsp, %rbp
subq $16, %rsp

call leggiint
movl %eax, ss(%rbp)     # ss.n1 = leggiint()
call leggichar
movb %al, ss+4(%rbp)    # ss.c = leggiint()
call leggiint
movl %eax, ss-8(%rbp)   # ss.n2 = leggiint()

movq ss(%rbp), %rax
movl ss-8(%rbp), %edx

leave
ret

scrivis:
/* STACK
+---------------------------+
|             |    ss.n2    | -16 <- rsp
+---------------------------+
|        |ss.c|    ss.n1    | -8
+---------------------------+
|         old rbp           | <- rbp
+---------------------------+
*/

.set ss, -8

# prologo
pushq %rbp
movq %rsp, %rbp
subq $16, %rsp

# parametri
movq %rdi, ss(%rbp)
movl %esi, ss-8(%rbp)

movl ss(%rbp), %edi
call scriviint
movb ss+4(%rbp), %dil
call scrivichar
movl ss-8(%rbp), %edi
call scriviint
call nuovalinea

leave
ret

.global main
/* STACK
+-----------------------+
|           |   st2.n2  | -32 <- rsp
+-----------------------+
|     |st2.c|   st2.n1  | -24
+-----------------------+
|           |   st1.n2  | -16
+-----------------------+
|     |st1.c|   st1.n1  | -8
+-----------------------+
|       old rbp         | <- rbp
+-----------------------+
*/
.set st1, -8
.set st2, -24

main:
# prologo
pushq %rbp
movq %rsp, %rbp
subq $32, %rsp

# st1 = leggis()
call leggis
movq %rax, st1(%rbp)
movl %edx, st1-8(%rbp)

# st2 = fai(st1)
movq st1(%rbp), %rdi
movl st1-8(%rbp), %esi
call fai
movq %rax, st2(%rbp)
movl %edx, st2-8(%rbp)

# scrivis(st2)
movq st2(%rbp), %rdi
movl st2-8(%rbp), %esi
call scrivis

movl $0, %eax

leave
ret
