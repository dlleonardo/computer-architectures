// programma struttura, file es10a.cpp
#include "./../servi2.cpp"
struct s{
    int n1;
    char c;
    int n2;
};
extern "C" s leggis(){
    s ss;
    ss.n1 = leggiint();
    ss.c = leggichar();
    ss.n2 = leggiint();
    return ss;              // in rax, edx 
};
extern "C" void scrivis(s ss){
    scriviint(ss.n1);
    scrivichar(ss.c);
    scriviint(ss.n2);
    nuovalinea();
};
extern "C" s fai(s st);

int main(){
    s st1, st2;
    st1 = leggis();
    st2 = fai(st1);
    scrivis(st2);
    return 0;
}