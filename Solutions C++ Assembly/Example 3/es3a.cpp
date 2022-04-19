// programma sommaintRif, file es3a.cpp
#include "./../servi2.cpp"
extern "C" void elab3(int& tot, int n1, int n2);

int main(){
    int a, b;
    int ris;
    a = leggiint();
    b = leggiint();
    elab3(ris, a, b);
    scriviint(ris);
    nuovalinea();
};