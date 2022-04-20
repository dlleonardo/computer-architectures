.global trovamin

/* STACK
+7 . . . . . . . . +0
+-------------------+
|        XXX        | -32 <- rsp
+-------------------+
|         pb        | -24
+-------------------+
|         pa        | -16
+-------------------+
|         p         | -8
+-------------------+
|     old rbp       | <- rbp 
+-------------------+
*/

.set p, -8
.set pa, -16
.set pb, -24

trovamin:

    # prologo
    pushq %rbp
    movq %rsp, %rbp
    subq $32, %rsp

    # rdi, rsi, rdx
    movq %rdi, p(%rbp)
    movq %rsi, pa(%rbp)
    movq %rdx, pb(%rbp)

    movq pa(%rbp), %rax     # rax = indirizzo di pa
    movq pb(%rbp), %rbx     # rbx = indirizzo di pb
    movl (%rax), %ecx       # ecx = contenuto di pa, indirizzamento indiretto
    movl (%rbx), %edx       # edx = contenuti di pb

    cmpl %edx, %ecx       # confronto pa con pb, pa < pb
    jg else
    # p = pa, attenzione sono indirizzi
    movq pa(%rbp), %rax
    movq p(%rbp), %rbx
    movq %rax, (%rbx)   # l'indirizzo di pa, deve andare in p, *p = pa
    jmp fine

else:
    # p = pb
    movq pb(%rbp), %rax 
    movq p(%rbp), %rbx
    movq %rax, (%rbx)

fine:
    leave
    ret
