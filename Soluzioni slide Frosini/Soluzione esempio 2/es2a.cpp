// programma sommaintLoc, file es2a.cpp
#include "./../servi2.cpp"
extern "C" int elab2(int n, int m);
int main(){
    int a, b, ris;
    a = leggiint();
    b = leggiint();
    ris = elab2(a,b);
    scriviint(ris);
    nuovalinea();
    return 0;
};