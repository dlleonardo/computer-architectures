.text
.global elab1
.set n1, -8
.set n2, -4
.set i, -16
.set j, -12

/*
Disegno lo stack

+7 +6  ... +1 +0
+---------------+
|  j	|  i	| -16 <-- rsp
+---------------+
|  n2	|  n1	| -8
+---------------+
| vecchio rbp	| <-- rbp
+---------------+
*/

elab1:
#prologo
pushq %rbp
movq %rsp, %rbp
subq $16, %rsp

# rdi, rsi, rdx, rcx, r8, r9
# edi = n1, esi = n2, questi parametri mi verranno passati dal main
# nei rispettivi registri edi, esi
movl %edi, n1(%rbp)
movl %esi, n2(%rbp)

#calcolo di i, lo metto nel registro %eax (posso sceglierlo arbitrariamente)
movl n1(%rbp), %eax
addl n2(%rbp), %eax	# calcola %eax = n2(%rbp) + %eax
movl %eax, i(%rbp)

#calcolo di j, lo metto nel registro %ebx
movl n1(%rbp), %ebx
subl n2(%rbp), %ebx	#calcola %ebx = %ebx - n2(%rbp)
movl %ebx, j(%rbp)

#calcola il valore di return
movl i(%rbp), %eax
imull j(%rbp), %eax	#calcola %eax = %eax * j(%rbp)

leave
ret
