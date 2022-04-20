.include "./../servi2.s"
.text
.global main

/* STACK
+7 . . . . . . . . +0
+-------------------+
|         |         | -32 <- rsp
+-------------------+
|    i    |   ar[4] | -24
+-------------------+
|    ..   |    ..   | -16
+-------------------+
|   ..    |  ar[0]  | -8
+-------------------+
|         old rbp   | <- rbp
+-------------------+
*/

.set ar, -8
.set i, -20

main:

# prologo 
pushq %rbp
movq %rsp, %rbp
subq $32, %rsp

movl $0, i(%rbp)
# primo for
primo_for:
    cmpl $5, i(%rbp)            # confronto i < n
    jge fine_primo_for
    call leggiint     
    movslq i(%rbp), %rdi        # estensione di i da long a quad, per accedere a ar[i]          
    movl %eax, ar(%rbp,%rdi,4)  # ar(%rbp,%rdi,4), %rbp perche' siamo nel main 
    incl i(%rbp)            
    jmp primo_for

fine_primo_for:
    leaq ar(%rbp), %rdi
    movl $5, %esi
    call raddoppia
    movl $0, i(%rbp)

# secondo ciclo for
secondo_for:
    cmpl $5, i(%rbp)
    jge fine_secondo_for
    movslq i(%rbp), %rax
    movl ar(%rbp, %rax, 4), %edi
    # passo il parametro tramite %rdi
    call scriviint
    incl i(%rbp)
    jmp secondo_for

fine_secondo_for:
    call nuovalinea
    mov $0, %eax
    leave
    ret
