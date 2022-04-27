.global _ZN2clC1E3st1
_ZN2clC1E3st1:

/*

+7 +6 +5 +4  | +3 +2 +1 +0

+-----------------------+
|         this          | -32
+-----------------------+
|            |     i    | -24
+-----------------------+
|    st1[1]  |   st1[0] | -16
+-----------------------+
|    st1[3]  |   st1[2] | -8
+-----------------------+

classe 
   +-----------------------+
0  |           v2[0]       | 
   +-----------------------+
8  |           v2[1]       |    
   +-----------------------+
16 |            v2[2]      |    
   +-----------------------+
24 |            v2[3]      |    
   +-----------------------+
32 |v1|v1|v1|v1|   v3[0]   |    
   +-----------------------+
40 |    v3[1]   |    v3[2] |
   +-----------------------+
48 |    v3[3]   |  XXXXXX  | 
   +-----------------------+
*/



.set i,-24
.set st1,-16
.set this,-32

.set v2,0
.set v1,32
.set v3,36

push %rbp
movq %rsp,%rbp
subq $32,%rsp

#rdi this

movq %rdi,this(%rbp)
movq %rsi,st1(%rbp)
movq %rdx,st1+8(%rbp)

movl $0,i(%rbp)

for:
    cmpl $4,i(%rbp)
    jge fine_for

    movslq i(%rbp),%rcx
    movl st1(%rbp,%rcx,4),%eax
    movb %al,v1(%rdi,%rcx,1)

    movslq %eax,%r8
    addq %r8,%r8
    #alternativa sall $1,%r8
    movq %r8,v2(%rdi,%rcx,8)

    #moltiplico per 4
    sall $2,%eax
    movl %eax,v3(%rdi,%rcx,4)

    incl i(%rbp)
    jmp for


fine_for:

leave 
ret


.set ar2,-32
.set i,-24
.set st1,-16
.set this,-40

/*
+7 +6 +5 +4  | +3 +2 +1 +0

+-----------------------+
|     XXXXXXXXXXXXX     | -48
+-----------------------+
|           this        | -40
+-----------------------+
|           ar2         | -32
+-----------------------+
|            |     i    | -24
+-----------------------+
|    st1[1]  |   st1[0] | -16
+-----------------------+
|    st1[3]  |   st1[2] | -8
+-----------------------+

*/

.global _ZN2clC1E3st1Pi
_ZN2clC1E3st1Pi:

push %rbp
movq %rsp,%rbp
subq $48,%rsp

movq %rdi,this(%rbp)
movq %rsi,st1(%rbp)
movq %rdx,st1+8(%rbp)
movq %rcx,ar2(%rbp)

movl $0,i(%rbp)

for_due:
cmpl $4,i(%rbp)
jge fine_for_due

movslq i(%rbp),%rcx
movl st1(%rbp,%rcx,4),%eax
movb %al,v1(%rdi,%rcx,1)

movslq %eax,%r8
salq $3,%r8
movq %r8,v2(%rdi,%rcx,8)

#indirizzo
movq ar2(%rbp),%r9
movl (%r9,%rcx,4),%eax
movl %eax,v3(%rdi,%rcx,4)

incl i(%rbp)
jmp for_due

fine_for_due:

leave
ret


/*
+7 +6 +5 +4  | +3 +2 +1 +0

+-----------------------+
|         XXXXXXXXX     | -64
+-----------------------+
|         this          | -56
+-----------------------+
|         ritorno       | -48
+-----------------------+
|           st1         | -40
+-----------------------+
|           st1         | -32
+-----------------------+
|            |     i    | -24
+-----------------------+
|           st2  addr   | -16
+-----------------------+
|           ar1 addr    | -8
+-----------------------+

*/

.set ritorno,-48
.set this,-56
.set ar1,-8
.set st2,-16
.set st1,-40
.set i,-24


.global _ZN2cl5elab1EPKcR3st2
_ZN2cl5elab1EPKcR3st2:

pushq %rbp
movq %rsp,%rbp
subq $64,%rsp

movq %rdi,ritorno(%rbp)
movq %rsi,this(%rbp)
movq %rdx,ar1(%rbp)
movq %rcx,st2(%rbp)

movl $0,i(%rbp)

for_tre:
cmpl $4,i(%rbp)
jge fine_for_tre

movslq i(%rbp),%rcx

movq ar1(%rbp),%r8
movb (%r8,%rcx,1),%al
movsbl %al,%eax #ar1 esteso
addl i(%rbp),%eax
movl %eax,st1(%rbp,%rcx,4)

incl i(%rbp)
jmp for_tre

fine_for_tre:

movq ritorno(%rbp),%rdi
movq st1(%rbp),%rsi
movq st1+8(%rbp),%rdx
call _ZN2clC1E3st1


movl $0,i(%rbp)

for_quattro:
cmpl $4,i(%rbp)
jge fine_for_quattro

movslq i(%rbp),%rcx
movq st2(%rbp),%r9
movb (%r9,%rcx,1),%al
movsbl %al,%eax

movq ritorno(%rbp),%r10
movl %eax,v3(%r10,%rcx,4)

incl i(%rbp)
jmp for_quattro

fine_for_quattro:
leave
ret
