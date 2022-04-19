.include "./../servi2.s"
.data
alfa:   .long 0
beta:   .long 0

.text
.set ris, -8

/* 

+--------------+
|       |      | -16
+--------------+
|       | ris  | -8 <- rsp
+--------------+
| vecchio rbp  | <-- rbp
+--------------+ 

*/

.global main
main:

# prologo
pushq %rbp
movq %rsp, %rbp
subq $8, %rsp

# la funzione leggiint lascia il parametro inserito nel registro %eax
call leggiint
movl %eax, alfa(%rip)
call leggiint
movl %eax, beta(%rip)

# %edi, %esi vengono poi utilizzati da elab1
movl alfa(%rip), %edi
movl beta(%rip), %esi
call elab1
movl %eax, ris(%rbp)

# scriviint vuole il risultato dentro %edi
movl ris(%rbp), %edi
call scriviint
call nuovalinea

leave
ret
