.include "./../servi2.s"
/* STACK leggis
+-------------------+
|                   |
+-------------------+
|         |  ss.n2  | -16 <- rsp
+-------------------+
|    |ss.c|  ss.n1  | -8
+-------------------+
|                   | <- rbp
+-------------------+
*/

.text

.set ss, -8

leggis:
# prologo
pushq %rbp
movq %rsp, %rbp
subq $16, %rsp

# lettura dei parametri di ss
call leggiint
movl %eax, ss(%rbp)
call leggichar
movb %al, ss+4(%rbp)
call leggiint
movl %eax, ss-8(%rbp)

# return ss
movq ss(%rbp), %rdi
movl ss-8(%rbp), %esi

leave
ret


/* STACK scrivis
+-----------------------+
|                       |
+-----------------------+
|           |   ss.n2   | -16
+-----------------------+
|      |ss.c|   ss.n1   | -8
+-----------------------+
|                       | <- rbp
+-----------------------+
*/
.set ss, -8
scrivis:

# prologo
pushq %rbp
movq %rsp, %rbp
subq $16, %rsp

# passaggio parametri
movq %rdi, ss(%rbp)
movl %esi, ss-8(%rbp)

# scrittura a video
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

/* STACK main
+-----------------------+
|                       |
+-----------------------+
|           |   st.n2   | -16
+-----------------------+
|      |st.c|   st.n1   | -8
+-----------------------+
|                       | <- rbp
+-----------------------+
*/
.set st, -8

main:

# prologo
pushq %rbp
movq %rsp, %rbp
subq $16, %rsp

# st = leggis()
call leggis
movq %rdi, st(%rbp)
movl %esi, st-8(%rbp)

# fair(st)
leaq st(%rbp), %rdi
call fair

# scrivis(st)
movq st(%rbp), %rdi
movl st-8(%rbp), %esi
call scrivis

leave
ret
