.include "./../servi2.s"

.data

.text
.global main

.set a, -8
.set b, -4
.set ris, -16

/*
+-------------------+
|          |   ris  |  -16 <- rsp
+-------------------+
|    b     |    a   |  -8
+-------------------+
|    vecchio rbp    |  <- rbp
+-------------------+
*/

main:

# prologo
pushq %rbp
movq %rsp, %rbp
subq $16, %rsp

call leggiint
movl %eax, a(%rbp)
call leggiint
movl %eax, b(%rbp)

# chiamata a elab2, i parametri sono passati in edi, esi
movl a(%rbp), %edi
movl b(%rbp), %esi
call elab2
movl %eax, ris(%rbp)

# chiamata a scriviint
mov ris(%rbp), %edi
call scriviint
call nuovalinea

leave
ret
