// programma puntatoreRif, file es5a.cpp
#include "./../servi2.cpp"
extern "C" void trovamin(int*& p, int* pa, int* pb);
// restituisce in p uno fra pa e pb, a seconda del minimo intero puntato
int main(){
    int n, m;
    int* pun;
    n = leggiint();
    m = leggiint();
    trovamin(pun, &n, &m);
    scriviint(*pun);
    nuovalinea();
    return 0;
};