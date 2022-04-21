// programma strutturaRif, file es11a.cpp
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
    return ss;
};
extern "C" void scrivis(s ss){
    scriviint(ss.n1);
    scrivichar(ss.c);
    scriviint(ss.n2);
    nuovalinea();
};
extern "C" void fair(s& ss);
int main(){
    s st;
    st = leggis();
    fair(st);
    scrivis(st);
    return 0;
};