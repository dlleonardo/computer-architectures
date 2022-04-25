// programma strutturaLungaRif, file es13a.cpp
#include "./../servi2.cpp"
struct s{
    int n1;
    int n2;
    char a[10];
};
extern "C" void fstructr(s& st, int a, char c);
extern "C" void scriviris(s& ss){
    int i;
    scriviint(ss.n1);
    scriviint(ss.n2);
    for(i=0; i<10; i++) scrivichar(ss.a[i]);
    nuovalinea();
};
int main(){
    s sa;
    fstructr(sa, 5, 'a');
    scriviris(sa);
    return 0;
};