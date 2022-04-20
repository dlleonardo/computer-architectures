// programma puntatore, file es4a.cpp
#include "./../servi2.cpp"
extern "C" void add(int* p, int i);

int main(){
    int a, b;
    a = leggiint();
    b = leggiint();
    add(&a, b);
    scriviint(a);
    nuovalinea();
    return 0;
};